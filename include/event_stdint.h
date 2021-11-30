#ifndef __EVENT_STDINT_H__
#define __EVENT_STDINT_H__

#include <stdint.h>
#include <iostream>
#include <string>
#include <limits>

typedef signed char                 i8;
typedef short                       i16;
typedef int                         i32;
typedef long long                   i64;
typedef unsigned char               u8;
typedef unsigned short              u16;
typedef unsigned int                u32;
typedef unsigned long long          u64;
#if defined(H_OS_X64)
typedef signed long long            i96;
typedef unsigned long long          u96;
#elif defined(H_OS_X86)
typedef signed int                  i96;
typedef unsigned int                u96; 

#endif

#endif // __EVENT_STDINT_H__