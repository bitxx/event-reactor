#include <event_timer.h>
#include <event_loop.h>
namespace Evpp
{
    EventTimer::EventTimer(EventLoop* basic, const EventTimerHandler& callback, u96 index) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_index(index),
        event_handle(std::make_shared<struct event>()),
        event_callback(callback)
    {
        EVENT_TRACE("EventTimer::EventTimer");
    }

    EventTimer::~EventTimer()
    {
        EVENT_TRACE("EventTimer::~EventTimer");
    }

    bool EventTimer::InitedTimer()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (0 != event_assign(event_handle.get(), event_basic->EventBasic(), INVALID_SOCKET, EV_TIMEOUT | EV_PERSIST, &EventTimer::OnNotify, this))
        {
            return false;
        }

        return ChangeStatus(Status::Init);
    }

    bool EventTimer::AssignTimer(u32 delay, u32 activate)
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        return AssignTimer(std::move(std::chrono::milliseconds(delay)), activate);
    }

    bool EventTimer::AssignTimer(const std::chrono::milliseconds& delay, u32 activate)
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        return AssignTimer(std::addressof(const_cast<struct timeval&>(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))), activate);
    }

    bool EventTimer::AssignTimer(const struct timeval* delay, u32 activate)
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsRuning())
        {
            return true;
        }

        if (ExistsInited() || ExistsStoped())
        {
            // 添加到挂起的事件集
            if (0 != event_add(event_handle.get(), delay))
            {
                return false;
            }

            if (1 == activate)
            {
                // 激活挂起事件立即激活
                event_active(event_handle.get(), 0, 0);
            }

            if (ExistsStoped() && RemoveStatus(Status::Stop))
            {
                EVENT_INFO("定时器重新启动[Stop]");
            }

            if (ExistsExited() && RemoveStatus(Status::Exit))
            {
                EVENT_INFO("定时器重新启动[Exit]");
            }

            return ChangeStatus(Status::Exec);
        }
        return false;
    }

    bool EventTimer::CancelTimer()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (ExistsRuning())
        {
            if (event_initialized(event_handle.get()))
            {
                if (0 == event_del(event_handle.get()))
                {
                    return RemoveStatus(Status::Exec) && ChangeStatus(Status::Stop);
                }
            }
        }

        return true;
    }

    bool EventTimer::KilledTimer()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsRuning())
        {
            // 解除事件设置 若事件本身是一个超时事件则该函数相当于 event_del 函数
            if (event_pending(event_handle.get(), EV_TIMEOUT, nullptr) && 0 == event_remove_timer(event_handle.get()))
            {
                // 再次执行取消定时器 重复检查 event_del 防止句柄泄露
                if (CancelTimer())
                {
                    return ChangeStatus(Status::Exit);
                }
            }
        }

        return false;
    }

    void EventTimer::OnNotify(evutil_socket_t fd, short flags)
    {
        if (flags & EV_TIMEOUT || 0 == flags)
        {
            if (nullptr == event_callback)
            {
                return;
            }
            else
            {
                std::invoke(event_callback, event_basic, shared_from_this(), event_index);
            }
        }
    }

    void EventTimer::OnNotify(evutil_socket_t fd, short flags, void* data)
    {
        if (nullptr != data)
        {
            EventTimer* watcher = static_cast<EventTimer*>(data);
            {
                if (watcher == nullptr || INVALID_SOCKET != fd)
                {
                    return;
                }

                if (flags & EV_TIMEOUT || 0 == flags)
                {
                    watcher->OnNotify(fd, flags);
                }
            }
        }
    }
}