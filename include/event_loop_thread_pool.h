#ifndef __EVENT_LOOP_THREAD_POOL_H__
#define __EVENT_LOOP_THREAD_POOL_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventLoopThread;
    class EventLoopThreadPool : public EventLoopImpl, public EventPlexing<EventLoopThreadPool, u96>
    {
    public:
        explicit EventLoopThreadPool(EventLoop* basic);
        explicit EventLoopThreadPool(EventLoop* basic, u96 size);
        virtual ~EventLoopThreadPool();
    public:
        bool CreaterEventThreadPool(const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000), const EventThreadStarHandler& star = EventThreadStarHandler(), const EventThreadStopHandler& stop = EventThreadStopHandler());
        bool CreaterEventThreadPool(u96 thread_size, const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000), const EventThreadStarHandler& star = EventThreadStarHandler(), const EventThreadStopHandler& stop = EventThreadStopHandler());
        bool DestroyEventThreadPool(const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000));
    public:
        EventLoop* GetNextLoop();
        EventLoop* GetNextLoop(u96 index);
    private:
        EventLoop* DoGetNextLoop(u96 index);
    private:
        bool InitialEventThreadPool(const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop);
    private:
        bool CreaterEventThread(u96 index);
        bool DestroyEventThread(u96 index);
    private:
        EventLoop*                                                                              event_basic;
        u96                                                                                     event_size;
        std::atomic<u96>                                                                        event_index;
        std::unordered_map<u96, std::shared_ptr<EventLoopThread>>                               event_loops;
    private:
        std::recursive_mutex                                                                    event_mutex;
    };
}
#endif // __EVENT_LOOP_THREAD_POOL_H__