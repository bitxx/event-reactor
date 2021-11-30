#include <network/tcp/server/tcp_listen_pool.h>
#include <network/tcp/server/tcp_listen.h>

#include <event_loop.h>
#include <event_loop_thread_pool.h>
namespace Evpp
{
    TcpListenPool::TcpListenPool(EventLoop* basic, const InterfaceListener& callback) :
        event_basic(basic),
        event_listenner_callback(callback),
        event_thread_pool(std::make_shared<EventLoopThreadPool>(basic))
    {

    }

    TcpListenPool::~TcpListenPool()
    {

    }

    bool TcpListenPool::InitialListen()
    {
        if(nullptr == event_thread_pool)
        {
            return false;
        }

        return event_thread_pool->CreaterEventThreadPool(0);
    }

    bool TcpListenPool::DestroyListen()
    {
        if (nullptr == event_thread_pool)
        {
            return false;
        }

        if (event_listenner_pool.empty())
        {
            return false;
        }

        for (const auto& [address, listen] : event_listenner_pool)
        {
            if (listen->DestroyListen())
            {
                continue;
            }

            return false;
        }

        return event_thread_pool->DestroyEventThreadPool();
    }

    bool TcpListenPool::CreaterListen()
    {
        if(event_listenner_pool.empty())
        {
            return false;
        }

        for(const auto& [address, listen] : event_listenner_pool)
        {
            if(listen->CreaterListen(address))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    bool TcpListenPool::CreaterListen(const std::string& address)
    {
        if (event_listenner_pool.find(address) == event_listenner_pool.end())
        {
            return event_listenner_pool.try_emplace(address, std::make_shared<TcpListen>(event_thread_pool->GetNextLoop(), event_listenner_callback)).second;
        }

        return false;
    }
}
