#ifndef __HTTP_RESPONSE_DETAILS_H__
#define __HTTP_RESPONSE_DETAILS_H__
#include <memory>
#include <unordered_map>

#include <event_core.h>
#include <event_curl.h>
#include <event_status.h>
#include <event_loop_impl.h>

namespace Evpp
{
    class EventLoop;
    class EventTimer;

    class HttpResponseInfo;
    class HttpResponsePerform;

    class HttpResponseDetails : public EventLoopImpl, public EventStatus
    {
    public:
        using Handler = std::function<void(const std::string&, CURLMSG, CURL*, CURLcode)>;
    public:
        explicit HttpResponseDetails(EventLoop* basic, const Handler& http_response_query);
        virtual ~HttpResponseDetails();
    public: 
        friend class HttpResponsePerform;
    public:
        bool InitialEasyPerform();
        bool CreaterEasyPerform(CURL* http_easy_handler);
    private:
        void OnTimer(EventLoop* loop, const std::shared_ptr<EventTimer>& timer, u96 index);
        void OnTimerClose(EventLoop* loop, const std::shared_ptr<EventTimer>& timer, u96 index);
    private:
        void QueryResponseInfo(const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result);
    private:
        void DefaultResponse(CURL* http_easy_handler, evutil_socket_t fd, i32 action, HttpResponsePerform* perform);
        void DefaultResponseTimer(CURLM* http_multi_handler, i32 delay);
    private:
        static void DefaultResponse(CURL* http_easy_handler, evutil_socket_t fd, i32 action, void* handler, HttpResponsePerform* perform);
        static void DefaultResponseTimer(CURLM* http_multi_handler, i32 delay, void* handler);
    private:
        EventLoop*                                                                              event_basic;
        std::binary_semaphore                                                                   event_prepare;
        std::shared_ptr<EventTimer>                                                             event_timer;
    private:
        std::shared_ptr<HttpResponseInfo>                                                       http_response_info;
        Handler                                                                                 http_response_query;
        i32                                                                                     http_multi_active;
        CURLM*                                                                                  http_multi_handler;
        i32                                                                                     http_easy_queue;
    };
}
#endif // __HTTP_RESPONSE_DETAILS_H__