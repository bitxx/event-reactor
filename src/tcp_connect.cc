#include <network/tcp/client/tcp_connect.h>
#include <network/tcp/client/tcp_connect_basic.h>

#include <event_loop_thread_pool.h>

namespace Evpp
{
    TcpConnect::TcpConnect(EventLoop* basic, const Details::InterfaceConnect& connect) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_thread_pool(std::make_shared<EventLoopThreadPool>(basic)),
        event_connect(connect)
    {
        EVENT_TRACE("TcpConnect::TcpConnect");
        {
            event_thread_pool->CreaterEventThreadPool();
        }
    }

    TcpConnect::~TcpConnect()
    {
        EVENT_TRACE("TcpConnect::~TcpConnect");
    }

    bool TcpConnect::InitialConnect(u96 thread_size)
    {
        if (nullptr == event_thread_pool)
        {
            return false;
        }

        return event_thread_pool->CreaterEventThreadPool(thread_size);
    }

    bool TcpConnect::CreaterConnect()
    {
        std::unique_lock<std::mutex> lock(tcp_connect_mutex);
        {
            for (const auto & [index, connect] : tcp_connect_pool)
            {
                if (connect->CreaterConnect())
                {
                    continue;
                }
                return false;
            }
        }

        return true;
    }

    bool TcpConnect::CreaterConnect(u96 index)
    {
        std::unique_lock<std::mutex> lock(tcp_connect_mutex);
        {
            if (tcp_connect_pool.find(index) != tcp_connect_pool.end())
            {
                return tcp_connect_pool[index]->CreaterConnect();
            }
        }

        return false;
    }

    bool TcpConnect::DestroyConnect(u96 index, u32 isclose)
    {
        std::unique_lock<std::mutex> lock(tcp_connect_mutex);
        {
            if (tcp_connect_pool.find(index) != tcp_connect_pool.end())
            {
                return tcp_connect_pool[index]->DestroyConnect(isclose);
            }
        }

        return false;
    }

    bool TcpConnect::DestroyConnect(u32 isclose)
    {
        std::unique_lock<std::mutex> lock(tcp_connect_mutex);
        {
            for (const auto & [index, connect] : tcp_connect_pool)
            {
                if (connect->DestroyConnect(isclose))
                {
                    continue;
                }
                return false;
            }
        }

        return event_thread_pool->DestroyEventThreadPool();
    }

    bool TcpConnect::AddConnect(const std::string& address, u32 port)
    {
        return AddConnect(event_thread_pool->GetNextLoop(), address, port, PlexingIndex());
    }

    bool TcpConnect::AddConnect(EventLoop* loop, const std::string& address, u32 port, u96 index)
    {
        std::unique_lock<std::mutex> lock(tcp_connect_mutex);
        {
            if (tcp_connect_pool.find(index) == tcp_connect_pool.end())
            {
                return tcp_connect_pool.try_emplace(index, std::make_shared<TcpConnectBasic>(loop,
                                                                                             index,
                                                                                             event_connect,
                                                                                             address,
                                                                                             port)).second;
            }
        }

        return false;
    }
}