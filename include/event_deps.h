#ifndef __EVENT_DEPS_H__
#define __EVENT_DEPS_H__

#       include <algorithm>     // std::min
#       include <cerrno>        // errno
#       include <cstddef>       // std::size_t
#       include <cstdio>        // std::FILE, std::fopen, std::fclose, std::fwrite
#       include <cstdlib>       // EXIT_SUCCESS, EXIT_FAILURE
#       include <iostream>      // std::cerr, std::endl
#       include <memory>        // std::unique_ptr
#       include <string>        // std::string
#       include <system_error>  // std::system_error, std::system_category
#       include <thread>        // std::thread
#       include <mutex>         // std::mutex
#       include <any>           // std::any
#       include <map>           // std::map
#       include <unordered_map> // std::unordered_map
#       include <queue>         // std::queue
#       include <vector>        // std::vector
#       include <semaphore>     // std::semaphore
#       include <future>        // std::async
#       include <atomic>        // std::atomic
#       include <functional>    // std::function

#       include <event_logging.h>

#       include <event2/event.h>
#       include <event2/event_compat.h>
#       include <event2/event_struct.h>
#       include <event2/buffer.h>
#       include <event2/bufferevent.h>
#       include <event2/http.h>
#       include <event2/http_compat.h>
#       include <event2/http_struct.h>
#       include <event2/dns.h>
#       include <event2/dns_compat.h>
#       include <event2/dns_struct.h>
#       include <event2/listener.h>
#       include <event2/thread.h>


#if defined(H_OS_LINUX)
#       include <fcntl.h>
#       include <sys/socket.h>
#       include <sys/ioctl.h>
#       include <netinet/tcp.h>
#       include <netinet/in.h>
#       include <arpa/inet.h>
#endif
#endif // __EVENT_DEPS_H__