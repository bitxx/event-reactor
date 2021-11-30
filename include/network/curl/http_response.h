#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <event_core.h>
#include <event_curl.h>

namespace Evpp
{
    class EventLoop;

    class FileStream;
    class FileStreamResponse;

    class HttpResponse
    {
    public:
        explicit HttpResponse();
        virtual ~HttpResponse();
    public:
        virtual u96 OnResponse(void* buffer, u96 size, u96 nmemb) = 0;
        virtual i32 OnResponseProgress(double dl_file_max, double dl_file_min, double ul_file_max, double ul_file_min) = 0;
    public:
        static u96 OnResponse(void* buffer, u96 size, u96 nmemb, void* handler);
        static i32 OnResponseProgress(void* handler, double dl_file_max, double dl_file_min, double ul_file_max, double ul_file_min);
    };
}
#endif // __HTTP_RESPONSE_H__