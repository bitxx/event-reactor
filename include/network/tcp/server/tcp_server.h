#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventLoopThread;
    class EventLoopThreadPool;

    class TcpListen;
    class TcpListenPool;
    class TcpSession;

    class TcpServer : public EventLoopImpl, public EventStatus, public EventPlexing<TcpServer, u96>
    {
    public:
        explicit TcpServer(EventLoop* basic);
        virtual ~TcpServer();
    public:
        bool CreaterListen(const std::string& address);
        bool CreaterServer(u96 thread_size, const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000), const EventThreadStarHandler& star = EventThreadStarHandler(), const EventThreadStopHandler& stop = EventThreadStopHandler());
        bool DestroyServer(const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000));
    public:
        bool Send(u96 index, const std::string& buffer);
    public:
        bool Close(u96 index, const Handler& callback);
    public:
        void SetAcceptsCallback(const InterfaceAccepts& accepts);
        void SetMessageCallback(const InterfaceMessage& message);
        void SetDisconsCallback(const InterfaceDiscons& discons);
    private:
        bool CreateSession(EventLoop* loop, evutil_socket_t fd, struct sockaddr* addr, u96 index);
        bool DeleteSession(u96 index);
        bool DeleteSession();
        TcpSession* GetSession(u96 index);
    private:
        bool DefaultAccepts(EventLoop* loop, struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr);
        bool DefaultMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index);
        void DefaultDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index);
    private:
        EventLoop*                                                                              event_basic;
        std::shared_ptr<EventLoopThreadPool>                                                    event_thread_pool;
    private:
        std::unique_ptr<TcpListenPool>                                                          tcp_listen;
    private:
        std::unordered_map<u96, std::shared_ptr<TcpSession>>                                    tcp_session_pool;
        std::mutex                                                                              tcp_current_mutex;
        std::mutex                                                                              tcp_destroy_mutex;
    private:
        InterfaceAccepts                                                                        tcp_accepts;
        InterfaceMessage                                                                        tcp_meesage;
        InterfaceDiscons                                                                        tcp_discons;
    };
}
#endif // __TCP_SERVER_H__