#include <network/curl/http_response_details.h>
#include <network/curl/http_response_perform.h>
#include <network/curl/http_response_info.h>

#include <event_timer.h>
#include <event_loop.h>

namespace Evpp
{
    HttpResponseDetails::HttpResponseDetails(EventLoop* basic, const Handler& http_response_query) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_prepare(0),
        event_timer(std::make_shared<EventTimer>(basic, std::bind(&HttpResponseDetails::OnTimer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))),
        http_response_info(std::make_shared<HttpResponseInfo>(basic, std::bind(&HttpResponseDetails::QueryResponseInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))),
        http_response_query(http_response_query),
        http_multi_handler(nullptr),
        http_multi_active(0),
        http_easy_queue(0)
    {
        EVENT_TRACE("HttpResponseDetails::HttpResponseDetails");
    }

    HttpResponseDetails::~HttpResponseDetails()
    {
        EVENT_TRACE("HttpResponseDetails::~HttpResponseDetails");
        {
            if (nullptr != http_multi_handler)
            {
                RunInQueue(std::bind(curl_multi_cleanup, http_multi_handler));
            }
        }
    }

    bool HttpResponseDetails::InitialEasyPerform()
    {
        if (nullptr == event_basic || nullptr == event_timer)
        {
            return false;
        }

        if (ExistsInited())
        {
            return true;
        }

        if (EventThread())
        {
            if (event_timer->InitedTimer())
            {
                if (ChangeStatus(Status::Init))
                {
                    if (nullptr == http_multi_handler)
                    {
                        http_multi_handler = curl_multi_init();
                    }

                    if (nullptr != http_multi_handler)
                    {
                        if (
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_MAX_HOST_CONNECTIONS, 200L) &&      // 单线程最大任务执行数量
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_MAX_TOTAL_CONNECTIONS, 200L) &&     // 单线程最大任务连接数量
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_PIPELINING, 1L) &&
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_SOCKETDATA, this) &&
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_TIMERDATA, this) &&
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_SOCKETFUNCTION, static_cast<void(*)(CURL*, evutil_socket_t, i32, void*, HttpResponsePerform*)>(&HttpResponseDetails::DefaultResponse)) &&
                            CURLMcode::CURLM_OK == curl_multi_setopt(http_multi_handler, CURLMOPT_TIMERFUNCTION, static_cast<void(*)(CURLM*, i32, void*)>(&HttpResponseDetails::DefaultResponseTimer))
                            )
                        {
                            return ChangeStatus(Status::Exec);
                        }
                    }
                }
            }
            return false;
        }

        return RunInQueue(std::move(std::bind(&HttpResponseDetails::InitialEasyPerform, this)));
    }

    bool HttpResponseDetails::CreaterEasyPerform(CURL* http_easy_handler)
    {
        if (nullptr == event_basic || nullptr == event_timer)
        {
            return false;
        }

        if (EventThread())
        {
            if (nullptr == http_multi_handler || nullptr == http_easy_handler)
            {
                return false;
            }

            if (ExistsInited() && ExistsRuning())
            {
                if (CURLMcode::CURLM_OK == curl_multi_add_handle(http_multi_handler, http_easy_handler))
                {
                    return true;
                }
            }
            return false;
        }

        return RunInQueue(std::move(std::bind(&HttpResponseDetails::CreaterEasyPerform, this, http_easy_handler)));
    }

    void HttpResponseDetails::OnTimer(EventLoop* loop, const std::shared_ptr<EventTimer>& timer, u96 index)
    {
        // 此处与 perform.cc 代码重叠 需要改善
        switch (curl_multi_socket_action(http_multi_handler, CURL_SOCKET_TIMEOUT, 0, &http_multi_active))
        {
            case CURLMcode::CURLM_OK:                                                                              break;
            case CURLMcode::CURLM_BAD_HANDLE:               EVENT_ERROR("CURLMcode::CURLM_BAD_HANDLE");            break;
            case CURLMcode::CURLM_BAD_EASY_HANDLE:          EVENT_ERROR("CURLMcode::CURLM_BAD_EASY_HANDLE");       break;
            case CURLMcode::CURLM_OUT_OF_MEMORY:            EVENT_ERROR("CURLMcode::CURLM_OUT_OF_MEMORY");         break;
            case CURLMcode::CURLM_INTERNAL_ERROR:           EVENT_ERROR("CURLMcode::CURLM_INTERNAL_ERROR");        break;
            case CURLMcode::CURLM_BAD_SOCKET:               EVENT_ERROR("CURLMcode::CURLM_INTERNAL_ERROR");        break;
            case CURLMcode::CURLM_UNKNOWN_OPTION:           EVENT_ERROR("CURLMcode::CURLM_UNKNOWN_OPTION");        break;
            case CURLMcode::CURLM_ADDED_ALREADY:            EVENT_ERROR("CURLMcode::CURLM_ADDED_ALREADY");         break;
            case CURLMcode::CURLM_RECURSIVE_API_CALL:       EVENT_ERROR("CURLMcode::CURLM_RECURSIVE_API_CALL");    break;
            case CURLMcode::CURLM_WAKEUP_FAILURE:           EVENT_ERROR("CURLMcode::CURLM_WAKEUP_FAILURE");        break;
            case CURLMcode::CURLM_BAD_FUNCTION_ARGUMENT:    EVENT_ERROR("CURLMcode::CURLM_BAD_FUNCTION_ARGUMENT"); break;
            case CURLMcode::CURLM_LAST:                     EVENT_ERROR("CURLMcode::CURLM_LAST");                  break;
            default:                                        EVENT_ERROR("CURLMcode::UNKNOWN");                     break;
        }

        if (nullptr != http_response_info || nullptr != http_multi_handler)
        {
            while (http_response_info->QueryResponseInfo(http_multi_handler, http_multi_active));
        }
    }

    void HttpResponseDetails::QueryResponseInfo(const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result)
    {
        // 没有活动的对象，就没有必要存活了。
        if (0 == http_multi_active)
        {
            if (event_timer->ExistsRuning() && event_timer->KilledTimer())
            {
                EVENT_INFO("强制停止定时器");
            }
        }

        if (nullptr != http_response_query)
        {
            http_response_query(http_easy_url, http_multi_msg, http_easy_handler, http_easy_result);
        }
    }

    void HttpResponseDetails::DefaultResponse(CURL* http_easy_handler, evutil_socket_t fd, i32 action, HttpResponsePerform* perform)
    {
        if (nullptr == http_easy_handler || INVALID_SOCKET == fd || nullptr == perform)
        {
            return;
        }

        switch (action)
        {
            case CURL_POLL_IN:
            case CURL_POLL_OUT:
            case CURL_POLL_INOUT:
            {
                if (perform->Attach())
                {
                    break;
                }
                assert(0);
            }
            case CURL_POLL_REMOVE:
            {
                if (perform->Detach() && 0 == curl_multi_assign(http_multi_handler, fd, nullptr))
                {
                    break;
                }
                assert(0);
            }
            default: break;
        }
    }

    void HttpResponseDetails::DefaultResponseTimer(CURLM* http_multi_handler, i32 delay)
    {
        if (nullptr == event_timer)
        {
            return;
        }

        if (0 < delay)
        {
            if (event_timer->CancelTimer() && event_timer->AssignTimer(delay, 0))
            {
                return;
            }
            assert(0);
        }

        if (-1 == delay)
        {
            if (event_timer->KilledTimer())
            {
                return;
            }
        }
        assert(0);
    }

    void HttpResponseDetails::DefaultResponse(CURL* http_easy_handler, evutil_socket_t fd, i32 action, void* handler, HttpResponsePerform* perform)
    {
        if (nullptr != handler)
        {
            HttpResponseDetails* watcher = static_cast<HttpResponseDetails*>(handler);
            {
                if (nullptr != watcher)
                {
                    watcher->DefaultResponse(http_easy_handler, fd, action, nullptr == perform ? new HttpResponsePerform(watcher->event_basic,
                                                                                                                         watcher->http_response_info,
                                                                                                                         watcher->http_multi_handler,
                                                                                                                         fd) : perform);
                }
            }
        }
    }

    void HttpResponseDetails::DefaultResponseTimer(CURLM* http_multi_handler, i32 delay, void* handler)
    {
        if (nullptr != handler)
        {
            HttpResponseDetails* watcher = static_cast<HttpResponseDetails*>(handler);
            {
                if (nullptr != watcher)
                {
                    if (0 == delay)
                    {
                        delay = 1;
                    }

                    watcher->DefaultResponseTimer(http_multi_handler, delay);
                }
            }
        }
    }
}