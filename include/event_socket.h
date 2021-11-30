#ifndef __EVENT_SOCKET_H__
#define __EVENT_SOCKET_H__
#include <event_core.h>
namespace Evpp
{
    /// https://www.cnblogs.com/yizhizaiYI/articles/5236221.html
    bool CreateTcpSocket(const u32 family, const u32 type, const u32 protocol, evutil_socket_t& fd);
    // 异步连接
    bool SetupSockBlock(evutil_socket_t fd, const i32 optval = 1);
    // 异步I/O
    bool SetupSockASync(evutil_socket_t fd, const i32 optval = 1);
    // 内核保活
    bool SetupKeepalive(evutil_socket_t fd, const i32 optval = 1);
    // 端口复用
    bool SetupReuseaddr(evutil_socket_t fd, const i32 optval = 1);
    // 发送缓冲
    bool SetupSendiSize(evutil_socket_t fd, const i32 optval = 1);
    // 接收缓冲`
    bool SetupRecviSize(evutil_socket_t fd, const i32 optval = 1);
    // 端口独占
#if defined(H_OS_WINDOWS)
    bool SetupExclusive(evutil_socket_t fd, const i32 optval = 1);
#endif
    // Delay算法
    bool SetupSockDelay(evutil_socket_t fd, const i32 optval = 1);
    // 跳过等待
#if defined(H_OS_WINDOWS)
    bool SetupTimerWait(evutil_socket_t fd, const i32 optval = 1);
#endif
    // TCP6独占
    bool SetupSixerOnly(evutil_socket_t fd, const i32 optval = 1);
    // 立即关闭
    bool SetupCloseOnce(evutil_socket_t fd, const linger& optval = linger{ 1, 0 });
    // 接收超时
    bool SetupSendTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay);
    // 发送超时
    bool SetupRecvTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay);
    // 连接超时
    bool SetupConnTimer(evutil_socket_t fd, const std::chrono::milliseconds& delay);
    // 获取地址
#ifdef H_OS_WINDOWS
    template<typename _Ty>
    bool GetSockStorage(evutil_socket_t fd, _Ty& t, i32 size = sizeof(_Ty))
#elif H_OS_LINUX
    template<typename _Ty>
    bool GetSockStorage(evutil_socket_t fd, _Ty& t, u32 size = sizeof(_Ty))
#endif
    {
        return 0 == getsockname(fd, reinterpret_cast<struct sockaddr*>(&t),  &size);
    }
    // 获取地址
#ifdef H_OS_WINDOWS
    template<typename _Ty>
    bool GetPeerStorage(evutil_socket_t fd, _Ty& t, i32 size = sizeof(_Ty))
#elif H_OS_LINUX
    template<typename _Ty>
    bool GetPeerStorage(evutil_socket_t fd, _Ty& t, u32 size = sizeof(_Ty))
#endif
    {
        return 0 == getpeername(fd, reinterpret_cast<struct sockaddr*>(&t), &size);
    }
    // 关闭句柄
    bool CloseSocketHandler(evutil_socket_t& handler);
   
#if defined(H_OS_WINDOWS)
    bool SocketFormatErrorString(std::string& format);
    bool SocketFormatErrorString(const u32 code, std::string& format);
#endif
}
#endif // __EVENT_SOCKET_H__