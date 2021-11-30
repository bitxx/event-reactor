#ifndef __EVENT_UNTILITY_H__
#define __EVENT_UNTILITY_H__

#   define EVPP_GET_SOCKET_ERROR()      WSAGetLastError()

#   define EVPP_SET_SOCKET_ERROR(x)     WSASetLastError(x)

#if defined(H_OS_WINDOWS)
#   define CLOSE_ERROR(e)   (WSAENOTSOCK == e)
#elif defined(H_OS_LINUX)
#   define INVALID_SOCKET   (~0)
#   define CLOSE_ERROR(e)   (ENOSYS == e) || (EBADF == e)
#endif

#   define EVPP_SUCCESS(x)              (0x00000000 == static_cast<i32>(x))
#   define EVPP_FAILURE(x)              (0x00000000 != static_cast<i32>(x))

#endif // __EVENT_UNTILITY_H__