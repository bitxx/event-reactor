#include <network/tcp/client/tcp_connect_basic.h>

#include <event_loop.h>

namespace Evpp
{
    TcpConnectBasic::TcpConnectBasic(EventLoop* basic, const Details::InterfaceConnect& connect, const std::string& address, u32 port) :
        TcpConnectBasic(basic, 0, connect, address, port)
    {
        EVENT_TRACE("TcpConnectBasic::TcpConnectBasic");
    }

    TcpConnectBasic::TcpConnectBasic(EventLoop* basic, u96 index, const Details::InterfaceConnect& connect, const std::string& address, u32 port) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_index(index),
        event_connect(connect),
        event_address(address),
        event_port(port),
        event_buffer(nullptr),
        event_dns(nullptr),
        event_socket(INVALID_SOCKET)
    {
        EVENT_TRACE("TcpConnectBasic::TcpConnectBasic");
    }

    TcpConnectBasic::~TcpConnectBasic()
    {
        EVENT_TRACE("TcpConnectBasic::~TcpConnectBasic");
    }

    bool TcpConnectBasic::CreaterConnect()
    {
        return CreaterConnect(event_address, event_port);
    }

    bool TcpConnectBasic::CreaterConnect(const std::string& address, u32 port)
    {
        if (nullptr == event_basic)
        {
            return false;
        }

        // 检查来源线程
        if (EventThread())
        {
            if (ExistsInited())
            {
                return true;
            }

            if (address.empty() || 0 == port)
            {
                return false;
            }

            if (CreateTcpSocket(AF_INET, SOCK_STREAM, IPPROTO_IP, event_socket))
            {
                if (ChangeStatus(Status::Init))
                {
                    return CreaterConnect(bufferevent_socket_new(event_basic->EventBasic(), event_socket, BEV_OPT_THREADSAFE),
                                          address,
                                          port);
                }
            }
            else
            {
                // lsof -n|awk '{print $2}'|sort|uniq -c|sort -nr|more
                EVENT_ERROR("CreateTcpSocket: %d -> %s", EVUTIL_SOCKET_ERROR(), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            }

            return false;
        }

        return RunInQueue(std::move(std::bind<bool(TcpConnectBasic::*)(const std::string&, u32)>(&TcpConnectBasic::CreaterConnect, this, address, port)));
    }

    bool TcpConnectBasic::CreaterConnect(struct bufferevent* bev, const std::string& address, u32 port)
    {
        if (ExistsRuning())
        {
            return true;
        }

        if (nullptr == bev)
        {
            EVENT_ERROR("bufferevent nullptr");
            return false;
        }

        if (bev != event_buffer)
        {
            event_buffer = bev;
        }

        bufferevent_setcb(bev,
                          0,
                          0, [](struct bufferevent* bev, short events, void* data)
                          {
                              if (nullptr != data)
                              {
                                  TcpConnectBasic* watcher = static_cast<TcpConnectBasic*>(data);
                                  {
                                      if (nullptr == watcher)
                                      {
                                          return;
                                      }
                                      return watcher->OnEventNotify(bev, events);
                                  }
                              }
                          }, this);

        if (nullptr == event_dns)
        {
            event_dns = evdns_base_new(event_basic->EventBasic(), EVDNS_BASE_INITIALIZE_NAMESERVERS | EVDNS_BASE_DISABLE_WHEN_INACTIVE | EVDNS_BASE_NAMESERVERS_NO_DEFAULT);
        }

        if (0 == bufferevent_socket_connect_hostname(bev,
                                                     event_dns,
                                                     AF_UNSPEC,
                                                     address.c_str(),
                                                     port))
        {
            return RemoveStatus(Status::Stop) && RemoveStatus(Status::Exit) && ChangeStatus(Status::Exec);
        }

        sockaddr_storage storage; i32 storage_size = sizeof(storage); std::string addr = address + ":" + std::to_string(port);
        {
            if (0 == evutil_parse_sockaddr_port(addr.c_str(), reinterpret_cast<struct sockaddr *>(std::addressof(storage)), std::addressof(storage_size)))
            {
                if(0 == bufferevent_socket_connect(bev, reinterpret_cast<struct sockaddr *>(std::addressof(storage)), storage_size))
                {
                    return RemoveStatus(Status::Stop) && RemoveStatus(Status::Exit) && ChangeStatus(Status::Exec);
                }
                else
                {
                    EVENT_ERROR("got an error from bufferevent: %d -> %s", EVUTIL_SOCKET_ERROR(), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
                }
            }
        }

        return false;
    }

    bool TcpConnectBasic::DestroyConnect(u32 isclose)
    {
        if (nullptr == event_buffer && INVALID_SOCKET == event_socket)
        {
            return true;
        }

        // 检查来源线程
        if (EventThread())
        {
            if (ExistsStoped() || ExistsExited())
            {
                return true;
            }

            if (nullptr != event_buffer)
            {
                bufferevent_free(event_buffer);
                event_buffer = nullptr;
            }

            if (nullptr != event_dns)
            {
                evdns_base_free(event_dns, 0);
                event_dns = nullptr;
            }

            if (isclose && CloseSocketHandler(event_socket))
            {
                EVENT_INFO("close file handler")
            }

            if (RemoveStatus(Status::Init) && RemoveStatus(Status::Exec) && ChangeStatus(Status::Exit))
            {
                return true;
            }
            return false;
        }
        return RunInQueue(std::move(std::bind(&TcpConnectBasic::DestroyConnect, this, isclose)));
    }

    void TcpConnectBasic::OnEventNotify(struct bufferevent* bev, short events)
    {
        if (nullptr == bev)
        {
            return;
        }

        if (nullptr != event_connect)
        {
            if (RunInQueue(std::move(std::bind(event_connect, event_basic, bev, event_socket, events, static_cast<u32>(events & BEV_EVENT_ERROR ? 0 : events & BEV_EVENT_CONNECTED ? 1 : 0), event_index))))
            {
                return;
            }

            if (event_connect(event_basic, bev, event_socket, events, static_cast<u32>(events & BEV_EVENT_ERROR ? 0 : events & BEV_EVENT_CONNECTED ? 1 : 0), event_index))
            {
                return;
            }
        }

        if (ExistsStoped() || ExistsExited() || !(events & BEV_EVENT_CONNECTED))
        {
            if (CloseSocketHandler(event_socket))
            {
                EVENT_INFO("监听服务已经停止，关闭文件描述符");
            }
        }

        EVENT_INFO("其它错误: [%d]", events);
    }
}