#ifndef __HTTP_RESPONSE_INFO_H__
#define __HTTP_RESPONSE_INFO_H__

#include <event_core.h>
#include <event_curl.h>
#include <event_loop_impl.h>

namespace Evpp
{
    class EventLoop;

    class HttpResponseInfo : public EventLoopImpl
    {
    public:
        using Handler = std::function<void(const std::string&, CURLMSG, CURL*, CURLcode)>;
    public:
        explicit HttpResponseInfo(EventLoop* basic, const Handler& http_response_query);
        virtual ~HttpResponseInfo();
    public:
        bool QueryResponseInfo(CURLM* http_multi_handler, i32 http_multi_active);
    private:
        bool QueryResponseInfo(CURLM* http_multi_handler, CURLMsg* http_multi_msg, i32 http_multi_active);
    private:
        EventLoop*                                                                              event_basic;
    private:
        Handler                                                                                 http_response_query;
        char*                                                                                   http_easy_url;
        i32                                                                                     http_easy_queue;
    };
}
#endif // __HTTP_DOWNLOAD_INFO_H__