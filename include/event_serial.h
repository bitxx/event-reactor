#ifndef __EVENT_SERIAL_H__
#define __EVENT_SERIAL_H__

#include <event_core.h>

namespace Evpp
{
    typedef enum
    {
        fivebits = 5,
        sixbits = 6,
        sevenbits = 7,
        eightbits = 8
    } bytesize_t;

    typedef enum
    {
        parity_none = 0,
        parity_odd = 1,
        parity_even = 2,
        parity_mark = 3,
        parity_space = 4
    } parity_t;

    typedef enum
    {
        stopbits_one = 1,
        stopbits_two = 2,
        stopbits_five
    } stopbits_t;

    typedef enum
    {
        flowcontrol_none = 0,
        flowcontrol_software,
        flowcontrol_hardware
    } flowcontrol_t;

    class EventBuffer;

    class EventSerial
    {
    public:
        explicit EventSerial(const std::shared_ptr<EventBuffer>& buffer);
        virtual ~EventSerial();
    public:
        bool CreaterConnect(const std::string& name);
        bool ConfigComState(u32 baudrate = CBR_9600, bytesize_t bytesize = eightbits, stopbits_t stopbits = stopbits_two, parity_t parity = parity_none, flowcontrol_t flowcontrol = flowcontrol_none);
        bool DestroyConnect();
    public:
        bool Send(const std::string& buffer);
        bool Send(const char* buffer, u64 size);
        bool Read();
    public:
    private:
        HANDLE                                                    event_handler;
        std::shared_ptr<EventBuffer>                              event_buffer;
    };
}
#endif