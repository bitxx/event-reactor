#ifndef __EVENT_CALLBACK_H__
#define __EVENT_CALLBACK_H__
#include <iostream>
#include <functional>
#include <memory>
namespace Evpp
{
    class EventLoop;
    class EventConn;
    class EventPoll;
    class EventFamily;
    class EventForward;;
    class EventTimer;
    class EventBuffer;

    class TcpSession;

    using Handler                       = std::function<void()>;
    using Functor                       = std::function<bool()>;
    using InterfaceListener             = std::function<bool(EventLoop*, struct evconnlistener*, evutil_socket_t, struct sockaddr*)>;

    using EventThreadStarHandler        = std::function<bool(EventLoop*, u96)>;
    using EventThreadStopHandler        = std::function<bool(u96)>;
    using EventThreadDestroyHandler     = std::function<bool(u96)>;

    using EventTimerHandler             = std::function<void(EventLoop*, const std::shared_ptr<EventTimer>&, u96)>;

    using InterfaceFiles                = std::function<bool(const std::string&, const std::uintmax_t, const std::chrono::seconds&)>;

    using InterfaceAccepts              = std::function<bool(EventLoop*, const std::shared_ptr<TcpSession>&, const u96)>;
    using InterfaceConnect              = std::function<bool(EventLoop*, const std::shared_ptr<TcpSession>&, const u96)>;
    using InterfaceDiscons              = std::function<void(EventLoop*, const std::shared_ptr<TcpSession>&, const u96)>;
    using InterfaceMessage              = std::function<bool(EventLoop*, const std::shared_ptr<TcpSession>&, const std::shared_ptr<EventBuffer>&, const u96)>;

    namespace Details
    {
        using InterfaceMessages         = std::function<bool(EventLoop*, const std::shared_ptr<EventBuffer>&)>;
        using InterfaceDiscon           = std::function<void()>;
        using InterfaceConnect          = std::function<bool(EventLoop*, struct bufferevent*, evutil_socket_t, i16, u32, u96)>;
    }

}
#endif // __EVENT_CALLBACK_H__