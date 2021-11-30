#ifndef __LOGGING_H__
#define __LOGGING_H__

#   ifndef EVENT_HAVE_TRACE
#       define EVENT_HAVE_TRACE
#   endif /* EVENT_HAVE_TRACE */

// if you need to close the system printout, please use the macro "GOOGLE_LOGGING" to close it
#   ifndef GOOGLE_GLOG_DLL_DECL
#       define GOOGLE_GLOG_DLL_DECL
#   endif

#   ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#       define GLOG_NO_ABBREVIATED_SEVERITIES
#   endif

#   ifndef HAVE_CXX11_ATOMIC
#       define HAVE_CXX11_ATOMIC
#endif

#include <glog/logging.h>
#include <glog/raw_logging.h>

#ifdef GOOGLE_LOGGING
#       define EVENT_CANCEL_LOGGING 1
#       define EVENT_SYSTEM_LOGGING 1
#endif // GOOGLE_LOGGING

#if defined(EVENT_DEBUG_MODE)
#       ifndef GOOGLE_LOGGING
#           define GOOGLE_LOG(severity)                             LOG(severity) 
#           define                                                  DebugMsg(severity, ...) { char debugMsg[2048]; sprintf(debugMsg, ##__VA_ARGS__); google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity).stream() << debugMsg; };
#       else
#           define GOOGLE_LOG(severity)                             LOG(severity) 
#           define                                                  DebugMsg(severity, ...) { };
#       endif
#else
#           define GOOGLE_LOG(severity)                             LOG(severity) 
#           define                                                  DebugMsg(severity, ...) { char debugMsg[2048]; sprintf(debugMsg, ##__VA_ARGS__); google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity).stream() << debugMsg; };
#endif

//#ifndef GOOGLE_LOGGING
//#       define LOG_INFO                                             GOOGLE_LOG(INFO)
//#       define LOG_WARN                                             GOOGLE_LOG(WARNING)
//#       define LOG_ERROR                                            GOOGLE_LOG(ERROR)
//#       define LOG_FATAL                                            GOOGLE_LOG(FATAL)
//#elif   EVENT_CANCEL_LOGGING
//#       define LOG_INFO                                             GOOGLE_LOG(INFO)
//#       define LOG_WARN                                             GOOGLE_LOG(WARNING)
//#       define LOG_ERROR                                            GOOGLE_LOG(ERROR)
//#       define LOG_FATAL                                            GOOGLE_LOG(FATAL)
//#endif // GOOGLE_LOGGING

#ifndef GOOGLE_LOGGING
#       define EVENT_INFO(...)                                      DebugMsg(INFO, __VA_ARGS__)
#       define EVENT_WARN(...)                                      DebugMsg(WARNING, __VA_ARGS__)
#       define EVENT_ERROR(...)                                     DebugMsg(ERROR, __VA_ARGS__)
#       define EVENT_FATAL(...)                                     DebugMsg(FATAL, __VA_ARGS__)
#elif   EVENT_SYSTEM_LOGGING
#       define EVENT_INFO(...)                                      DebugMsg(INFO, __VA_ARGS__)
#       define EVENT_WARN(...)                                      DebugMsg(WARNING, __VA_ARGS__)
#       define EVENT_ERROR(...)                                     DebugMsg(ERROR, __VA_ARGS__)
#       define EVENT_FATAL(...)                                     DebugMsg(FATAL, __VA_ARGS__)
#endif // GOOGLE_LOGGING

#if defined(EVENT_HAVE_TRACE)
#       define EVENT_TRACE(...)                                     DebugMsg(INFO, __VA_ARGS__)
#else
#       define EVENT_TRACE(...)
#endif

#endif // __LOGGING_H__