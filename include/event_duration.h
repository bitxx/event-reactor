#ifndef __EVENT_DURATION_H__
#define __EVENT_DURATION_H__
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

namespace std
{
    namespace chrono
    {
        namespace detail
        {
            template<typename From, typename To>
            struct posix_duration_cast;

            // chrono -> timeval caster
            template<typename Rep, typename Period>
            struct posix_duration_cast<std::chrono::duration<Rep, Period>, struct timeval>
            {
                static struct timeval cast(std::chrono::duration<Rep, Period> const& d)
                {
                    struct timeval tv;
                    std::chrono::seconds const sec = std::chrono::duration_cast<std::chrono::seconds>(d);
                    tv.tv_sec = sec.count();
                    tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(d - sec).count();
                    return std::move(tv);
                }
            };

            // timeval -> chrono caster
            template<typename Rep, typename Period>
            struct posix_duration_cast<struct timeval, std::chrono::duration<Rep, Period>>
            {
                static std::chrono::duration<Rep, Period> cast(struct timeval const& tv)
                {
                    return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec));
                }
            };
        }

        // chrono -> timeval
        template<typename T, typename Rep, typename Period>
        auto duration_cast(std::chrono::duration<Rep, Period> const& d)
            -> std::enable_if_t<std::is_same<T, struct timeval>::value, struct timeval>
        {
            return detail::posix_duration_cast<std::chrono::duration<Rep, Period>, timeval>::cast(d);
        }

        // timeval -> chrono
        template<typename Duration>
        Duration duration_cast(struct timeval const& tv)
        {
            return detail::posix_duration_cast<struct timeval, Duration>::cast(tv);
        }
    }
}

namespace Evpp
{
    using namespace std::literals::chrono_literals;

    using milliseconds = std::chrono::duration<double, std::ratio<1, 1000>>;
    using microseconds = std::chrono::duration<double, std::ratio<1, 10000>>;

    template <class _Rep, class _Period>
    void duration_sleep_for(const std::chrono::duration<_Rep, _Period>& _Rel_time)
    {
        std::this_thread::sleep_until(std::chrono::steady_clock::now() + _Rel_time);
    }

    static void duration_sleep_until(const double v)
    {
        std::this_thread::sleep_for(milliseconds(v));
    }

    static bool CheckSystemClock(const milliseconds& v, const double t)
    {
        return std::chrono::duration_cast<milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() - v).count() < t;
    }

    static u64 ComplexUnixTimer()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static std::string SerializeChronoDate(const char* format = "%Y-%m-%d %H:%M:%S") // format = %F %T
    {
        char buffer[40] = {};
        {
            if (std::strftime(
                buffer,
                std::size(buffer),
                format,
                std::localtime(std::addressof(static_cast<const std::time_t&>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))))
            ))
            {
                return std::string(buffer);
            }
        }

        throw std::runtime_error("failed to get current date as string");
    }

    static std::string SerializeChronoDate(const __time64_t unixtime, const char* format = "%Y-%m-%d %H:%M:%S") // format = %F %T
    {
        char buffer[40] = {};
        {
            if (std::strftime(
                buffer,
                std::size(buffer),
                format,
                std::localtime(std::addressof(static_cast<const std::time_t&>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(unixtime)))))
            ))
            {
                return std::string(buffer);
            }
        }

        throw std::runtime_error("failed to get current date as string");
    }
}
#endif // __EVENT_DURATION_H__
