#ifndef __TCP_LISTEN_POOL_H__
#define __TCP_LISTEN_POOL_H__

#include <event_core.h>

namespace Evpp
{
    class EventLoop;
    class EventLoopThreadPool;
    class TcpListen;

    class TcpListenPool
    {
    public:
        explicit TcpListenPool(EventLoop* basic, const InterfaceListener& callback);
        virtual ~TcpListenPool();
    public:
        bool InitialListen();
        bool CreaterListen();
        bool CreaterListen(const std::string& address);
        bool DestroyListen();
    private:
        EventLoop*                                                                              event_basic;
        InterfaceListener                                                                       event_listenner_callback;
        std::shared_ptr<EventLoopThreadPool>                                                    event_thread_pool;
    private:
        std::unordered_map<std::string, std::shared_ptr<TcpListen>>                             event_listenner_pool;
    };
}

#endif