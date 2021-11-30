#ifndef __EVENT_STRUCT_H__
#define __EVENT_STRUCT_H__
namespace Evpp
{
    namespace Details
    {
        struct EventFamily
        {
            union
            {
                u16                 family;
                sockaddr_storage    ss;
                sockaddr            sa;
                sockaddr_in         sin;
                sockaddr_in6        sin6;
            };
        };
    }
}
#endif // __EVENT_STRUCT_H__