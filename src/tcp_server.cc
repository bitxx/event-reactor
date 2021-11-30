#include <network/tcp/server/tcp_server.h>
#include <network/tcp/server/tcp_listen.h>
#include <network/tcp/server/tcp_listen_pool.h>
#include <network/tcp/tcp_session.h>
#include <event_loop.h>
#include <event_loop_thread_pool.h>
namespace Evpp
{
    TcpServer::TcpServer(EventLoop* basic) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_thread_pool(std::make_shared<EventLoopThreadPool>(basic)),
        tcp_listen(std::make_unique<TcpListenPool>(basic,
                                                   std::bind(&TcpServer::DefaultAccepts, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)))
    {
        event_set_log_callback([] (int severity, const char* msg)
                               {
                                   if (nullptr == msg)
                                   {
                                       return;
                                   }

                                   switch (severity)
                                   {
                                       case EVENT_LOG_DEBUG:    EVENT_TRACE(msg);       break;
                                       case EVENT_LOG_MSG:      EVENT_INFO(msg);        break;
                                       case EVENT_LOG_WARN:     EVENT_WARN(msg);        break;
                                       case EVENT_LOG_ERR:      EVENT_ERROR(msg);       break;
                                       default:                                         break; /* never reached */
                                   }
                               });

        event_set_fatal_callback([] (int error)
                                 {
                                     EVENT_FATAL("%d", error);
                                 });
    }

    TcpServer::~TcpServer()
    {
        EVENT_TRACE("TcpServer::~TcpServer");
    }

    bool TcpServer::CreaterListen(const std::string& address)
    {
        if (nullptr == tcp_listen)
        {
            return false;
        }

        return tcp_listen->CreaterListen(address);
    }

    bool TcpServer::CreaterServer(u96 thread_size, const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        if (nullptr != tcp_listen)
        {
            if (EventThread())
            {
                if (ChangeStatus(Status::Init))
                {
                    std::unique_lock<std::mutex> lock(tcp_destroy_mutex);
                    {
                        if (event_thread_pool->CreaterEventThreadPool(thread_size, delay, star, stop) && tcp_listen->InitialListen())
                        {
                            if (tcp_listen->CreaterListen())
                            {
                                return ChangeStatus(Status::Exec);
                            }
                            else
                            {
                                EVENT_ERROR("创建监听服务失败");
                            }
                        }
                    }
                }
                return false;
            }
            return RunInQueue(std::move(std::bind(&TcpServer::CreaterServer, this, thread_size, delay, star, stop)));
        }
        return false;
    }

    bool TcpServer::DestroyServer(const std::chrono::milliseconds& delay)
    {
        if (nullptr == event_basic || nullptr == tcp_listen || nullptr == event_thread_pool)
        {
            return false;
        }

        if (EventThread())
        {
            if (ExistsUnlade() || ExistsStoped() || ExistsExited())
            {
                return true;
            }

            // 设置为停止状态
            if (ChangeStatus(Status::Stop))
            {
                std::unique_lock<std::mutex> lock(tcp_destroy_mutex);
                {
                    // 首先销毁监听
                    if (0 == tcp_listen->DestroyListen())
                    {
                        EVENT_ERROR("销毁监听服务出现错误");
                    }

                    // 关闭服务器上所有现存的会话
                    if (0 == DeleteSession())
                    {
                        EVENT_ERROR("关闭会话过程出现错误");
                    }

                    // 最后销毁线程池
                    if (0 == event_thread_pool->DestroyEventThreadPool(delay))
                    {
                        EVENT_ERROR("销毁线程过程出现错误");
                    }

                    // 主线程收尸
                    if (event_basic->StopDispatch())
                    {
                        return ChangeStatus(Status::Exit);
                    }
                }
            }
            return false;
        }
        return RunInQueue(std::move(std::bind(&TcpServer::DestroyServer, this, delay)));
    }

    bool TcpServer::Send(u96 index, const std::string& buffer)
    {
        return [&] (TcpSession* session) { if (nullptr == session) { return false; } return session->Send(buffer); } (GetSession(index));
    }

    bool TcpServer::Close(u96 index, const Handler& callback)
    {
        return [&] (TcpSession* session) { if (nullptr == session) { return false; }  return session->Close(callback); } (GetSession(index));
    }

