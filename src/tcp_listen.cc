#include <network/tcp/server/tcp_listen.h>
#include <event_poll.h>
#include <event_socket.h>
#include <event_loop.h>
namespace Evpp
{
    TcpListen::TcpListen(EventLoop* basic, const InterfaceListener& callback) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_descriptor(INVALID_SOCKET),
        event_listenner_callback(callback),
        event_socket_sa(struct sockaddr_storage()),
        event_listenner(nullptr)
    {

    }

    TcpListen::~TcpListen()
    {
        EVENT_TRACE("TcpListen::~TcpListen");
    }

    bool TcpListen::CreaterListen(const std::string& socket_addr)
    {
        if (nullptr == event_listenner)
        {
           /*
            * LEV_OPT_LEAVE_SOCKETS_BLOCKING 默认情况下，连接监听器接收新套接字后，会将其设置为非阻塞的，以便将其用于libevent。如果不想要这种行为，可以设置这个标志。
            * LEV_OPT_CLOSE_ON_FREE 如果设置了这个选项，释放连接监听器会关闭底层套接字。
            * LEV_OPT_CLOSE_ON_EXEC 如果设置了这个选项，连接监听器会为底层套接字设置close-on-exec标志。更多信息请查看fcntl和FD_CLOEXEC的平台文档。
            * LEV_OPT_REUSEABLE 某些平台在默认情况下，关闭某监听套接字后，要过一会儿其他套接字才可以绑定到同一个端口。设置这个标志会让libevent标记套接字是可重用的，这样一旦关闭，可以立即打开其他套接字，在相同端口进行监听。
            * LEV_OPT_THREADSAFE 为监听器分配锁，这样就可以在多个线程中安全地使用了。这是2.0.8-rc的新功能。
            * LEV_OPT_DISABLED 初始化监听器以将其禁用，而不是启用。 您可以使用evconnlistener_enable（）手动将其打开。 Libevent 2.1.1-alpha中的新功能
            */

            if (socket_addr.size())
            {
                if (0 != evutil_parse_sockaddr_port(socket_addr.c_str(), reinterpret_cast<sockaddr*>(std::addressof(event_socket_sa)), const_cast<i32*>(std::addressof(static_cast<const i32&>(int(sizeof(struct sockaddr_storage)))))))
                {
                    throw std::runtime_error("parse sockaddr port error");
                }
            }

            event_listenner = evconnlistener_new_bind(event_basic->EventBasic(),
                                                      &TcpListen::OnNotify,
                                                      this,
                                                      LEV_OPT_THREADSAFE | LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                                      SOMAXCONN,
                                                      reinterpret_cast<const sockaddr*>(std::addressof(event_socket_sa)),
                                                      sizeof(sockaddr_storage));
        }

        if (nullptr == event_listenner)
        {
            EVENT_ERROR("创建监听对象失败");
        }

        return 0 != event_listenner;
    }

    bool TcpListen::DestroyListen()
    {
        if (nullptr != event_listenner)
        {
            if (0 == evconnlistener_disable(event_listenner))
            {
                evconnlistener_free(event_listenner);
                event_listenner = nullptr;

                return true;
            }
            assert(0);
        }

        return false;
    }

    void TcpListen::OnNotify(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen)
    {
        if (nullptr == listener || INVALID_SOCKET == fd || nullptr == addr || 0 == socklen)
        {
            return;
        }

        if (listener != event_listenner)
        {
            return;
        }

        if (nullptr != event_listenner_callback)
        {
            if (0 == evutil_make_socket_nonblocking(fd))
            {
                if (event_listenner_callback(event_basic, listener, fd, addr))
                {
                    return;
                }
                else
                {
                    EVENT_WARN("the callback function is executed abnormally, and the remote client connection is closed");
                }
            }

            if (CloseSocketHandler(fd))
            {
                EVENT_ERROR("set the file descriptor to be asynchronous and an error occurs");
            }
        }
    }

    void TcpListen::OnNotify(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* data)
    {
        if (nullptr != data)
        {
            TcpListen* watcher = static_cast<TcpListen*>(data);
            {
                if (nullptr == watcher)
                {
                    return;
                }

                return watcher->OnNotify(listener, fd, addr, socklen);
            }
        }
    }
}