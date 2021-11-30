#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__
#include <event_core.h>
#include <event_loop_impl.h>
namespace Evpp
{
    class EventLoop;
    class TcpConnectBasic;


    class TcpClient : public EventLoopImpl
    {
    public:
        explicit TcpClient(EventLoop* basic);
        explicit TcpClient(EventLoop* basic, const std::string& address, u32 port);
        virtual ~TcpClient();
    public:
        bool CreaterConnect();
        bool CreaterConnect(const std::string& address, u32 port);
        bool DestroyConnect();
    public:
        void SetConnectCallback(const InterfaceAccepts& connect);
        void SetMessageCallback(const InterfaceMessage& message);
        void SetDisconsCallback(const InterfaceDiscons& discons);
    private:
        bool ConnectServer(EventLoop* loop);
    private:
        bool CreateSession(EventLoop* loop, evutil_socket_t fd);
        bool DeleteSession();
    private:
        bool OnConnect(EventLoop* loop, struct bufferevent* bev, evutil_socket_t fd, i16 events, u32 status, u96 index);
        bool OnMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index);
        void OnDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index);
    private:
        EventLoop*                                                                              event_basic;
        std::string                                                                             event_address;
        u32                                                                                     event_port;
    private:
        InterfaceConnect                                                                        tcp_connect;
        InterfaceMessage                                                                        tcp_meesage;
        InterfaceDiscons                                                                        tcp_discons;
    private:
        std::shared_ptr<TcpConnectBasic>                                                        tcp_connect_basic;
        std::shared_ptr<TcpSession>                                                             tcp_session;
        std::mutex                                                                              tcp_session_mutex;
    };
}
#endif // __TCP_CLIENT_H__