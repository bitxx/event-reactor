#ifndef __EVENT_TIMER_H__
#define __EVENT_TIMER_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;

    class EventTimer : public EventLoopImpl, public EventStatus, public std::enable_shared_from_this<EventTimer>
    {
    public:
        explicit EventTimer(EventLoop* basic, const EventTimerHandler& callback = EventTimerHandler(), u96 index = 0);
        virtual ~EventTimer();
    public:
        bool InitedTimer();
        bool AssignTimer(u32 delay, u32 activate = 1);
        bool AssignTimer(const std::chrono::milliseconds& delay, u32 activate = 1);
    private:
        bool AssignTimer(const struct timeval* delay, u32 activate = 1);
    public:
        bool CancelTimer();
        bool KilledTimer();
    private:
        void OnNotify(evutil_socket_t fd, short flags);
        static void OnNotify(evutil_socket_t fd, short flags, void* data);
    private:
        EventLoop*                                                                              event_basic;
        u96                                                                                     event_index;
        std::shared_ptr<struct event>                                                           event_handle;
        EventTimerHandler                                                                       event_callback;
    };
}
#endif // __EVENT_TIMER_H__