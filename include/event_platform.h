﻿#ifndef __EVENT_PLATFORM_H__
#define __EVENT_PLATFORM_H__

#ifdef H_OS_WINDOWS
#       define FORCEINLINE __forceinline
#       define NOFORCEINLINE __declspec (noinline)
#else
#   define FORCEINLINE __attribute__ ((always_inline))
#   define NOFORCEINLINE __attribute__ ((noinline))
#endif

// Use CXX20 coroutine to assist EventLoop operation.
#if defined(__cpp_coroutines) && !defined(__cpp_impl_coroutine) || defined(_HAS_CXX20)
#   ifndef EVPP_USE_STL_COROUTINES
//#       define EVPP_USE_STL_COROUTINES
#   endif
#endif

// Use Boost Assembly
#ifdef EVPP_USE_BOOST_ASSEMBLY
// b2 install --prefix="f:\common\boost\lib\x86" --toolset=msvc-14.2 --build-type=complete link=static runtime-link=static threading=multi debug release
#       ifndef EVPP_USE_BOOST_THREAD
#           define EVPP_USE_BOOST_THREAD
#       endif

#       ifndef EVPP_USE_BOOST_LOCKFREE_QUEUE
#           define EVPP_USE_BOOST_LOCKFREE_QUEUE
#       endif
#else
// Use STL-based multi-threaded running mode (std::thread) to close this macro, and use libuv_thread to run multi-threaded
#       ifndef EVPP_USE_STL_THREAD
#           define EVPP_USE_STL_THREAD
#       endif

// Use https://github.com/cameron314/concurrentqueue
#       ifndef EVPP_USE_CAMERON314_CONCURRENTQUEUE
#           define EVPP_USE_CAMERON314_CONCURRENTQUEUE
#       endif
#endif

#if (defined(_DEBUG) || defined(DEBUG) || defined(_DEBUG_))
#   define EVENT_DEBUG_MODE
#endif

#ifdef H_OS_WINDOWS
#   define EVPP_THREAD_YIELD SwitchToThread
#else
#   define EVPP_THREAD_YIELD std::this_thread::yield
#endif

#define USE_PARAMETER(P) (P)

#ifndef MAKEWORD
#define MAKEWORD(a, b)                      ((u16)(((u8)(((u32)(a)) & 0xff)) | ((u16)((u8)(((u32)(b)) & 0xff))) << 8))
#endif

#ifndef MAKELONG
#   define MAKELONG(a, b)                   ((u32)(((u16)(((u32)(a)) & 0xffff)) | ((u32)((u16)(((u32)(b)) & 0xffff))) << 16))
#endif

#ifndef MAKELONGLONG
#   define MAKELONGLONG(a, b)               ((u64)(((u32)(((u64)(a)) & 0xffffffff)) | ((u64)((u32)(((u64)(b)) & 0xffffffff))) << 32))
#endif

#ifndef LOWORD
#   define LOWORD(l)                        ((u16)(((u32)(l)) & 0xffff))
#endif

#ifndef HIWORD
#   define HIWORD(l)                        ((u16)((((u32)(l)) >> 16) & 0xffff))
#endif

#ifndef LODWORD
#   define LODWORD(l)                       ((u32)(((u64)(l)) & 0xffffffff))
#endif

#ifndef HIDWORD
#   define HIDWORD(l)                       ((u32)((((u64)(l)) >> 32) & 0xffffffff))
#endif

#ifndef LOBYTE
#   define LOBYTE(l)                        ((u8)(((u32)(l)) & 0xff))
#endif

#ifndef HIBYTE
#   define HIBYTE(l)                        ((u8)((((u32)(l)) >> 8) & 0xff))
#endif

#ifndef GETBIT
#   define GETBIT(x, y)                     (((x) >> (y)) & 1)
#endif

#ifdef H_OS_WINDOWS
#   pragma warning( disable: 4251 )
#   pragma warning( disable: 4996 )
#   pragma warning( disable: 4244 4251 4355 4715 4800 4996 4005 4819)
#   pragma warning( disable: 4505 )
#   pragma warning( disable: 4530 )
#   pragma warning( disable: 4577 )
#   pragma warning( disable: 4503 )
#   pragma warning( disable: 4458 )
#   pragma warning( disable: 4100 )
#   pragma warning( disable: 4702 )
#endif

#ifdef H_OS_WINDOWS
#   pragma comment(lib, "ws2_32.lib")
#   pragma comment(lib, "wldap32.lib")
#   pragma comment(lib, "iphlpapi.lib")
#   pragma comment(lib, "psapi.lib")
#   pragma comment(lib, "userenv.lib")
#   pragma comment(lib, "crypt32.lib")
#   pragma comment(lib, "onecore.lib")
#   pragma comment(lib, "onecoreuap.lib")
#endif

#endif // __EVENT_PLATFORM_H__