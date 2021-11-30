#ifndef __EVENT_EXPLICIT_H__
#define __EVENT_EXPLICIT_H__

#include <event_core.h>

namespace Evpp
{
#ifdef H_OS_WINDOWS
    class EventExplicit
    {
    public:
        explicit EventExplicit() : wsaData({})
        {
            if (WSAStartup(MAKEWORD(2, 2), &wsaData))
            {
                EVENT_ERROR("could not initialize libevent");
            }
        }

        virtual ~EventExplicit()
        {
            WSACleanup();
            std::system("pause");
        }
    private:
        WSADATA wsaData;
    } __s_onexit_pause;
#endif
}
#endif // __EVENT_EXPLICIT_H__
