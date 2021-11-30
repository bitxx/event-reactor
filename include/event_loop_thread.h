#ifndef __EVENT_THRHEAD_H__
#define __EVENT_THRHEAD_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;

    class EventLoopThread : public EventLoopImpl, public EventStatus
    {
    public:
        explicit EventLoopThread(EventLoop* basic, u96 index = 0);
        explicit EventLoopThread(EventLoop* basic, const EventThreadDestroyHandler& destroy, u96 index = 0);
        virtual ~EventLoopThread();
    public:
        bool CreaterThread(const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000), const EventThreadStarHandler& star = EventThreadStarHandler(), const EventThreadStopHandler& stop = EventThreadStopHandler());
        bool DestroyThread(const std::chrono::milliseconds& delay = std::chrono::milliseconds(30 * 1000));
    public:
        EventLoop* GetEventLoop();
    private:
        void ExecDispatch(const EventThreadStarHandler& star, const EventThreadStopHandler& stop);
    private:
        EventLoop*                                                                              event_basic;
        EventThreadDestroyHandler                                                               event_destroy;
        u96                                                                                     event_index;
        std::shared_ptr<EventLoop>                                                              event_loops;
        std::binary_semaphore                                                                   event_prepare;
        std::future<void>                                                                       event_safe_thread;
        std::shared_ptr<std::thread>                                                            event_thread;
    };
}
#endif // __EVENT_THRHEAD_H__