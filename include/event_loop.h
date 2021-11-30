#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__
#include <event_core.h>
namespace Evpp
{
    class EventQueue;
    class EventLoop : public EventLoopImpl, public EventStatus
    {
    public:
        explicit EventLoop();
        virtual ~EventLoop();
    public:
        friend class EventLoopImpl;
    public:
        bool InitEventBasic();
    public:
        bool ExecDispatch(bool pending = false);
        bool ExecDispatch(i32 flags);
        bool StopDispatch(const std::chrono::milliseconds& delay = std::chrono::milliseconds(1));
    private:
        bool InitEventBasic(event_config* conf);
    public:
        virtual bool RunInAfter(Handler&& function) override;
        virtual bool RunInBlock(Handler&& function) override;
        virtual bool RunInQueue(Handler&& function) override;
    public:
        bool AddContext(u96 index, const std::any& context);
        bool DelContext(u96 index);
        const std::any& GetContext(u96 index);
    public:
        virtual u32 EventThread() override;
        virtual u32 EventThreadId() override;
        virtual u32 EventCurrentThreadId() override;
    public:
        struct event_base* EventBasic() const { return event_basic; }
    private:
        struct event_base*                                                                      event_basic;
        std::shared_ptr<EventQueue>                                                             event_queue;
        std::thread::id                                                                         event_thread;
        std::unordered_map<u96, std::any>                                                       event_context;
    };
}
#endif // __EVENT_LOOP_H__