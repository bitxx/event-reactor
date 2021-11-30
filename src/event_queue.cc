#include <event_queue.h>
#include <event_loop.h>
#include <event_watcher.h>
namespace Evpp
{
    EventQueue::EventQueue(EventLoop* basic) :
        event_basic(basic),
        event_watcher_block(std::make_shared<EventWatcher>(basic, std::move(std::bind<void(EventQueue::*)()>(&EventQueue::BlockHandler, this)))),
        event_watcher_noblock(std::make_shared<EventWatcher>(basic, std::move(std::bind<void(EventQueue::*)()>(&EventQueue::NoBlockHandler, this)))),
        event_prepare(0)
    {
        EVENT_TRACE("EventQueue::EventQueue");
    }

    EventQueue::~EventQueue()
    {
        EVENT_TRACE("EventQueue::~EventQueue");
    }

    bool EventQueue::CreaterQueue()
    {
        if (nullptr == event_basic || nullptr == event_watcher_block || nullptr == event_watcher_noblock)
        {
            return false;
        }

        if (ChangeStatus(Status::Init))
        {
            if (event_watcher_block->CreaterWatcher() && event_watcher_noblock->CreaterWatcher())
            {
                return ChangeStatus(Status::Exec);
            }
        }

        return false;
    }

    bool EventQueue::DestroyQueue()
    {
        if (nullptr == event_basic || nullptr == event_watcher_noblock)
        {
            return false;
        }

        if (ChangeStatus(Status::Stop))
        {
            if (event_queue_block.size())
            {
                BlockHandler();
            }

            if (event_queue_noblock.size())
            {
                NoBlockHandler();
            }

            if (event_watcher_block->DestroyWatcher() && event_watcher_noblock->DestroyWatcher())
            {
                return ChangeStatus(Status::Exit);
            }
        }

        return false;
    }

    bool EventQueue::RunInAfter(Handler&& function)
    {
        if (nullptr == event_basic || nullptr == event_watcher_noblock)
        {
            return false;
        }

        if (ExistsRuning())
        {
            if (event_basic->EventThread())
            {
                if (nullptr != function)
                {
                    function();
                }

                return true;
            }

            return NoBlockHandler(std::move(function));
        }
        return false;
    }

    bool EventQueue::RunInBlock(Handler&& function)
    {
        if (ExistsRuning())
        {
            if (event_basic->EventThread())
            {
                assert(0);
            }
            return BlockHandler(std::move(function));
        }
        return false;
    }

    bool EventQueue::RunInQueue(Handler&& function)
    {
        if (nullptr == event_basic || nullptr == event_watcher_noblock)
        {
            return false;
        }

        if (ExistsRuning())
        {
            return NoBlockHandler(std::move(function));
        }
        return false;
    }

    bool EventQueue::BlockHandler(Handler&& function)
    {
        if (nullptr == event_basic || nullptr == event_watcher_block)
        {
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(event_queue_mutex_block);
            {
                event_queue_block.emplace_back(std::move(function));
            }
        }

        if (event_watcher_block->SendNotify())
        {
            if (0 == event_prepare.try_acquire_for(std::chrono::milliseconds(3 * 10000)))
            {
                EVENT_ERROR("事件队列同步等待超时");
            }
            return true;
        }
        return false;
    }

    void EventQueue::BlockHandler()
    {
        std::vector<Handler> functors;
        {
            std::lock_guard<std::mutex> lock(event_queue_mutex_block);
            {
                functors.swap(event_queue_block);
            }
        }

        if (functors.size())
        {
            for (const auto& function : functors)
            {
                if (nullptr != function)
                {
                    function();
                }
                event_prepare.release();
            }
        }
    }

    bool EventQueue::NoBlockHandler(Handler&& function)
    {
        if (nullptr == event_basic || nullptr == event_watcher_noblock)
        {
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(event_queue_mutex_noblock);
            {
                event_queue_noblock.emplace_back(std::move(function));
            }
        }

        return event_watcher_noblock->SendNotify();
    }

    void EventQueue::NoBlockHandler()
    {
        std::vector<Handler> functors;
        {
            std::lock_guard<std::mutex> lock(event_queue_mutex_noblock);
            {
                functors.swap(event_queue_noblock);
            }
        }

        if (functors.size())
        {
            for (const auto& function : functors)
            {
                if (nullptr != function)
                {
                    function();
                }
            }
        }
    }
}