#ifndef __HTTP_DOWNLOAD_PERFORM_H__
#define __HTTP_DOWNLOAD_PERFORM_H__

#include <event_core.h>
#include <event_curl.h>
#include <event_loop_impl.h>

namespace Evpp
{
    class EventLoop;
    class EventGreater;

    class HttpResponseInfo;

    class HttpResponsePerform : public EventLoopImpl
    {
    public:
        using Handler = std::function<void(const std::string&, CURLMSG, CURL*, CURLcode)>;
    public:
        explicit HttpResponsePerform(EventLoop* basic, const std::shared_ptr<HttpResponseInfo>& http_response_info, CURLM* http_multi_handler, evutil_socket_t fd);
        virtual ~HttpResponsePerform();
    public:
        bool Attach();
        bool Detach();
    private:
        void OnNotify(EventLoop* loop, const std::shared_ptr<EventGreater>& greater, evutil_socket_t fd, int events);
    private:
        EventLoop*                                                                              event_basic;
        std::shared_ptr<EventGreater>                                                           event_greater;
    private:
        std::shared_ptr<HttpResponseInfo>                                                       http_response_info;
        CURLM*                                                                                  http_multi_handler;
        i32                                                                                     http_multi_active;
        evutil_socket_t                                                                         http_easy_descriptor;
    };
}
#endif // __HTTP_DOWNLOAD_PERFORM_H__