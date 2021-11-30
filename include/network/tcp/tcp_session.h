#ifndef __TCP_CONN_H__
#define __TCP_CONN_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventTimer;
    class TcpMessage;

    class TcpSession : public EventLoopImpl, public std::enable_shared_from_this<TcpSession>
    {
    public:
        explicit TcpSession(EventLoop* basic, evutil_socket_t fd, struct sockaddr* addr, u96 index,  const InterfaceMessage& message_callback, const InterfaceDiscons& discons_callback);
        virtual ~TcpSession();
    public:
        bool InitialSession();
    public:
        bool Send(const char* buffer, u96 size);
        bool Send(const std::string& buffer);
        bool Close(const Handler& callback = Handler());
    public:
        const std::string& GetHost();
    private:
        bool OnMessages(EventLoop* loop, const std::shared_ptr<EventBuffer>& buffer);
        void OnDiscon();
    private:
        EventLoop*                                                                              event_basic;
        evutil_socket_t                                                                         event_descriptor;
        u96                                                                                     event_index;
        InterfaceMessage                                                                        event_message;
        InterfaceDiscons                                                                        event_discons;
        std::string                                                                             event_address;
        u32                                                                                     event_port;
        std::shared_ptr<EventTimer>                                                             event_timer;
    private:
        std::shared_ptr<TcpMessage>                                                             tcp_message;
    };
}
#endif // __TCP_CONN_H__