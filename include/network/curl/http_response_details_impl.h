#ifndef __HTTP_RESPONSE_DETAILS_IMPL_H__
#define __HTTP_RESPONSE_DETAILS_IMPL_H__

#include <event_core.h>
#include <event_curl.h>
#include <network/curl/http_response_details.h>

namespace Evpp
{
    class EventLoop;

    class HttpResponseDetailsImpl : public HttpResponseDetails
    {
    public:
        using Handler = std::function<void(u96, const std::string&, CURLMSG, CURL*, CURLcode)>;
    public:
        explicit HttpResponseDetailsImpl(EventLoop* basic, const Handler& http_response_query, u96 index = 0);
        virtual ~HttpResponseDetailsImpl();
    public:
        void QueryResponseInfo(const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result);
    private:
        EventLoop*                                                                              event_basic;
        u96                                                                                     event_index;
    private:
        Handler                                                                                 http_response_query;
    };
}
#endif // __HTTP_RESPONSE_DETAILS_IMPL_H__