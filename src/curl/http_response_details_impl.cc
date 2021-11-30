#include <network/curl/http_response_details_impl.h>

namespace Evpp
{
    HttpResponseDetailsImpl::HttpResponseDetailsImpl(EventLoop* basic, const Handler& http_response_query, u96 index) :
        HttpResponseDetails(basic,
                            std::bind(&HttpResponseDetailsImpl::QueryResponseInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
        event_basic(basic),
        event_index(index),
        http_response_query(http_response_query)
    {
        EVENT_TRACE("HttpResponseDetailsImpl::HttpResponseDetailsImpl");
    }

    HttpResponseDetailsImpl::~HttpResponseDetailsImpl() 
    {
        EVENT_TRACE("HttpResponseDetailsImpl::~HttpResponseDetailsImpl");
    }

    void HttpResponseDetailsImpl::QueryResponseInfo(const std::string& http_easy_url, CURLMSG http_multi_msg, CURL* http_easy_handler, CURLcode http_easy_result)
    {
        if (nullptr != http_response_query)
        {
            http_response_query(event_index, http_easy_url, http_multi_msg, http_easy_handler, http_easy_result);
        }
    }
}