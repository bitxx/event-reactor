#include <event_serial.h>
#include <event_convert.h>
#include <event_utils.h>
#include <event_buffer.h>

namespace Evpp
{
    EventSerial::EventSerial(const std::shared_ptr<EventBuffer>& buffer) :
        event_buffer(buffer), event_handler(nullptr)
    {

    }

    EventSerial::~EventSerial()
    {

    }

    bool EventSerial::CreaterConnect(const std::string& name)
    {
        std::string name_prefix = R"(\\.\)" + name;
#ifdef UNICODE
        std::wstring temp;
        {
            if (A2W(name_prefix, temp))
            {
                if (nullptr == event_handler)
                {
                    event_handler = CreateFile(
                        temp.c_str(),
                        GENERIC_READ | GENERIC_WRITE,                         // 允许读和写
                        0,                                                    // 指定共享属性，由于串口不能共享，该参数必须置为0,独占方式
                        nullptr,                                              // 引用安全性属性结构，缺省值为NULL
                        OPEN_EXISTING,                                        // 创建标志，对串口操作该参数必须置为OPEN_EXISTING
                        FILE_ATTRIBUTE_NORMAL,                                // 属性描述，此处指定该串口异步
                        nullptr);                                 // 对串口而言该参数必须置为NULL
                }

                return reinterpret_cast<HANDLE>(INVALID_HANDLE_VALUE) != event_handler;
            }
        }
#else
        if (nullptr == event_handler)
        {
            event_handler = CreateFile(
                name_prefix.c_str(),
                GENERIC_READ | GENERIC_WRITE,                         // 允许读和写
                0,                                                    // 指定共享属性，由于串口不能共享，该参数必须置为0,独占方式
                nullptr,                                              // 引用安全性属性结构，缺省值为NULL
                OPEN_EXISTING,                                        // 创建标志，对串口操作该参数必须置为OPEN_EXISTING
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,         // 属性描述，此处指定该串口异步
                nullptr);
        }

        return reinterpret_cast<HANDLE>(INVALID_HANDLE_VALUE) != event_handler;
#endif
        return false;
    }

    bool EventSerial::ConfigComState(u32 baudrate, bytesize_t bytesize, stopbits_t stopbits, parity_t parity, flowcontrol_t flowcontrol)
    {
        if (INVALID_HANDLE_VALUE == event_handler)
        {
            return false;
        }

        DCB dcb = {};
        {
            if (sizeof(DCB) != dcb.DCBlength)
            {
                dcb.DCBlength = sizeof(DCB);
            }

            if (GetCommState(event_handler, &dcb))
            {
                switch (baudrate)
                {
                    case CBR_300:       dcb.BaudRate = CBR_300;     break;
                    case CBR_600:       dcb.BaudRate = CBR_600;     break;
                    case CBR_1200:      dcb.BaudRate = CBR_1200;    break;
                    case CBR_2400:      dcb.BaudRate = CBR_2400;    break;
                    case CBR_4800:      dcb.BaudRate = CBR_4800;    break;
                    case CBR_9600:      dcb.BaudRate = CBR_9600;    break;
                    case CBR_14400:     dcb.BaudRate = CBR_14400;   break;
                    case CBR_19200:     dcb.BaudRate = CBR_19200;   break;
                    case CBR_38400:     dcb.BaudRate = CBR_38400;   break;
                    case CBR_56000:     dcb.BaudRate = CBR_56000;   break;
                    case CBR_115200:    dcb.BaudRate = CBR_115200;  break;
                    case CBR_128000:    dcb.BaudRate = CBR_128000;  break;
                    case CBR_256000:    dcb.BaudRate = CBR_256000;  break;
                }

                switch (bytesize)
                {
                    case eightbits:     dcb.ByteSize = 8; break;
                    case sevenbits:     dcb.ByteSize = 7; break;
                    case sixbits:       dcb.ByteSize = 6; break;
                    case fivebits:      dcb.ByteSize = 5; break;
                }

                switch (stopbits)
                {
                    case stopbits_one:  dcb.StopBits = ONESTOPBIT;    break;
                    case stopbits_five: dcb.StopBits = ONE5STOPBITS;  break;
                    case stopbits_two:  dcb.StopBits = TWOSTOPBITS;   break;
                }

                switch (parity)
                {
                    case parity_none:   dcb.Parity = NOPARITY;    break;
                    case parity_even:   dcb.Parity = EVENPARITY;  break;
                    case parity_odd:    dcb.Parity = ODDPARITY;   break;
                    case parity_mark:   dcb.Parity = MARKPARITY;  break;
                    case parity_space:  dcb.Parity = SPACEPARITY; break;
                }

                switch (flowcontrol)
                {
                    case flowcontrol_none:
                    {
                        dcb.fOutxCtsFlow = false;
                        dcb.fRtsControl = RTS_CONTROL_DISABLE;
                        dcb.fOutX = false;
                        dcb.fInX = false;
                        break;
                    }
                    case flowcontrol_software:
                    {
                        dcb.fOutxCtsFlow = false;
                        dcb.fRtsControl = RTS_CONTROL_DISABLE;
                        dcb.fOutX = true;
                        dcb.fInX = true;
                        break;
                    }
                    case flowcontrol_hardware:
                    {
                        dcb.fOutxCtsFlow = true;
                        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
                        dcb.fOutX = false;
                        dcb.fInX = false;
                        break;
                    }
                }

                if (SetCommState(event_handler, &dcb))
                {
                    COMMTIMEOUTS time = {}; //设定读超时
                    {
                        time.ReadIntervalTimeout = 0;
                        time.ReadTotalTimeoutMultiplier = 0;
                        time.ReadTotalTimeoutConstant = 0;
                        time.WriteTotalTimeoutMultiplier = 0;
                        time.WriteTotalTimeoutConstant = 0;
                    }
                    return 1 == SetCommTimeouts(event_handler, &time);
                }

                if (CloseHandle(event_handler))
                {
                    EVENT_ERROR("failed to set the status of serial communication parameters");
                }
            }
        }

        return false;
    }

    bool EventSerial::DestroyConnect()
    {
        if (INVALID_HANDLE_VALUE == event_handler)
        {
            return false;
        }

        return 1 == CloseHandle(event_handler);
    }

    bool EventSerial::Send(const std::string& buffer)
    {
        return Send(buffer.c_str(), buffer.size());
    }

    bool EventSerial::Send(const char* buffer, u64 size)
    {
        if (INVALID_HANDLE_VALUE == event_handler)
        {
            return false;
        }

        unsigned long writer_bytes = 0;
        {
            if (1 == WriteFile(event_handler, buffer, static_cast<unsigned long>(size), &writer_bytes, nullptr))
            {
                return size == static_cast<u96>(writer_bytes);
            }
        }
        return false;
    }

    bool EventSerial::Read()
    {
        if (INVALID_HANDLE_VALUE == event_handler)
        {
            return false;
        }

        unsigned long reader_bytes = 0; unsigned char buffer[1024];
        {
            if (1 == ReadFile(event_handler, buffer, static_cast<unsigned long>(std::size(buffer)), &reader_bytes, nullptr))
            {
                if (nullptr != event_buffer)
                {
                    return event_buffer->Append(buffer, reader_bytes);
                }
            }
        }

        return false;
    }
}