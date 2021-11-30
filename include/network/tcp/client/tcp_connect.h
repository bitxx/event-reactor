#ifndef __TCP_CONNECT_H__
#define __TCP_CONNECT_H__
#include <event_core.h>
#include <event_loop_impl.h>
#include <event_plexing.h>
namespace Evpp
{
    class EventLoop;
    class EventLoopThreadPool;

    class TcpConnectBasic;

    class TcpConnect : public EventLoopImpl, public EventPlexing<TcpConnect, u96>
    {
    public:
        explicit TcpConnect(EventLoop* basic, const Details::InterfaceConnect& connect = Details::InterfaceConnect());
        virtual ~TcpConnect();
    public:
        bool InitialConnect(u96 thread_size);
        bool CreaterConnect();
        bool CreaterConnect(u96 index);
        bool DestroyConnect(u96 index, u32 isclose = 0);
        bool DestroyConnect(u32 isclose = 1);
    public:
        bool AddConnect(const std::string& address, u32 port);
        bool AddConnect(EventLoop* loop, const std::string& address, u32 port, u96 index);
    private:
        EventLoop*                                                                              event_basic;
        Details::InterfaceConnect                                                               event_connect;
        std::shared_ptr<EventLoopThreadPool>                                                    event_thread_pool;
    private:
        std::unordered_map<u96, std::shared_ptr<TcpConnectBasic>>                               tcp_connect_pool;
        std::mutex                                                                              tcp_connect_mutex;
    };
}
#endif // __TCP_CONNECT_H__