    void TcpServer::SetAcceptsCallback(const InterfaceAccepts& accepts)
    {
        if (nullptr == tcp_accepts)
        {
            tcp_accepts = accepts;
        }
    }

    void TcpServer::SetMessageCallback(const InterfaceMessage& message)
    {
        if (nullptr == tcp_meesage)
        {
            tcp_meesage = message;
        }
    }

    void TcpServer::SetDisconsCallback(const InterfaceDiscons& discons)
    {
        if (nullptr == tcp_discons)
        {
            tcp_discons = discons;
        }
    }

    bool TcpServer::CreateSession(EventLoop* loop, evutil_socket_t fd, struct sockaddr* addr, u96 index)
    {
        if (EventThread())
        {
            std::unique_lock<std::mutex> lock(tcp_current_mutex);
            {
                if (tcp_session_pool.find(index) == tcp_session_pool.end())
                {
                    if (0 == tcp_session_pool.try_emplace
                    (
                        index,
                        std::make_shared<TcpSession>(loop,
                                                     fd,
                                                     reinterpret_cast<struct sockaddr*>(addr),
                                                     index,
                                                     std::move(std::bind(&TcpServer::DefaultMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
                                                     std::move(std::bind(&TcpServer::DefaultDiscons, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))
                    ).second)
                    {
                        return CloseSocketHandler(fd);
                    }

                    return [&] (const std::shared_ptr<TcpSession>& session)
                    {
                        if (session->InitialSession())
                        {
                            if (tcp_accepts(loop, session, index))
                            {
                                return true;
                            }

                            return RunInQueue(std::bind(&TcpSession::Close, session.get(), Handler()));
                        }
                        return false;
                    }(tcp_session_pool[index]);
                }
            }
            return false;
        }

        return RunInQueue(std::bind(&TcpServer::CreateSession, this, loop, fd, addr, index));
    }

    bool TcpServer::DeleteSession(u96 index)
    {
        std::unique_lock<std::mutex> lock(tcp_current_mutex);
        {
            if (tcp_session_pool.find(index) != tcp_session_pool.end())
            {
                if (1 == tcp_session_pool.erase(index))
                {
                    ResumeIndex(index);
                }

                return true;
            }
        }
        return false;
    }

    bool TcpServer::DeleteSession()
    {
        std::unique_lock<std::mutex> lock(tcp_current_mutex);
        {
            for (const auto& [index, session] : tcp_session_pool)
            {
                if (session->Close())
                {
                    continue;
                }
                return false;
            }
        }
        return true;
    }

    TcpSession* TcpServer::GetSession(u96 index)
    {
        std::unique_lock<std::mutex> lock(tcp_current_mutex);
        {
            if (tcp_session_pool.find(index) != tcp_session_pool.end())
            {
                return tcp_session_pool[index].get();
            }
        }
        return nullptr;
    }

    bool TcpServer::DefaultAccepts(EventLoop* loop, struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr)
    {
        if (nullptr == loop || nullptr == listener || INVALID_SOCKET == fd || nullptr == addr)
        {
            return false;
        }

        // 如果服务器停止 不在接受新用户直接关闭用户描述符;
        if (ExistsStoped() || ExistsExited())
        {
            return false;
        }

        if (CreateSession(event_thread_pool->GetNextLoop(), fd, addr, PlexingIndex()))
        {
            return true;
        }

        return false;
    }

    bool TcpServer::DefaultMessage(EventLoop* loop, const std::shared_ptr<TcpSession>& session, const std::shared_ptr<EventBuffer>& buffer, u96 index)
    {
        if (nullptr == loop || nullptr == session || nullptr == buffer)
        {
            return false;
        }

        if (nullptr != tcp_meesage)
        {
            return tcp_meesage(loop, session, buffer, index);
        }
        return true;
    }

    void TcpServer::DefaultDiscons(EventLoop* loop, const std::shared_ptr<TcpSession>& session, u96 index)
    {
        if (nullptr == loop || nullptr == session)
        {
            return;
        }

        if (nullptr != tcp_discons)
        {
            tcp_discons(loop, session, index);
        }

        if (RunInQueue(std::bind<bool(TcpServer::*)(u96)>(&TcpServer::DeleteSession, this, index)))
        {
            return;
        }
    }
}