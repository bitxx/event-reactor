#include <event_loop_thread_pool.h>
#include <event_loop_thread.h>
#include <future>
namespace Evpp
{
    EventLoopThreadPool::EventLoopThreadPool(EventLoop* basic) : EventLoopThreadPool(basic, 0)
    {
        EVENT_TRACE("EventLoopThreadPool::EventLoopThreadPool");
    }

    EventLoopThreadPool::EventLoopThreadPool(EventLoop* basic, u96 size) : 
        EventLoopImpl(basic),
        event_basic(basic),
        event_size(size),
        event_index(0)
    {
        EVENT_TRACE("EventLoopThreadPool::EventLoopThreadPool");
    }

    EventLoopThreadPool::~EventLoopThreadPool()
    {
        EVENT_TRACE("EventLoopThreadPool::~EventLoopThreadPool");
    }

    bool EventLoopThreadPool::CreaterEventThreadPool(const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        return CreaterEventThreadPool(event_size, delay, star, stop);
    }

    bool EventLoopThreadPool::CreaterEventThreadPool(u96 thread_size, const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        if (nullptr == event_basic)
        {
            return false;
        }

        if (EventThread())
        {
            for (u96 i = 0; i < thread_size; ++i)
            {
                if (CreaterEventThread(PlexingIndex()))
                {
                    continue;
                }
                else
                {
                    EVENT_WARN("该线程已经存在");
                }
            }

            return InitialEventThreadPool(delay, star, stop);
        }
        return RunInBlock(std::move(std::bind<bool(EventLoopThreadPool::*)(const u96, const std::chrono::milliseconds&, const EventThreadStarHandler&, const EventThreadStopHandler&)>(&EventLoopThreadPool::CreaterEventThreadPool, this, thread_size, delay, star, stop)));
    }

    bool EventLoopThreadPool::DestroyEventThreadPool(const std::chrono::milliseconds& delay)
    {
        std::unique_lock<std::recursive_mutex> lock(event_mutex);
        {
            for (const auto& [index, thread] : event_loops)
            {
                thread->DestroyThread(delay);
            }
        }

        return true;
    }

    EventLoop* EventLoopThreadPool::GetNextLoop()
    {
        return GetNextLoop(event_index.fetch_add(1));
    }

    EventLoop* EventLoopThreadPool::GetNextLoop(u96 index)
    {
        return DoGetNextLoop(index);
    }

    EventLoop* EventLoopThreadPool::DoGetNextLoop(u96 index)
    {
        try
        {
            std::unique_lock<std::recursive_mutex> lock(event_mutex);
            {
                // 如果容器数量为空 并且 如果递归次数 > 容器总数 && 默认事件循环对象 != 无效指针 此时 返回一个默认的事件循环对象
                if (event_loops.empty() || event_loops.empty() && index > event_loops.size() && nullptr != event_basic)
                {
                    return event_basic;
                }
                return event_loops[index % event_loops.size()]->GetEventLoop();
            }
        }
        catch (const std::system_error& ex)
        {
            EVENT_ERROR("DoGetNextLoop: %s", ex.what());
        }

        return nullptr;
    }

    bool EventLoopThreadPool::InitialEventThreadPool(const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        std::unique_lock<std::recursive_mutex> lock(event_mutex);
        {
            for (const auto& [index, thread] : event_loops)
            {
                if (nullptr == thread)
                {
                    continue;
                }

                if (thread->CreaterThread(delay, star, stop))
                {
                    continue;
                }
                else
                {
                    EVENT_ERROR("创建线程超时");
                }
            }
        }

        return true;
    }

    bool EventLoopThreadPool::CreaterEventThread(u96 index)
    {
        std::unique_lock<std::recursive_mutex> lock(event_mutex);
        {
            if (event_loops.find(index) == event_loops.end())
            {
                return event_loops.emplace(index, std::make_shared<EventLoopThread>(event_basic, std::bind(&EventLoopThreadPool::DestroyEventThread, this, std::placeholders::_1), index)).second;
            }
        }

        return false;
    }

    bool EventLoopThreadPool::DestroyEventThread(u96 index)
    {
        std::unique_lock<std::recursive_mutex> lock(event_mutex);
        {
            if (event_loops.find(index) != event_loops.end())
            {
                return 1 == event_loops.erase(index);
            }
        }
        return false;
    }
}