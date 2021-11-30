#ifndef __EVENT_STATUS_H__
#define __EVENT_STATUS_H__

#include <event_core.h>

namespace Evpp
{
    enum Status
    {
        None = 1 << 0,
        Init = 1 << 1,
        Exec = 1 << 2,
        Stop = 1 << 3,
        Exit = 1 << 4,
        Fail = 1 << 5,
    };

    class EventStatus
    {
    public:
        explicit EventStatus();
        virtual ~EventStatus();
    public:
        virtual bool ChangeStatus(Status original, const Status other);
        virtual bool ChangeStatus(const Status other);
        virtual bool RemoveStatus(const Status other);
        virtual bool RevertStatus(const Status other);
        virtual bool ExistsStarts(const Status other);
    public:
        virtual bool ExistsUnlade();
        virtual bool ExistsNoneed();
        virtual bool ExistsInited();
        virtual bool ExistsRuning();
        virtual bool ExistsStoped();
        virtual bool ExistsExited();
    protected:
        std::atomic<u32>                                                            event_status;
    };
}
#endif // __EVENT_STATUS_H__
