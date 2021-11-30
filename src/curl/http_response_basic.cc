#include <network/curl/http_response_basic.h>

#include <event_loop.h>

namespace Evpp
{
    HttpResponseBasic::HttpResponseBasic(const std::string& uri, const std::string& path, CURL* handler, const ResponseProgressHandler& http_response_progress) :
        http_uri(uri),
        http_handler(handler),
        http_response_progress(http_response_progress),
        http_response_code(0),
        http_response_cache(0)
    {
        if (0 == http_stream.is_open())
        {
            http_stream.open(path, std::ios::out | std::ios::app | std::ios::binary);
        }
    }

    HttpResponseBasic::~HttpResponseBasic()
    {
        http_stream.clear();
        http_stream.close();
    }

    u96 HttpResponseBasic::OnResponse(void* buffer, u96 size, u96 nmemb)
    {
        if (http_stream.is_open())
        {
            if (http_stream.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(size * nmemb)).good())
            {
                return size * nmemb;
            }
        }
        return size * nmemb;
    }

    i32 HttpResponseBasic::OnResponseProgress(double dl_file_max, double dl_file_min, double ul_file_max, double ul_file_min)
    {
        if (CURLcode::CURLE_OK == curl_easy_getinfo(http_handler, CURLINFO::CURLINFO_RESPONSE_CODE, &http_response_code))
        {
            switch (http_response_code)
            {
                case 206: break;
                case 0:
                case 300:
                case 301:
                case 302:
                {
                    return 0;
                }
                case 303:
                case 304:
                case 305:
                case 306:
                case 307:
                case 400:
                {
                    EVENT_ERROR("The server could not understand the request due to invalid syntax."); return 1;
                }
                case 401:
                {
                    EVENT_ERROR(R"(Although the HTTP standard specifies "unauthorized", semantically this response means "unauthenticated". That is, the client must authenticate itself to get the requested response.)");
                    return 1;
                }
                case 402:
                {
                    EVENT_ERROR("This response code is reserved for future use. The initial aim for creating this code was using it for digital payment systems, however this status code is used very rarely and no standard convention exists.");
                    return 1;
                }
                case 403:
                {
                    EVENT_ERROR("The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401, the client's identity is known to the server.");
                    return 1;
                }
                case 404:
                {
                    EVENT_ERROR("wrong uri address, please check if the file can be downloaded normally");
                    return 1;
                }
                case 405:
                {
                    EVENT_ERROR("The request method is known by the server but is not supported by the target resource. For example, an API may forbid DELETE-ing a resource.");
                    return 1;
                }
                case 406:
                {
                    EVENT_ERROR("This response is sent when the web server, after performing server-driven content negotiation, doesn't find any content that conforms to the criteria given by the user agent.");
                    return 1;
                }
                case 407:
                case 408:
                case 409:
                case 410:
                case 411:
                case 412:
                case 413:
                case 414:
                case 415: return 1;
                case 416:
                {

                    break;
                }
                case 417:
                case 500:
                case 501:
                case 502:
                case 503:
                case 504:
                case 505:
                {
                    EVENT_ERROR("下载地址: %s 错误代码: %d", http_uri.c_str(), http_response_code);
                    return 1;
                }
            }

            if (0 == dl_file_max && 0 == dl_file_min && 0 == ul_file_max && 0 == ul_file_min)
            {
                return 0;
            }

            if (200 == http_response_code || 206 == http_response_code || 416 == http_response_code)
            {
                if (dl_file_min == http_response_cache)
                {
                    return 0;
                }
                else
                {
                    http_response_cache = dl_file_min;
                }

                if (0 == dl_file_max)
                {
                    dl_file_max = dl_file_min;
                }

                if (dl_file_min > 0)
                {
                    if (nullptr != http_response_progress)
                    {
                        return http_response_progress(http_uri, (dl_file_min * 100.0F) / dl_file_max);
                    }
                    else
                    {
                        EVENT_INFO("当前下载: %.2f%%", (dl_file_min * 100.0F) / dl_file_max);
                    }
                }
            }
        }
        return 0;
    }
}