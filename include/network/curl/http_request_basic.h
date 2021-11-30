#ifndef __HTTP_REQUEST_BASIC_H__
#define __HTTP_REQUEST_BASIC_H__

#include <memory>
#include <unordered_map>

#include <event_core.h>
#include <event_curl.h>
#include <event_loop_impl.h>

namespace Evpp
{
    class EventLoop;

    class HttpResponseBasic;
    class HttpResponseDetailsImpl;
    class HttpRequestSetting;

    class HttpRequestBasic : public EventLoopImpl, public EventPlexing<HttpRequestBasic, u96>
    {
    public:
        using HttpResponseHandler = std::function<bool(u96, const std::string&, CURLcode)>;
    public:
        explicit HttpRequestBasic(EventLoop* basic, const HttpResponseHandler& response_handler);
        virtual ~HttpRequestBasic();
    public:
        bool CreaterRequest(const std::string& uri, CURL* http_easy_handler);
    public:
        bool DestroyRequest(const std::chrono::milliseconds& delay = std::chrono::milliseconds(30 * 1000));
    private:
        bool CreaterRequest(u96 index, const std::string& uri, CURL* http_easy_handler);
    private:
        bool AddResponse(u96 index, const std::string& uri);
        bool DelResponse(u96 index, const std::string& uri, CURLcode http_easy_result = CURLcode::CURLE_OK);
        bool DelResponse(const std::string& http_easy_url, CURL* http_easy_handler, CURLcode http_easy_result);
    private:
        void QueryResponseInfo(u96 index, const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result);
        static bool OnResponseLogging(CURL *http_easy_handler, curl_infotype type, char *data, size_t size, void *handler);
    private:
        EventLoop*                                                                              event_basic;
        std::binary_semaphore                                                                   event_prepare;
    private:
        HttpResponseHandler                                                                     http_response_handler;
        std::shared_ptr<HttpResponseDetailsImpl>                                                http_response_details;
        std::unordered_map<std::string, u96>                                                    http_response_session;
        std::mutex                                                                              http_mutex;
    };
}
#endif // __HTTP_REQUEST_BASIC_H__