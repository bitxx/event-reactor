#ifndef __HTTP_DOWNLOAD_SESSION_H__
#define __HTTP_DOWNLOAD_SESSION_H__
#include <string>

#include <event_core.h>
#include <event_curl.h>

#include <network/curl/http_response.h>

namespace Evpp
{
    class EventLoop;

    class FileStream;
    class FileStreamResponse;

    class HttpResponseBasic : public HttpResponse
    {
    public:
        using ResponseProgressHandler = std::function<i32(const std::string&, double)>;
    public:
        explicit HttpResponseBasic(const std::string& uri, const std::string& path, CURL* handler, const ResponseProgressHandler& response_progress = ResponseProgressHandler());
        virtual ~HttpResponseBasic();
    public:
        virtual u96 OnResponse(void* buffer, u96 size, u96 nmemb) override;
        virtual i32 OnResponseProgress(double dl_file_max, double dl_file_min, double ul_file_max, double ul_file_min) override;
    private:
        std::string                                                                                 http_uri;
        CURL*                                                                                       http_handler;
        ResponseProgressHandler                                                                     http_response_progress;
        i32                                                                                         http_response_code;
        i32                                                                                         http_response_cache;
        std::ofstream                                                                               http_stream;
    };
}
#endif // __HTTP_DOWNLOAD_SESSION_H__