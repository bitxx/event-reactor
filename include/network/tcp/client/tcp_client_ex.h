#ifndef __TCP_CLIENT_EX_H__
#define __TCP_CLIENT_EX_H__
#include <event_core.h>
#include <event_loop_impl.h>
#include <event_status.h>
namespace Evpp
{
    class EventLoop;
    class EventBuffer;
    class EventLoopThreadPool;

    class TcpSession;
    class TcpConnect;

    class TcpClientEx : public EventLoopImpl, public EventStatus
    {
    public:
        explicit TcpClientEx(EventLoop* basic);
        virtual ~TcpClientEx();
    public:
        bool InitialConnect(u96 thread_size);
        bool CreaterConnect();
        bool DestroyConnect();
        bool ConnectServers(u96 index, u32 isclose = 1);
    public:
        bool AddConnect(const std::string& address, u32 port);
    public:
        bool Send(u96 index, const std::string& buffer);
        bool Close(u96 index, const Handler& callback = Handler());
    public:
        void SetConnectCallback(const InterfaceConnect& connect);
        void SetMessageCallback(const InterfaceMessage& message);
        void SetDisconsCallback(const InterfaceDiscons& discons);
    private:
        bool CreateSession(EventLoop* loop, u96 index, evutil_socket_t fd);
        bool SwitchSession(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index, evutil_socket_t fd);
        bool DeleteSession(u96 index);
        TcpSession* GetSession(u96 index);
    private:
        bool OnConnect(EventLoop* loop, struct bufferevent* bev, evutil_socket_t fd, i16 events, u32 status, u96 index);
        bool OnMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index);
        void OnDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index);
    private:
        EventLoop*                                                                              event_basic;
    private:
        InterfaceConnect                                                                        tcp_connect;
        InterfaceMessage                                                                        tcp_meesage;
        InterfaceDiscons                                                                        tcp_discons;
    private:
        std::unordered_map<u96, std::shared_ptr<TcpSession> >                                   tcp_session_pool;
        std::shared_ptr<TcpConnect>                                                             tcp_connect_pool;
        std::recursive_mutex                                                                    tcp_session_mutex;
    };
}
#endif // __TCP_CLIENT_EX_H__