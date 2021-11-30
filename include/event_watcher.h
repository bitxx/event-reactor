#ifndef __EVENT_WATCHER_H__
#define __EVENT_WATCHER_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventWatcher : public EventLoopImpl, public EventStatus
    {
    public:
        explicit EventWatcher(EventLoop* basic, Handler&& watcher_callback);
        virtual ~EventWatcher();
    public:
        bool CreaterWatcher();
        bool DestroyWatcher();
    public:
        bool SendNotify();
    private:
        void OnNotify(evutil_socket_t fd, short flags);
        static void OnNotify(evutil_socket_t fd, short flags, void* data);
    private:
        EventLoop*                                                                              event_basic;
        Handler                                                                                 event_watcher_callback;
        std::shared_ptr<struct event>                                                           event_handle;
        evutil_socket_t                                                                         event_descriptor[2];
    };
}
#endif // __EVENT_WATCHER_H__