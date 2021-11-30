#ifndef __EVEMT_POLL_H__
#define __EVEMT_POLL_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventAccept;
    class EventForward;

    class EventPoll : public EventLoopImpl, public EventStatus
    {
    public:
        explicit EventPoll(EventLoop* basic);
        explicit EventPoll(EventLoop* basic, evutil_socket_t fd);
        virtual ~EventPoll();
    public:
        bool Create(short flags);
        bool Create(short flags, evutil_socket_t fd);
        bool Attach();
        bool Detach();
    private:
        virtual void OnNotify(evutil_socket_t fd, short events) {};
        static void OnNotify(evutil_socket_t fd, short events, void* data);
    private:
        EventLoop*                                                                              event_basic;
        std::shared_ptr<struct event>                                                           event_handle;
        evutil_socket_t                                                                         event_descriptor;
        short                                                                                   event_flags;
    };


    class EventGreater final : public EventPoll, public std::enable_shared_from_this<EventGreater>
    {
    public:
        using Handler = std::function<void(EventLoop*, const std::shared_ptr<EventGreater>&, evutil_socket_t, short)>;
    public:
        explicit EventGreater(EventLoop* basic, const Handler& notify);
        explicit EventGreater(EventLoop* basic, evutil_socket_t fd, const Handler& notify);
        virtual ~EventGreater();
    private:
        virtual void OnNotify(evutil_socket_t fd, short events) override;
    private:
        EventLoop*                                                                              event_basic;
        Handler                                                                                 event_notify;
    };
}
#endif // __EVEMT_POLL_H__