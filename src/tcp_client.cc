#include <network/tcp/client/tcp_client.h>
#include <network/tcp/client/tcp_connect_basic.h>
#include <network/tcp/tcp_session.h>

#include <event_loop.h>

namespace Evpp
{
    TcpClient::TcpClient(EventLoop* basic) :
        TcpClient(basic, std::string(), 0)
    {

    }

    TcpClient::TcpClient(EventLoop* basic, const std::string& address, u32 port) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_address(address),
        event_port(port),
        tcp_connect_basic(std::make_shared<TcpConnectBasic>(basic,
                                                      std::bind(&TcpClient::OnConnect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)))
    {
        EVENT_TRACE("TcpClient::TcpClient");
    }

    TcpClient::~TcpClient()
    {
        EVENT_TRACE("TcpClient::~TcpClient");
    }

    bool TcpClient::CreaterConnect()
    {
        return CreaterConnect(event_address, event_port);
    }

    bool TcpClient::CreaterConnect(const std::string& address, u32 port)
    {
        if (nullptr == tcp_connect)
        {
            return false;
        }

        if (address != event_address)
        {
            event_address = address;
        }

        if (port != event_port)
        {
            event_port = port;
        }

        return tcp_connect_basic->CreaterConnect(address, port);
    }

    bool TcpClient::DestroyConnect()
    {
        if (nullptr == tcp_connect_basic)
        {
            return false;
        }

        if (nullptr != tcp_session)
        {
            tcp_session->Close();
            tcp_session.reset();
        }

        if (tcp_connect_basic->DestroyConnect())
        {
            return event_basic->StopDispatch();
        }
        return false;
    }

    void TcpClient::SetConnectCallback(const InterfaceConnect& connect)
    {
        if (nullptr == tcp_connect)
        {
            tcp_connect = connect;
        }
    }

    void TcpClient::SetMessageCallback(const InterfaceMessage& message)
    {
        if (nullptr == tcp_meesage)
        {
            tcp_meesage = message;
        }
    }

    void TcpClient::SetDisconsCallback(const InterfaceDiscons& discons)
    {
        if (nullptr == tcp_discons)
        {
            tcp_discons = discons;
        }
    }

    bool TcpClient::ConnectServer(EventLoop* loop)
    {
        if (nullptr == tcp_connect_basic)
        {
            return false;
        }

        if (loop->EventThread())
        {
            if (tcp_connect_basic->DestroyConnect() && tcp_connect_basic->CreaterConnect(event_address, event_port))
            {
                EVENT_INFO("正在重连失败...");
                return true;
            }
            else
            {
                EVENT_INFO("尝试重连失败...");
            }
            
            return false;
        }
        return loop->RunInQueue(std::move(std::bind(&TcpClient::ConnectServer, this, loop)));
    }

    bool TcpClient::CreateSession(EventLoop* loop, evutil_socket_t fd)
    {
        if (loop->EventThread())
        {
            std::unique_lock<std::mutex> lock(tcp_session_mutex);
            {
                if (nullptr == tcp_session)
                {
                    struct sockaddr_storage ss;
                    {
                        if (GetSockStorage(fd, ss))
                        {
                            tcp_session = std::make_shared<TcpSession>(loop,
                                                                       fd,
                                                                       reinterpret_cast<struct sockaddr*>(&ss),
                                                                       0,
                                                                       std::bind(&TcpClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                                                       std::bind(&TcpClient::OnDiscons, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
                                                                       );

                            return true;
                        }
                    }
                }
            }

            return false;
        }
        return loop->RunInQueue(std::move(std::bind(&TcpClient::CreateSession, this, loop, fd)));
    }

    bool TcpClient::DeleteSession()
    {
        std::unique_lock<std::mutex> lock(tcp_session_mutex);
        {
            if (nullptr != tcp_session)
            {
                tcp_session.reset();
                return true;
            }
        }

        return false;
    }

    bool TcpClient::OnConnect(EventLoop* loop, struct bufferevent* bev, evutil_socket_t fd, i16 events, u32 status, u96 index)
    {
        if (nullptr == loop || INVALID_SOCKET == fd)
        {
            return false;
        }

        if (status)
        {
            if (CreateSession(loop, fd))
            {
                if (tcp_session->InitialSession())
                {
                    if (nullptr != tcp_connect)
                    {
                        return tcp_connect(loop, tcp_session, index);
                    }
                }
            }

            return CloseSocketHandler(fd);
        }

        if (ConnectServer(loop))
        {
            return true;
        }

        return true;
    }

    bool TcpClient::OnMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index)
    {
        if (nullptr != tcp_meesage)
        {
            return tcp_meesage(loop, session, buffer, index);
        }
        return true;
    }

    void TcpClient::OnDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index)
    {
        if (nullptr == loop || nullptr == session)
        {
            return;
        }

        if (nullptr != tcp_discons)
        {
            tcp_discons(loop, session, index);
        }

        if (DeleteSession())
        {
            if (RunInQueue(std::move(std::bind(&TcpClient::ConnectServer, this, loop))))
            {
                return;
            }
        }
    }
}