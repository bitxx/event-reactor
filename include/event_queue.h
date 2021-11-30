#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventWatcher;

    class EventQueue : public EventStatus
    {
    public:
        explicit EventQueue(EventLoop* basic);
        virtual ~EventQueue();
    public:
        bool CreaterQueue();
        bool DestroyQueue();
    public:
        bool RunInAfter(Handler&& function);
        bool RunInBlock(Handler&& function);
        bool RunInQueue(Handler&& function);
    private:
        bool BlockHandler(Handler&& function);
        void BlockHandler();
        bool NoBlockHandler(Handler&& function);
        void NoBlockHandler();
    private:
        EventLoop*                                                                              event_basic;
        std::shared_ptr<EventWatcher>                                                           event_watcher_block;
        std::shared_ptr<EventWatcher>                                                           event_watcher_noblock;
    private:
        std::vector<Handler>                                                                    event_queue_block;
        std::vector<Handler>                                                                    event_queue_noblock;
        std::mutex                                                                              event_queue_mutex_block;
        std::mutex                                                                              event_queue_mutex_noblock;
        std::counting_semaphore<1>                                                              event_prepare;
    };
}
#endif // __EVENT_QUEUE_H__