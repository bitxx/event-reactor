#include <network/curl/http_response_perform.h>
#include <network/curl/http_response_info.h>

#include <event_poll.h>

namespace Evpp
{
    HttpResponsePerform::HttpResponsePerform(EventLoop* basic, const std::shared_ptr<HttpResponseInfo>& http_response_info, CURLM* http_multi_handler, evutil_socket_t fd) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_greater(std::make_shared<EventGreater>(basic,
                                                     fd,
                                                     std::bind(&HttpResponsePerform::OnNotify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))),
        http_response_info(http_response_info),
        http_multi_handler(http_multi_handler),
        http_multi_active(0),
        http_easy_descriptor(fd)
    {
        EVENT_TRACE("HttpResponsePerform::HttpResponsePerform");
        {
            if (event_greater->Create(EV_READ))
            {
                if (0 == curl_multi_assign(http_multi_handler, fd, this))
                {
                    EVENT_INFO("polling event initialized successfully");
                }
            }
        }
    }

    HttpResponsePerform::~HttpResponsePerform()
    {
        EVENT_TRACE("HttpResponsePerform::~HttpResponsePerform");
    }

    bool HttpResponsePerform::Attach()
    {
        if (nullptr == event_greater)
        {
            return false;
        }

        return event_greater->Attach();
    }

    bool HttpResponsePerform::Detach()
    {
        if (nullptr == event_greater)
        {
            return false;
        }

        if (event_greater->Detach())
        {
            return RunInQueue(std::bind([&, this] { delete this; }));
        }

        return false;
    }

    void HttpResponsePerform::OnNotify(EventLoop* loop, const std::shared_ptr<EventGreater>& greater, evutil_socket_t fd, int events)
    {
        if (nullptr == loop || nullptr == greater || INVALID_SOCKET == fd)
        {
            assert(0);
        }

        if (events & EV_READ)
        {
            events |= CURL_CSELECT_IN;
        }

        if (events & EV_WRITE)
        {
            events |= CURL_CSELECT_OUT;
        }

        switch (curl_multi_socket_action(http_multi_handler, fd, events, &http_multi_active))
        {
            case CURLMcode::CURLM_OK:                                                                              break;
            case CURLMcode::CURLM_BAD_HANDLE:               EVENT_ERROR("CURLMcode::CURLM_BAD_HANDLE");            break;
            case CURLMcode::CURLM_BAD_EASY_HANDLE:          EVENT_ERROR("CURLMcode::CURLM_BAD_EASY_HANDLE");       break;
            case CURLMcode::CURLM_OUT_OF_MEMORY:            EVENT_ERROR("CURLMcode::CURLM_OUT_OF_MEMORY");         break;
            case CURLMcode::CURLM_INTERNAL_ERROR:           EVENT_ERROR("CURLMcode::CURLM_INTERNAL_ERROR");        break;
            case CURLMcode::CURLM_BAD_SOCKET:               EVENT_ERROR("CURLMcode::CURLM_INTERNAL_ERROR");        break;
            case CURLMcode::CURLM_UNKNOWN_OPTION:           EVENT_ERROR("CURLMcode::CURLM_UNKNOWN_OPTION");        break;
            case CURLMcode::CURLM_ADDED_ALREADY:            EVENT_ERROR("CURLMcode::CURLM_ADDED_ALREADY");         break;
            case CURLMcode::CURLM_RECURSIVE_API_CALL:       EVENT_ERROR("CURLMcode::CURLM_RECURSIVE_API_CALL"); event_greater->Detach();   break;
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
}