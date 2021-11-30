#ifndef __EVENT_LOOP_IMPL_H__
#define __EVENT_LOOP_IMPL_H__
#include <event_core.h>
namespace Evpp
{
    class EventLoop;
    class EventLoopImpl
    {
    public:
        explicit EventLoopImpl() = default;
        explicit EventLoopImpl(EventLoop* basic);
        virtual ~EventLoopImpl();
    public:
        virtual bool RunInAfter(Handler&& function);
        virtual bool RunInBlock(Handler&& function);
        virtual bool RunInQueue(Handler&& function);
    public:
        virtual u32 EventThread();
        virtual u32 EventThreadId();
        virtual u32 EventCurrentThreadId();
    private:
        EventLoop*                                                                              event_basic;
    };
}
#endif // __EVENT_LOOP_IMPL_H__