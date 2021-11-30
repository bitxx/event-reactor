#include <network/curl/http_response.h>

#include <event_loop.h>

namespace Evpp
{
    HttpResponse::HttpResponse()
    {
        
    }

    HttpResponse::~HttpResponse()
    {
        
    }

    u96 HttpResponse::OnResponse(void* buffer, u96 size, u96 nmemb, void* handler)
    {
        if (nullptr != handler)
        {
            HttpResponse* watcher = static_cast<HttpResponse*>(handler);
            {
                if (nullptr != watcher)
                {
                    return watcher->OnResponse(buffer, size, nmemb);
                }
            }
        }
        return 0;
    }

    i32 HttpResponse::OnResponseProgress(void* handler, double dl_file_max, double dl_file_min, double ul_file_max, double ul_file_min)
    {
        if (nullptr != handler)
        {
            HttpResponse* watcher = static_cast<HttpResponse*>(handler);
            {
                if (nullptr != watcher)
                {
                    return watcher->OnResponseProgress(dl_file_max, dl_file_min, ul_file_max, ul_file_min);
                }
            }
        }
        return 0;
    }
}