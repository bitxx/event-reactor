#ifndef __EVENT_BYTESWAP_H__
#define __EVENT_BYTESWAP_H__

#ifndef __bswap_constant_16__
#   define __bswap_constant_16__(x) (((static_cast<uint16_t>(x) >> 8) & 0xff) | ((static_cast<uint16_t>(x) & 0xff) << 8))
#endif

#ifndef __bswap_constant_32__
#   define __bswap_constant_32__(x) (((static_cast<uint32_t>(x) & 0xff000000) >> 24) | ((static_cast<uint32_t>(x) & 0x00ff0000) >>  8) | ((static_cast<uint32_t>(x) & 0x0000ff00) <<  8) | ((static_cast<uint32_t>(x) & 0x000000ff) << 24))
#endif

#ifndef __bswap_constant_64__
#   define __bswap_constant_64__(x) (((static_cast<uint64_t>(x) & 0xff00000000000000ull) >> 56) | ((static_cast<uint64_t>(x) & 0x00ff000000000000ull) >> 40) | ((static_cast<uint64_t>(x) & 0x0000ff0000000000ull) >> 24) | ((static_cast<uint64_t>(x) & 0x000000ff00000000ull) >> 8) | ((static_cast<uint64_t>(x) & 0x00000000ff000000ull) << 8) | ((static_cast<uint64_t>(x) & 0x0000000000ff0000ull) << 24) | ((static_cast<uint64_t>(x) & 0x000000000000ff00ull) << 40) | ((static_cast<uint64_t>(x) & 0x00000000000000ffull) << 56))
#endif

#ifndef __bswap_16__
#   define __bswap_16__         __bswap_constant_16__
#endif

#ifndef __bswap_32__
#   define __bswap_32__         __bswap_constant_32__
#endif

#ifndef __bswap_64__
#   define __bswap_64__         __bswap_constant_64__
#endif

#   if defined(H_OS_X64)
#       ifndef __bswap_96__
#           define __bswap_96__         __bswap_constant_64__
#       endif
#   elif defined(H_OS_X86)
#       ifndef __bswap_96__
#           define __bswap_96__         __bswap_constant_32__
#       endif
#   endif

#ifndef swap16
#   define swap16               __bswap_16__
#endif

#ifndef swap32
#   define swap32               __bswap_32__
#endif

#ifndef swap64
#   define swap64               __bswap_64__
#endif

#ifndef htons
#   define htons                __bswap_16__
#endif

#ifndef htonl
#   define htonl                __bswap_32__
#endif

#ifndef htonll
#   define htonll               __bswap_64__
#endif

#ifndef ntohs
#   define ntohs                __bswap_16__
#endif

#ifndef ntohl
#   define ntohl                __bswap_32__
#endif

#ifndef ntohll
#   define ntohll               __bswap_64__
#endif

#endif // __EVENT_BYTESWAP_H__