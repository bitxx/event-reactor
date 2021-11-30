#include <event_poll.h>
#include <event_loop.h>

namespace Evpp
{
    EventPoll::EventPoll(EventLoop* basic) : EventPoll(basic, INVALID_SOCKET)
    {

    }

    EventPoll::EventPoll(EventLoop* basic, evutil_socket_t fd) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_handle(std::make_shared<struct event>()),
        event_descriptor(fd),
        event_flags(0)
    {
        if (ChangeStatus(Status::None))
        {
            EVENT_INFO("eventpoll constructed function");
        }
    }

    EventPoll::~EventPoll()
    {

    }

    bool EventPoll::Create(short flags)
    {
        if (INVALID_SOCKET == event_descriptor)
        {
            return false;
        }

        return Create(flags, event_descriptor);
    }

    bool EventPoll::Create(short flags, evutil_socket_t fd)
    {
        if (nullptr == event_handle || nullptr == event_basic || INVALID_SOCKET == fd)
        {
            return false;
        }

        if (INVALID_SOCKET == event_descriptor)
        {
            event_descriptor = fd;
        }

        if (0 != event_assign(event_handle.get(), event_basic->EventBasic(), fd, flags | EV_PERSIST, &EventPoll::OnNotify, this))
        {
            return false;
        }

        if (0 != event_base_set(event_basic->EventBasic(), event_handle.get()))
        {
            return false;
        }

        return ChangeStatus(Status::Init);
    }

    bool EventPoll::Attach()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (ExistsInited() || ExistsRuning())
        {
            if (EVPP_SUCCESS(event_add(event_handle.get(), nullptr/*std::addressof(const_cast<struct timeval&>(static_cast<const struct timeval&>(struct timeval(1, 0)))*/)))
            {
                return ChangeStatus(Status::Exec);
            }
        }

        return false;
    }

    bool EventPoll::Detach()
    {
        if (nullptr == event_handle || nullptr == event_basic)
        {
            return false;
        }

        if (ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (ExistsInited() || ExistsRuning() && ChangeStatus(Status::Stop))
        {
            if (event_initialized(event_handle.get()))
            {
                if (0 == event_del(event_handle.get()))
                {
                    return ChangeStatus(Status::Exit);
                }
            }
        }

        return false;
    }

    void EventPoll::OnNotify(evutil_socket_t fd, short flags, void* data)
    {
        if (nullptr != data)
        {
            EventPoll* watcher = static_cast<EventPoll*>(data);
            {
                if (nullptr == watcher || fd != watcher->event_descriptor)
                {
                    return;
                }

                if (flags & EV_READ | EV_WRITE)
                {
                    watcher->OnNotify(fd, flags);
                }
                else
                {
                    EVENT_ERROR("flags: %d", flags);
                }
            }
        }
    }

    EventGreater::EventGreater(EventLoop* basic, const Handler& notify) :
        EventGreater(basic, INVALID_SOCKET, notify)
    {

    }

    EventGreater::EventGreater(EventLoop* basic, evutil_socket_t fd, const Handler& notify):
        EventPoll(basic, fd),
        event_basic(basic),
        event_notify(notify)
    {

    }

    EventGreater::~EventGreater()
    {

    }
    
    void EventGreater::OnNotify(evutil_socket_t fd, short events)
    {
        if (nullptr != event_notify)
        {
            return event_notify(event_basic, shared_from_this(), fd, events);
        }
    }
}