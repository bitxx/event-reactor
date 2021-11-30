#include <event_status.h>

namespace Evpp
{
    EventStatus::EventStatus() : event_status(Status::None)
    {

    }

    EventStatus::~EventStatus()
    {

    }

    bool EventStatus::ChangeStatus(Status original, const Status other)
    {
        return event_status.compare_exchange_strong(reinterpret_cast<u32&>(original), reinterpret_cast<const u32&>(other), std::memory_order_release);
    }

    bool EventStatus::ChangeStatus(const Status other)
    {
        if (event_status & other)
        {
            return true;
        }

        return !(other & event_status.exchange(event_status | other, std::memory_order_release));
    }

    bool EventStatus::RemoveStatus(const Status other)
    {
        if (event_status & other)
        {
            return other & event_status.exchange(event_status & ~other, std::memory_order_release);
        }

        return true;
    }

    bool EventStatus::RevertStatus(const Status other)
    {
        return !(other & event_status.exchange(other | Status::None, std::memory_order_release));
    }

    bool EventStatus::ExistsStarts(const Status other)
    {
        return other & event_status;
    }

    bool EventStatus::ExistsUnlade()
    {
        return 0 == event_status;
    }

    bool EventStatus::ExistsNoneed()
    {
        return Status::None & event_status;
    }

    bool EventStatus::ExistsInited()
    {
        return Status::Init & event_status;
    }

    bool EventStatus::ExistsRuning()
    {
        return Status::Exec & event_status;
    }

    bool EventStatus::ExistsStoped()
    {
        return Status::Stop & event_status;
    }

    bool EventStatus::ExistsExited()
    {
        return Status::Exit & event_status;
    }
}