#include <event_socket.h>
namespace Evpp
{
    bool CreateTcpSocket(const u32 family, const u32 type, const u32 protocol, evutil_socket_t& fd)
    {
        if (fd = socket(family, type, protocol); fd != INVALID_SOCKET)
        {
            // 地址复用
            if (0 == SetupReuseaddr(fd))
            {
                return false;
            }

            // 连接超时
            if (0 == SetupConnTimer(fd, std::chrono::milliseconds(3)))
            {
                return false;
            }

            // 异步连接
            if (0 == SetupSockBlock(fd))
            {
                return false;
            }

            // 设置保活
            if (0 == SetupKeepalive(fd))
            {
                return false;
            }

            // 关闭 Delay 算法
            if (0 == SetupSockDelay(fd))
            {
                return false;
            }

            if(0 == SetupCloseOnce(fd))
            {
                return false;
            }

            return true;
        }
        return false;
    }

    bool SetupSockBlock(evutil_socket_t fd, i32 optval)
    {
#if defined(H_OS_WINDOWS)
        return 0 == ioctlsocket(fd, FIONBIO, std::addressof(reinterpret_cast<unsigned long&>(const_cast<i32&>(optval))));
#elif defined(H_OS_LINUX)
        return 0 == ioctl(fd, FIONBIO, std::addressof(reinterpret_cast<unsigned long&>(const_cast<i32&>(optval))));
#endif
    }

    bool SetupSockASync(evutil_socket_t fd, i32 optval)
    {
#if defined(H_OS_WINDOWS)
        return 0 == ioctlsocket(fd, FIOASYNC, std::addressof(reinterpret_cast<unsigned long&>(const_cast<i32&>(optval))));
#elif defined(H_OS_LINUX)
        return 0 == ioctl(fd, O_NONBLOCK, std::addressof(reinterpret_cast<unsigned long&>(const_cast<i32&>(optval))));
#endif
    }

    bool SetupKeepalive(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }

    bool SetupReuseaddr(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }

    bool SetupSendiSize(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }

    bool SetupRecviSize(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }
#if defined(H_OS_WINDOWS)
    bool SetupExclusive(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }
#endif

    bool SetupSockDelay(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, TCP_NODELAY, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }
#if defined(H_OS_WINDOWS)
    bool SetupTimerWait(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }
#endif

    bool SetupSixerOnly(evutil_socket_t fd, const i32 optval)
    {
        return 0 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, std::addressof(reinterpret_cast<const char&>(const_cast<i32&>(optval))), sizeof(i32));
    }

    bool SetupCloseOnce(evutil_socket_t fd, const linger& optval)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_LINGER, std::addressof(reinterpret_cast<const char&>(const_cast<linger&>(optval))), sizeof(linger));
    }

    bool SetupSendTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(std::addressof(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))), sizeof(struct timeval));
    }

    bool SetupRecvTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay)
    {
        return 0 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(std::addressof(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))), sizeof(struct timeval));
    }

    bool SetupConnTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay)
    {
#if defined(H_OS_WINDOWS)
        // windows
        return 0 == setsockopt(fd, IPPROTO_TCP, TCP_MAXRT, reinterpret_cast<const char*>(std::addressof(static_cast<const i32&>(delay.count()))), sizeof(i32));
#elif defined( MY_MACOSX )
        // (billhoo) MacOS uses TCP_CONNECTIONTIMEOUT to do so.
        return 0 == setsockopt(fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, reinterpret_cast<const char*>(std::addressof(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))), sizeof(struct timeval));
#else   // linux like systems
        return 0 == setsockopt(fd, IPPROTO_TCP, TCP_USER_TIMEOUT, reinterpret_cast<const char*>(std::addressof(static_cast<const struct timeval&>(std::chrono::duration_cast<struct timeval>(delay)))), sizeof(struct timeval));
#endif
    }

    bool CloseSocketHandler(evutil_socket_t& handler)
    {
        if (0 == evutil_closesocket(handler))
        {
            return true;
        }

        if (CLOSE_ERROR(evutil_socket_geterror(handler)))
        {
            if (INVALID_SOCKET != handler)
            {
                handler = INVALID_SOCKET;
            }
            return true;
        }
        else
        {
            EVENT_FATAL("关闭句柄失败");
        }

        return false;
    }
#if defined(H_OS_WINDOWS)
    bool SocketFormatErrorString(std::string& format)
    {
        char FormatString[256] = {};
        {
            if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, EVPP_GET_SOCKET_ERROR(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), FormatString, static_cast<u32>(std::size(FormatString)), NULL))
            {
                if (format.size())
                {
                    format.clear();
                }
                return format.append(FormatString).size();
            }
        }

        return false;
    }

    bool SocketFormatErrorString(const u32 code, std::string& format)
    {
        char FormatString[256] = {};
        {
            if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), FormatString, static_cast<u32>(std::size(FormatString)), NULL))
            {
                if (format.empty())
                {
                    format.append(FormatString);
                }
                return true;
            }
        }
        return false;
    }
#endif
}