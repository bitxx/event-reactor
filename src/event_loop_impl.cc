#include <event_loop_impl.h>
#include <event_loop.h>
namespace Evpp
{
    EventLoopImpl::EventLoopImpl(EventLoop* basic) : event_basic(basic)
    {

    }

    EventLoopImpl::~EventLoopImpl()
    {

    }

    bool EventLoopImpl::RunInAfter(Handler&& function)
    {
        if (nullptr == event_basic)
        {
            return false;
        }
        return event_basic->RunInAfter(std::move(function));
    }

    bool EventLoopImpl::RunInBlock(Handler&& function)
    {
        if (nullptr == event_basic)
        {
            return false;
        }
        return event_basic->RunInBlock(std::move(function));
    }

    bool EventLoopImpl::RunInQueue(Handler&& function)
    {
        if (nullptr == event_basic)
        {
            return false;
        }
        return event_basic->RunInQueue(std::move(function));
    }

    u32 EventLoopImpl::EventThread()
    {
        if (nullptr == event_basic)
        {
            return 0;
        }

        return event_basic->EventThread();
    }

    u32 EventLoopImpl::EventThreadId()
    {
        if (nullptr == event_basic)
        {
            return 0;
        }

        return event_basic->EventThreadId();
    }

    u32 EventLoopImpl::EventCurrentThreadId()
    {
        if (nullptr == event_basic)
        {
            return 0;
        }

        return event_basic->EventCurrentThreadId();
    }
}