#include <network/tcp/client/tcp_client_ex.h>
#include <network/tcp/client/tcp_connect.h>
#include <network/tcp/tcp_session.h>

#include <event_loop.h>
#include <event_loop_thread_pool.h>

namespace Evpp
{
    TcpClientEx::TcpClientEx(EventLoop* basic) :
        EventLoopImpl(basic),
        tcp_connect_pool(std::make_shared<TcpConnect>(basic,
                                                      std::bind(&TcpClientEx::OnConnect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)))
    {

    }

    TcpClientEx::~TcpClientEx()
    {

    }

    bool TcpClientEx::InitialConnect(u96 thread_size)
    {
        if (nullptr == tcp_connect_pool)
        {
            return false;
        }

        return tcp_connect_pool->InitialConnect(thread_size);
    }

    bool TcpClientEx::CreaterConnect()
    {
        if (nullptr == tcp_connect_pool)
        {
            return false;
        }

        if (tcp_connect_pool->CreaterConnect())
        {
            return RemoveStatus(Status::Stop) && RemoveStatus(Status::Exit) && ChangeStatus(Status::Exec);
        }

        return false;
    }

    bool TcpClientEx::DestroyConnect()
    {
        if (nullptr == event_basic || nullptr == tcp_connect_pool)
        {
            return false;
        }

        if (ChangeStatus(Status::Stop))
        {
            if (tcp_connect_pool->DestroyConnect())
            {
                if (event_basic->StopDispatch())
                {
                    return RemoveStatus(Status::Init) && RemoveStatus(Status::Exec) && ChangeStatus(Status::Exit);
                }
            }
        }

        return false;
    }

    bool TcpClientEx::ConnectServers(u96 index, u32 isclose)
    {
        if (nullptr == tcp_connect_pool)
        {
            return false;
        }

        if (ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (ExistsInited() || ExistsRuning())
        {
            if (tcp_connect_pool->DestroyConnect(index, isclose) && tcp_connect_pool->CreaterConnect(index))
            {
                EVENT_INFO("[%02zu] -> 正在尝试重连...", index);
                return true;
            }
            assert(0);
            return false;
        }
        return true;
    }

    bool TcpClientEx::AddConnect(const std::string& address, u32 port)
    {
        return tcp_connect_pool->AddConnect(address, port);
    }

    bool TcpClientEx::Send(u96 index, const std::string& buffer)
    {
        return [&](TcpSession* session) { if (nullptr == session) { return false; } return session->Send(buffer); } (GetSession(index));
    }

    bool TcpClientEx::Close(u96 index, const Handler& callback)
    {
        return [&](TcpSession* session) { if (nullptr == session) { return false; }  if (session->Close(callback)) { return DeleteSession(index); } return false; } (GetSession(index));
    }

    void TcpClientEx::SetConnectCallback(const InterfaceAccepts& connect)
    {
        if (nullptr == tcp_connect)
        {
            tcp_connect = connect;
        }
    }

    void TcpClientEx::SetMessageCallback(const InterfaceMessage& message)
    {
        if (nullptr == tcp_meesage)
        {
            tcp_meesage = message;
        }
    }

    void TcpClientEx::SetDisconsCallback(const InterfaceDiscons& discons)
    {
        if (nullptr == tcp_discons)
        {
            tcp_discons = discons;
        }
    }

    bool TcpClientEx::CreateSession(EventLoop* loop, u96 index, evutil_socket_t fd)
    {
        if (nullptr == loop || INVALID_SOCKET == fd)
        {
            return false;
        }

        if (loop->EventThread())
        {
            std::unique_lock<std::recursive_mutex> lock(tcp_session_mutex);
            {
                // 主线程插入数据
                if (tcp_session_pool.find(index) == tcp_session_pool.end())
                {
                    struct sockaddr_storage ss;
                    {
                        if (GetSockStorage(fd, ss))
                        {
                            if (0 == tcp_session_pool.try_emplace(index, std::make_shared<TcpSession>(loop,
                                                                                                      fd,
                                                                                                      reinterpret_cast<struct sockaddr*>(&ss),
                                                                                                      index,
                                                                                                      std::bind(&TcpClientEx::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                                                                                      std::bind(&TcpClientEx::OnDiscons, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
                                                                                                      )).second)
                            {
                                return CloseSocketHandler(fd);
                            }

                            return SwitchSession(loop, tcp_session_pool[index], index, fd);
                        }
                    }
                }
            }

            return Close(index);
        }
        return loop->RunInQueue(std::bind(&TcpClientEx::CreateSession, this, loop, index, fd));
    }

    bool TcpClientEx::SwitchSession(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index, evutil_socket_t fd)
    {
        if (nullptr == loop || nullptr == session)
        {
            return false;
        }

        if (loop->EventThread())
        {
            if (session->InitialSession())
            {
                if (nullptr != tcp_connect)
                {
                    if (tcp_connect(loop, session, index))
                    {
                        return true;
                    }
                    return CloseSocketHandler(fd);
                }
                return true;
            }
            return false;
        }
        return loop->RunInQueue(std::bind(&TcpClientEx::SwitchSession, this, loop, session, index, fd));
    }

    bool TcpClientEx::DeleteSession(u96 index)
    {
        std::unique_lock<std::recursive_mutex> lock(tcp_session_mutex);
        {
            if (tcp_session_pool.find(index) != tcp_session_pool.end())
            {
                if (1 == tcp_session_pool.erase(index))
                {
                    return true;
                }
            }
        }

        return false;
    }

    TcpSession* TcpClientEx::GetSession(u96 index)
    {
        std::unique_lock<std::recursive_mutex> lock(tcp_session_mutex);
        {
            if (tcp_session_pool.find(index) != tcp_session_pool.end())
            {
                return tcp_session_pool[index].get();
            }
        }

        return nullptr;
    }

    bool TcpClientEx::OnConnect(EventLoop* loop, struct bufferevent* bev, evutil_socket_t fd, i16 events, u32 status, u96 index)
    {
        if (ExistsStoped() || ExistsExited())
        {
            return false;
        }

        if (status && INVALID_SOCKET != fd)
        {
            // 转移到主线程
            if (CreateSession(loop, index, fd))
            {
                return true;
            }
        }

        if (RunInQueue(std::bind(&TcpClientEx::ConnectServers, this, index, 1)))
        {
            return true;
        }

        return ConnectServers(index, 1);
    }

    bool TcpClientEx::OnMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index)
    {
        if (nullptr != tcp_meesage)
        {
            return tcp_meesage(loop, session, buffer, index);
        }
        return true;
    }

    void TcpClientEx::OnDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index)
    {
        if (nullptr == loop || nullptr == session)
        {
            return;
        }

        if (nullptr != tcp_discons)
        {
            tcp_discons(loop, session, index);
        }

        if (DeleteSession(index))
        {
            if (RunInQueue(std::bind(&TcpClientEx::ConnectServers, this, index, 0)))
            {
                return;
            }
        }
    }
}