#ifndef __EVENT_PLEXING_H__
#define __EVENT_PLEXING_H__
#include <event_core.h>
namespace Evpp
{
    template<typename _Ty1, typename _Ty2>
    class EventPlexing
    {
    public:
        explicit EventPlexing() {};
        virtual ~EventPlexing() {};
    public:
        friend _Ty1;
    public:
        virtual _Ty2 PlexingIndex(_Ty2 index = (std::numeric_limits<_Ty2>::max)())
        {
            std::unique_lock<std::recursive_mutex> lock(tcp_plexing_mutex);
            {
                if (index == (std::numeric_limits<_Ty2>::max)())
                {
                    return PlexingIndex(tcp_plexing_index.empty() ? tcp_index.fetch_add(1) : tcp_plexing_index.top());
                }

                if (tcp_plexing_index.size())
                {
                    tcp_plexing_index.pop();
                }
            }
            return index;
        }
    public:
        void ResumeIndex(_Ty2 index)
        {
            std::unique_lock<std::recursive_mutex> lock(tcp_plexing_mutex);
            {
                tcp_plexing_index.emplace(index);
            }
        }
    private:
        std::atomic<_Ty2>                                                                        tcp_index;
        std::recursive_mutex                                                                     tcp_plexing_mutex;
        std::priority_queue<_Ty2, std::deque<_Ty2>, std::greater<_Ty2>>                          tcp_plexing_index;
    };
}
#endif // __EVENT_PLEXING_H__