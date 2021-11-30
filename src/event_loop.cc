#include <event_loop.h>
#include <event_queue.h>
namespace Evpp
{
    EventLoop::EventLoop() :
        event_basic(nullptr),
        event_queue(std::make_shared<EventQueue>(this)),
        event_thread()
    {
        EVENT_TRACE("EventLoop::EventLoop");
    }

    EventLoop::~EventLoop()
    {
        EVENT_TRACE("EventLoop::~EventLoop");
        {
            if (nullptr != event_basic)
            {
                event_base_free(event_basic);
            }
        }
    }

    bool EventLoop::InitEventBasic()
    {
#if defined(EVENT__HAVE_PTHREADS) && defined(H_OS_WINDOWS) || defined(H_OS_LINUX)
        if (0 == evthread_use_pthreads())
#elif defined(H_OS_WINDOWS)
        if (0 == evthread_use_windows_threads())
#endif
        {
            return [&](event_config* conf)
            {
                if ([](bool status) { EVENT_INFO("%s", status ? "event init success" : "event init fail"); return status; }(InitEventBasic(conf)))
                {
                    if (nullptr != conf)
                    {
                        event_config_free(conf);
                    }

                    return true;
                }

                return false;
            }(event_config_new());
        }
        return false;
    }

    bool EventLoop::ExecDispatch(bool pending)
    {
        if (nullptr != event_basic)
        {
            // 空跑一次
            if (ExecDispatch(EVLOOP_NONBLOCK))
            {
                if (ChangeStatus(Status::Exec))
                {
                    return ExecDispatch(pending ? EVLOOP_NO_EXIT_ON_EMPTY : 0);
                }
            }
        }

        return false;
    }

    bool EventLoop::ExecDispatch(i32 flags)
    {
        if (ExistsUnlade() || ExistsStoped() || ExistsExited())
        {
            return false;
        }

        if (ExistsInited())
        {
            if ([&](const i32 status) -> i32 { if (0 == status) EVENT_INFO("事件循环正常轮询 [%d]", 0); if (1 == status) EVENT_INFO("事件循环正常轮询 [%d]", 1); return 0 == status || 1 == status; }(event_base_loop(event_basic, flags)))
            {
                return true;
            }
            else
            {
                EVENT_ERROR("事件循环发生异常错误");
            }
        }
        return false;
    }

    bool EventLoop::StopDispatch(const std::chrono::milliseconds& delay)
    {
        if (nullptr == event_basic || nullptr == event_queue)
        {
            return false;
        }

        if (ExistsUnlade() || ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (EventThread())
        {
            if (ExistsInited() || ExistsRuning())
            {
                if (event_queue->DestroyQueue())
                {
                    if (delay.count())
                    {
                        return 0 == event_base_loopexit(event_basic, std::addressof(const_cast<struct timeval&>(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))));
                    }
                    return true;
                }
            }
            return false;
        }

        return RunInBlock(std::move(std::bind(&EventLoop::StopDispatch, this, delay)));
    }

    bool EventLoop::InitEventBasic(event_config* conf)
    {
        if (nullptr == conf)
        {
            return true;
        }

        if (ExistsRuning() || ExistsStoped() || ExistsExited() || std::thread::id() != event_thread)
        {
            return true;
        }

        if (std::thread::id() == event_thread)
        {
            event_thread = std::this_thread::get_id();
        }

        if (0 == event_config_set_flag(conf, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST) &&
            0 == event_config_set_flag(conf, EVENT_BASE_FLAG_NO_CACHE_TIME) &&
            0 == event_config_set_flag(conf, EVENT_BASE_FLAG_PRECISE_TIMER))
        {
#ifdef H_OS_WINDOWS
            if (0 == event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP))
#endif
            {
                if (nullptr == event_basic)
                {
                    event_basic = event_base_new_with_config(conf);
                }

                // 创建异步队列对象
                if (event_queue->CreaterQueue())
                {
                    return ChangeStatus(Status::Init);
                }
            }
        }

        return false;
    }

    bool EventLoop::RunInAfter(Handler&& function)
    {
        if (nullptr == event_queue)
        {
            return false;
        }

        if (ExistsInited() || ExistsRuning())
        {
            return event_queue->RunInAfter(std::move(function));
        }
        return false;
    }

    bool EventLoop::RunInBlock(Handler&& function)
    {
        if (nullptr == event_queue)
        {
            return false;
        }

        if (ExistsInited() || ExistsRuning())
        {
            return event_queue->RunInBlock(std::move(function));
        }
        return false;
    }

    bool EventLoop::RunInQueue(Handler&& function)
    {
        if (nullptr == event_queue)
        {
            return false;
        }

        if (ExistsInited() || ExistsRuning())
        {
            return event_queue->RunInQueue(std::move(function));
        }
        return false;
    }

    bool EventLoop::AddContext(u96 index, const std::any& context)
    {
        if (event_context.find(index) == event_context.end())
        {
            return event_context.try_emplace(index, context).second;
        }
        return false;
    }

    bool EventLoop::DelContext(u96 index)
    {
        if (event_context.find(index) != event_context.end())
        {
            return 1 == event_context.erase(index);
        }
        return false;
    }

    const std::any& EventLoop::GetContext(u96 index)
    {
        return event_context[index];
    }

    u32 EventLoop::EventThread()
    {
        if (std::thread::id() == event_thread)
        {
            return 0;
        }

        return std::this_thread::get_id() == event_thread;
    }

    u32 EventLoop::EventThreadId()
    {
        return *reinterpret_cast<const u32*>(std::addressof(static_cast<const std::thread::id&>(event_thread)));
    }

    u32 EventLoop::EventCurrentThreadId()
    {
        return *reinterpret_cast<const u32*>(std::addressof(static_cast<const std::thread::id&>(std::this_thread::get_id())));
    }
}