#include <network/tcp/server/tcp_server_service.h>
#include <network/tcp/server/tcp_server.h>

#include <event_loop.h>
namespace Evpp
{
    TcpServerService::TcpServerService(EventLoop* basic) :
        EventLoopImpl(basic),
        event_basic(basic),
        tcp_server(std::make_unique<TcpServer>(basic))
    {

    }

    TcpServerService::~TcpServerService()
    {

    }

    bool TcpServerService::CreaterListen(const std::string& address)
    {
        if (nullptr == tcp_server)
        {
            return false;
        }

        return tcp_server->CreaterListen(address);
    }

    bool TcpServerService::CreaterServer(u96 thread_size, const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        if (nullptr == tcp_server)
        {
            return false;
        }

        return tcp_server->CreaterServer(thread_size, delay, star, stop);
    }

    bool TcpServerService::DestroyServer(const std::chrono::milliseconds& delay)
    {
        if (nullptr == tcp_server)
        {
            return false;
        }

        return tcp_server->DestroyServer(delay);
    }

    bool TcpServerService::Send(u96 index, const std::string& buffer)
    {
        if (nullptr == tcp_server)
        {
            return false;
        }

        return tcp_server->Send(index, buffer);
    }

    bool TcpServerService::Close(u96 index, const Handler& callback)
    {
        if (nullptr == tcp_server)
        {
            return false;
        }

        return tcp_server->Close(index, callback);
    }

    void TcpServerService::SetAcceptsCallback(const InterfaceAccepts& accepts)
    {
        if (nullptr == tcp_server)
        {
            return;
        }

        return tcp_server->SetAcceptsCallback(accepts);
    }

    void TcpServerService::SetMessageCallback(const InterfaceMessage& message)
    {
        if (nullptr == tcp_server)
        {
            return;
        }

        return tcp_server->SetMessageCallback(message);
    }

    void TcpServerService::SetDisconsCallback(const InterfaceDiscons& discons)
    {
        if (nullptr == tcp_server)
        {
            return;
        }

        return tcp_server->SetDisconsCallback(discons);
    }
}