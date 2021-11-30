#include <network/tcp/client/tcp_client_service.h>
#include <network/tcp/client/tcp_client.h>
#include <event_loop.h>

namespace Evpp
{
    TcpClientService::TcpClientService(EventLoop* basic) :
        TcpClientService(basic, std::string(), 0)

    {

    }

    TcpClientService::TcpClientService(EventLoop* basic, const std::string& address, u32 port) :
        EventLoopImpl(basic),
        event_basic(basic),
        tcp_client(std::make_unique<TcpClient>(basic))
    {
        
    }

    TcpClientService::~TcpClientService()
    {

    }

    bool TcpClientService::CreaterConnect(const std::string& address, u32 port)
    {
        if (nullptr == tcp_client)
        {
            return false;
        }

        return tcp_client->CreaterConnect(address, port);
    }

    bool TcpClientService::DestroyConnect()
    {
        if (nullptr == tcp_client)
        {
            return false;
        }

        return tcp_client->DestroyConnect();
    }

    void TcpClientService::SetConnectCallback(const InterfaceConnect& connect)
    {
        if (nullptr == tcp_client)
        {
            return;
        }

        return tcp_client->SetConnectCallback(connect);
    }

    void TcpClientService::SetMessageCallback(const InterfaceMessage& message)
    {
        if (nullptr == tcp_client)
        {
            return;
        }

        return tcp_client->SetMessageCallback(message);
    }

    void TcpClientService::SetDisconsCallback(const InterfaceDiscons& discons)
    {
        if (nullptr == tcp_client)
        {
            return;
        }

        return tcp_client->SetDisconsCallback(discons);
    }
}
