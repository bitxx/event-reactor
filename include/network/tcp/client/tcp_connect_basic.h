#ifndef __TCP_CONNECT_BASIC_H__
#define __TCP_CONNECT_BASIC_H__

#include <event_core.h>
#include <event_loop_impl.h>
#include <event_status.h>

namespace Evpp
{
    class EventLoop;

    class TcpConnectBasic : public EventLoopImpl, public EventStatus
    {
    public:
        explicit TcpConnectBasic(EventLoop* basic, const Details::InterfaceConnect& connect = Details::InterfaceConnect(), const std::string& address = std::string(), u32 port = 0);
        explicit TcpConnectBasic(EventLoop* basic, u96 index, const Details::InterfaceConnect& connect = Details::InterfaceConnect(), const std::string& address = std::string(), u32 port = 0);
        virtual ~TcpConnectBasic();
    public:
        bool CreaterConnect();
        bool CreaterConnect(const std::string& address, u32 port);
    private:
        bool CreaterConnect(struct bufferevent* bev, const std::string& address, u32 port);
    public:
        bool DestroyConnect(u32 isclose = 1);
    private:
        void OnEventNotify(struct bufferevent* bev, short events);
    private:
        EventLoop*                                                                              event_basic;
        u96                                                                                     event_index;
        Details::InterfaceConnect                                                               event_connect;
        std::string                                                                             event_address;
        u32                                                                                     event_port;
        struct bufferevent*                                                                     event_buffer;
        struct evdns_base*                                                                      event_dns;
        evutil_socket_t                                                                         event_socket;
    };
}
#endif // __TCP_CONNECT_BASIC_H__