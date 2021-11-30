#ifndef __TCP_LISTEN_H__
#define __TCP_LISTEN_H__
#include <event_core.h>

namespace Evpp
{
    class EventLoop;
    class EventPoll;
    class EventAccept;
    class EventFamily;

    class TcpListen : public EventLoopImpl, public EventStatus
    {
    public:
        explicit TcpListen(EventLoop* basic, const InterfaceListener& callback = InterfaceListener());
        virtual ~TcpListen();
    public:
        bool CreaterListen(const std::string& socket_addr = std::string());
        bool DestroyListen();
    private:
        void OnNotify(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen);
        static void OnNotify(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* data);
    private:
        EventLoop*                                                                              event_basic;
        evutil_socket_t                                                                         event_descriptor;
        InterfaceListener                                                                       event_listenner_callback;
        struct sockaddr_storage                                                                 event_socket_sa;
        struct evconnlistener*                                                                  event_listenner;
    };
}
#endif // __TCP_LISTEN_H__