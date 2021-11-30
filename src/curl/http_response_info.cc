#include <network/curl/http_response_info.h>

namespace Evpp
{
    HttpResponseInfo::HttpResponseInfo(EventLoop* basic, const Handler& http_response_query) :
        EventLoopImpl(basic),
        event_basic(basic),
        http_response_query(http_response_query)
    {

    }

    HttpResponseInfo::~HttpResponseInfo() { }

    bool HttpResponseInfo::QueryResponseInfo(CURLM* http_multi_handler, i32 http_multi_active)
    {
        return QueryResponseInfo(http_multi_handler, curl_multi_info_read(http_multi_handler, &http_easy_queue), http_multi_active);
    }

    bool HttpResponseInfo::QueryResponseInfo(CURLM* http_multi_handler, CURLMsg* http_multi_msg, i32 http_multi_active)
    {
        if (nullptr != http_multi_msg)
        {
            if (CURLMSG::CURLMSG_DONE == http_multi_msg->msg)
            {
                if (CURLcode::CURLE_OK != curl_easy_getinfo(http_multi_msg->easy_handle, CURLINFO::CURLINFO_EFFECTIVE_URL, &http_easy_url))
                {
                    EVENT_ERROR("获取URL信息错误");
                }

                if (CURLMcode::CURLM_OK != curl_multi_remove_handle(http_multi_handler, http_multi_msg->easy_handle))
                {
                    return false;
                }

                if (nullptr != http_response_query)
                {
                    return RunInQueue(std::move(std::bind(http_response_query, http_easy_url, http_multi_msg->msg, http_multi_msg->easy_handle, http_multi_msg->data.result)));
                }

                return true;
            }
        }
        return false;
    }
}
