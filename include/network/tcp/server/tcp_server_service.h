#ifndef __TCP_SERVER_SERVICE_H__
#define __TCP_SERVER_SERVICE_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventThreadPool;

    class TcpServer;

    class TcpServerService : public EventLoopImpl
    {
    public:
        explicit TcpServerService(EventLoop* basic);
        virtual ~TcpServerService();
    public:
        bool CreaterListen(const std::string& address);
        bool CreaterServer(u96 thread_size, const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000), const EventThreadStarHandler& star = EventThreadStarHandler(), const EventThreadStopHandler& stop = EventThreadStopHandler());
        bool DestroyServer(const std::chrono::milliseconds& delay = std::chrono::milliseconds(3 * 1000));
    public:
        bool Send(u96 index, const std::string& buffer);
    public:
        bool Close(u96 index, const Handler& callback = Handler());
    public:
        void SetAcceptsCallback(const InterfaceAccepts& accepts);
        void SetMessageCallback(const InterfaceMessage& message);
        void SetDisconsCallback(const InterfaceDiscons& discons);
    protected:
        EventLoop*                                                                              event_basic;
    private:
        std::unique_ptr<TcpServer>                                                              tcp_server;
    };

}
#endif