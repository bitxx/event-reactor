#ifndef __TCP_CLIENT_SERVICE_H__
#define __TCP_CLIENT_SERVICE_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;

    class TcpClient;

    class TcpClientService : public EventLoopImpl
    {
    public:
        explicit TcpClientService(EventLoop* basic);
        explicit TcpClientService(EventLoop* basic, const std::string& address, u32 port);
        virtual ~TcpClientService();
    public:
        bool CreaterConnect(const std::string& address, u32 port);
        bool DestroyConnect();
    public:
        void SetConnectCallback(const InterfaceConnect& connect);
        void SetMessageCallback(const InterfaceMessage& message);
        void SetDisconsCallback(const InterfaceDiscons& discons);
    private:
        EventLoop*                                                                              event_basic;
    private:
        std::unique_ptr<TcpClient>                                                              tcp_client;
    };
}
#endif