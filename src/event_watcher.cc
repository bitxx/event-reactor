#include <event_watcher.h>
#include <event_loop.h>
namespace Evpp
{
    EventWatcher::EventWatcher(EventLoop* basic, Handler&& watcher_callback) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_watcher_callback(std::move(watcher_callback)),
        event_handle(std::make_shared<struct event>()),
        event_descriptor()
    {
        EVENT_TRACE("EventWatcher::EventWatcher");
    }

    EventWatcher::~EventWatcher()
    {
        EVENT_TRACE("EventWatcher::~EventWatcher");
    }

    bool EventWatcher::CreaterWatcher()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ChangeStatus(Status::Init))
        {
            if (0 != evutil_socketpair(AF_UNIX, SOCK_STREAM, IPPROTO_IP, event_descriptor))
            {
                return false;
            }

            // 异步模式
            if (0 != evutil_make_socket_nonblocking(event_descriptor[0]) || 0 != evutil_make_socket_nonblocking(event_descriptor[1]))
            {
                return false;
            }

            // 地址复用
            if (0 != evutil_make_listen_socket_reuseable(event_descriptor[0]) || 0 != evutil_make_listen_socket_reuseable(event_descriptor[1]))
            {
                return false;
            }
            // 关闭 Delay 算法
            if (0 == SetupSockDelay(event_descriptor[0]) || 0 == SetupSockDelay(event_descriptor[1]))
            {
                return false;
            }

            if (0 != event_assign(event_handle.get(), event_basic->EventBasic(), event_descriptor[1], EV_READ | EV_PERSIST, &EventWatcher::OnNotify, this))
            {
                return false;
            }

            if (0 != event_base_set(event_basic->EventBasic(), event_handle.get()))
            {
                return false;
            }

            // 添加到挂起的事件集
            if (0 == event_add(event_handle.get(), nullptr))
            {
                return ChangeStatus(Status::Exec);
            }
        }
        return false;
    }

    bool EventWatcher::DestroyWatcher()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (INVALID_SOCKET != event_descriptor[0] || INVALID_SOCKET != event_descriptor[1])
        {
            if (ChangeStatus(Status::Stop))
            {
                if (0 == event_del(event_handle.get()))
                {
                    if (CloseSocketHandler(event_descriptor[0]) && CloseSocketHandler(event_descriptor[1]))
                    {
                        memset(event_descriptor, INVALID_SOCKET, sizeof(evutil_socket_t) * 2);
                    }
                }
                return ChangeStatus(Status::Exit);
            }
        }

        return false;
    }

    bool EventWatcher::SendNotify()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsStoped() || ExistsExited())
        {
            return false;
        }

        if (INVALID_SOCKET == event_descriptor[0] || INVALID_SOCKET == event_descriptor[1])
        {
            return false;
        }

        if (ExistsRuning())
        {
            char buf[1] = { CHAR_MAX };
            {
                if (send(event_descriptor[0], buf, std::size(buf), 0) < 0)
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    void EventWatcher::OnNotify(evutil_socket_t fd, short flags)
    {
        if (flags & EV_READ)
        {
            if (ExistsRuning())
            {
                char buf[1];
                {
                    if (std::size(buf) == static_cast<u96>(recv(event_descriptor[1], buf, std::size(buf), 0)))
                    {
                        if (CHAR_MAX == buf[0])
                        {
                            if (nullptr != event_watcher_callback)
                            {
                                std::invoke(event_watcher_callback);
                            }
                            return;
                        }
                    }
                    else
                    {
                        EVENT_ERROR("failed to pass messages across threads: %d", EventCurrentThreadId());
                    }
                }
            }
        }
    }

    void EventWatcher::OnNotify(evutil_socket_t fd, short flags, void* data)
    {
        if (nullptr != data)
        {
            EventWatcher* watcher = static_cast<EventWatcher*>(data);
            {
                if (nullptr == watcher || fd != watcher->event_descriptor[1])
                {
                    return;
                }

                if (flags & EV_READ)
                {
                    watcher->OnNotify(fd, flags);
                }
            }
        }
    }
}