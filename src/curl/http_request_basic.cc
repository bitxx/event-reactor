#include <network/curl/http_request_basic.h>
#include <network/curl/http_response_basic.h>
#include <network/curl/http_response_details_impl.h>

#include <event_loop.h>

namespace Evpp
{
    HttpRequestBasic::HttpRequestBasic(EventLoop* basic, const HttpResponseHandler& response_handler) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_prepare(0),
        http_response_handler(response_handler),
        http_response_details(std::make_shared<HttpResponseDetailsImpl>(basic, std::bind(&HttpRequestBasic::QueryResponseInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), 0))
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    HttpRequestBasic::~HttpRequestBasic()
    {
        curl_global_cleanup();
    }

    bool HttpRequestBasic::CreaterRequest(const std::string& uri, CURL* http_easy_handler)
    {
        if (uri.empty())
        {
            return false;
        }

        if (EventThread())
        {
            return CreaterRequest(PlexingIndex(), uri, http_easy_handler);
        }

        return RunInQueue(std::move(std::bind<bool(HttpRequestBasic::*)(const std::string&, CURL*)>(&HttpRequestBasic::CreaterRequest, this, uri, http_easy_handler)));
    }

    bool HttpRequestBasic::DestroyRequest(const std::chrono::milliseconds& delay)
    {
        if (event_prepare.try_acquire_for(delay))
        {
            return true;
        }

        return false;
    }
    bool HttpRequestBasic::CreaterRequest(u96 http_easy_index, const std::string& uri, CURL* http_easy_handler)
    {
        if (uri.empty() || nullptr == http_easy_handler)
        {
            return false;
        }

        if (CURLcode::CURLE_OK != curl_easy_setopt(http_easy_handler, CURLOPT_DEBUGFUNCTION, static_cast<bool(*)(CURL*, curl_infotype, char*, size_t, void*)>(&HttpRequestBasic::OnResponseLogging)))
        {
            EVENT_ERROR("CURLOPT_DEBUGFUNCTION");
        }

        if (CURLcode::CURLE_OK != curl_easy_setopt(http_easy_handler, CURLOPT_DEBUGDATA, this))
        {
            EVENT_ERROR("CURLOPT_DEBUGFUNCTION");
        }

        if (AddResponse(http_easy_index, uri))
        {
            if (http_response_details->InitialEasyPerform() && http_response_details->CreaterEasyPerform(http_easy_handler))
            {
                return true;
            }
        }

        return DelResponse(http_easy_index, uri);
    }

    bool HttpRequestBasic::AddResponse(u96 index, const std::string& uri)
    {
        if (uri.empty())
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(http_mutex);
        {
            if (http_response_session.find(uri) == http_response_session.end())
            {
                return http_response_session.try_emplace(uri, index).second;
            }
        }

        return false;
    }

    bool HttpRequestBasic::DelResponse(u96 index, const std::string& uri, CURLcode http_easy_result)
    {
        std::lock_guard<std::mutex> lock(http_mutex);
        {
            if (http_response_session.find(uri) != http_response_session.end())
            {
                if (1 == http_response_session.erase(uri))
                {
                    if (nullptr != http_response_handler)
                    {
                        if (http_response_handler(index, uri, http_easy_result))
                        {
                            ResumeIndex(index);
                        }
                    }
                }

                if (http_response_session.empty())
                {
                    event_prepare.release();
                }

                return true;
            }
        }

        return false;
    }

    bool HttpRequestBasic::DelResponse(const std::string& http_easy_url, CURL* http_easy_handler, CURLcode http_easy_result)
    {
        if (nullptr == http_easy_handler)
        {
            return false;
        }

        if (DelResponse(http_response_session[http_easy_url], http_easy_url, http_easy_result))
        {
            if (nullptr != http_easy_handler)
            {
                curl_easy_cleanup(http_easy_handler);
            }
            return true;
        }
        return false;
    }

    void HttpRequestBasic::QueryResponseInfo(u96 index, const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result)
    {
        switch (http_easy_result)
        {
            case CURLE_OK:                  EVENT_INFO("execution process -> download completed");                  break;
            case CURLE_OPERATION_TIMEDOUT:  EVENT_INFO("execution process -> timeout");                             break;
            case CURLE_SEND_ERROR:          EVENT_WARN("execution process -> network failure");                     break;
            case CURLE_RECV_ERROR:          EVENT_WARN("execution process -> network failure");                     break;
            default:                        EVENT_WARN("execution process -> unknown error: %d", http_easy_result); break;
        }

        if (RunInQueue(std::move(std::bind<bool(HttpRequestBasic::*)(const std::string &, CURL*, CURLcode)>(&HttpRequestBasic::DelResponse, this, http_easy_url, http_easy_handler, http_easy_result))))
        {
            return;
        }
    }

    bool HttpRequestBasic::OnResponseLogging(CURL *http_easy_handler, curl_infotype type, char *data, size_t size, void *handler)
    {
        switch (type)
        {
            case CURLINFO_TEXT:             EVENT_INFO("%s", data); break;
            case CURLINFO_HEADER_OUT:       break;
            case CURLINFO_DATA_OUT:         break;
            case CURLINFO_SSL_DATA_OUT:     break;
            case CURLINFO_HEADER_IN:        break;
            case CURLINFO_DATA_IN:          break;
            case CURLINFO_SSL_DATA_IN:      break;
            default:                        break;
        }
        return true;
    }
}