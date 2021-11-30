#include <event_loop_thread.h>
#include <event_loop.h>
namespace Evpp
{
    EventLoopThread::EventLoopThread(EventLoop* basic, u96 index) :
        EventLoopThread(basic, EventThreadDestroyHandler(), index)
    {
        EVENT_TRACE("EventLoopThread::EventLoopThread");
    }

    EventLoopThread::EventLoopThread(EventLoop* basic, const EventThreadDestroyHandler& destroy, u96 index) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_destroy(destroy),
        event_index(index),
        event_loops(nullptr),
        event_prepare(0),
        event_thread(nullptr)
    {
        EVENT_TRACE("EventLoopThread::EventLoopThread");
    }

    EventLoopThread::~EventLoopThread()
    {
        EVENT_TRACE("EventLoopThread::~EventLoopThread");
    }

    bool EventLoopThread::CreaterThread(const std::chrono::milliseconds& delay, const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        if (nullptr == event_basic)
        {
            return false;
        }

        if (ExistsInited() || ExistsRuning())
        {
            return true;
        }

        if (ChangeStatus(Status::Init))
        {
            try
            {
                if (event_safe_thread.valid())
                {
                    return true;
                }
                else
                {
                    // 使用 async 替换 thread 有效解决线程频繁创建以及关闭耗费的内核资源
                    //event_safe_thread = std::async(std::launch::async, std::bind(&EventLoopThread::ExecDispatch, this, star, stop));
                    event_thread.reset(new std::thread(std::bind(&EventLoopThread::ExecDispatch, this, star, stop)));
                }
            }
            catch (const std::system_error& ex)
            {
                /*
                    // names for generic error codes
                    address_family_not_supported       = 102, // EAFNOSUPPORT
                    address_in_use                     = 100, // EADDRINUSE
                    address_not_available              = 101, // EADDRNOTAVAIL
                    already_connected                  = 113, // EISCONN
                    argument_list_too_long             = 7, // E2BIG
                    argument_out_of_domain             = 33, // EDOM
                    bad_address                        = 14, // EFAULT
                    bad_file_descriptor                = 9, // EBADF
                    bad_message                        = 104, // EBADMSG
                    broken_pipe                        = 32, // EPIPE
                    connection_aborted                 = 106, // ECONNABORTED
                    connection_already_in_progress     = 103, // EALREADY
                    connection_refused                 = 107, // ECONNREFUSED
                    connection_reset                   = 108, // ECONNRESET
                    cross_device_link                  = 18, // EXDEV
                    destination_address_required       = 109, // EDESTADDRREQ
                    device_or_resource_busy            = 16, // EBUSY
                    directory_not_empty                = 41, // ENOTEMPTY
                    executable_format_error            = 8, // ENOEXEC
                    file_exists                        = 17, // EEXIST
                    file_too_large                     = 27, // EFBIG
                    filename_too_long                  = 38, // ENAMETOOLONG
                    function_not_supported             = 40, // ENOSYS
                    host_unreachable                   = 110, // EHOSTUNREACH
                    identifier_removed                 = 111, // EIDRM
                    illegal_byte_sequence              = 42, // EILSEQ
                    inappropriate_io_control_operation = 25, // ENOTTY
                    interrupted                        = 4, // EINTR
                    invalid_argument                   = 22, // EINVAL
                    invalid_seek                       = 29, // ESPIPE
                    io_error                           = 5, // EIO
                    is_a_directory                     = 21, // EISDIR
                    message_size                       = 115, // EMSGSIZE
                    network_down                       = 116, // ENETDOWN
                    network_reset                      = 117, // ENETRESET
                    network_unreachable                = 118, // ENETUNREACH
                    no_buffer_space                    = 119, // ENOBUFS
                    no_child_process                   = 10, // ECHILD
                    no_link                            = 121, // ENOLINK
                    no_lock_available                  = 39, // ENOLCK
                    no_message_available               = 120, // ENODATA
                    no_message                         = 122, // ENOMSG
                    no_protocol_option                 = 123, // ENOPROTOOPT
                    no_space_on_device                 = 28, // ENOSPC
                    no_stream_resources                = 124, // ENOSR
                    no_such_device_or_address          = 6, // ENXIO
                    no_such_device                     = 19, // ENODEV
                    no_such_file_or_directory          = 2, // ENOENT
                    no_such_process                    = 3, // ESRCH
                    not_a_directory                    = 20, // ENOTDIR
                    not_a_socket                       = 128, // ENOTSOCK
                    not_a_stream                       = 125, // ENOSTR
                    not_connected                      = 126, // ENOTCONN
                    not_enough_memory                  = 12, // ENOMEM
                    not_supported                      = 129, // ENOTSUP
                    operation_canceled                 = 105, // ECANCELED
                    operation_in_progress              = 112, // EINPROGRESS
                    operation_not_permitted            = 1, // EPERM
                    operation_not_supported            = 130, // EOPNOTSUPP
                    operation_would_block              = 140, // EWOULDBLOCK
                    owner_dead                         = 133, // EOWNERDEAD
                    permission_denied                  = 13, // EACCES
                    protocol_error                     = 134, // EPROTO
                    protocol_not_supported             = 135, // EPROTONOSUPPORT
                    read_only_file_system              = 30, // EROFS
                    resource_deadlock_would_occur      = 36, // EDEADLK
                    resource_unavailable_try_again     = 11, // EAGAIN
                    result_out_of_range                = 34, // ERANGE
                    state_not_recoverable              = 127, // ENOTRECOVERABLE
                    stream_timeout                     = 137, // ETIME
                    text_file_busy                     = 139, // ETXTBSY
                    timed_out                          = 138, // ETIMEDOUT
                    too_many_files_open_in_system      = 23, // ENFILE
                    too_many_files_open                = 24, // EMFILE
                    too_many_links                     = 31, // EMLINK
                    too_many_symbolic_link_levels      = 114, // ELOOP
                    value_too_large                    = 132, // EOVERFLOW
                    wrong_protocol_type                = 136 // EPROTOTYPE
                */
                switch (ex.code().value())
                {
                    case static_cast<i32>(std::errc::resource_unavailable_try_again) : EVENT_ERROR("线程创建失败尝试重建"); event_thread.reset(new std::thread(std::bind(&EventLoopThread::ExecDispatch, this, star, stop))); return false;
                    default: return false;
                }
            }
            catch (const std::bad_alloc& ex)
            {
                EVENT_ERROR("内存分配失败: %s", ex.what());
            }
            return event_prepare.try_acquire_for(delay);
        }

        return false;
    }

    bool EventLoopThread::DestroyThread(const std::chrono::milliseconds& delay)
    {
        if (nullptr == event_basic || nullptr == event_loops)
        {
            return false;
        }

        if (ExistsUnlade() || ExistsStoped() || ExistsExited())
        {
            return true;
        }

        if (ExistsInited() || ExistsRuning())
        {
            if (0 == event_loops->StopDispatch())
            {
                return false;
            }

            if (event_safe_thread.valid())
            {
                switch (event_safe_thread.wait_for(delay))
                {
                    case std::future_status::ready:     EVENT_INFO("线程销毁 -> 已经完成"); break;
                    case std::future_status::timeout:   EVENT_INFO("线程销毁 -> 已经超时"); break;
                    case std::future_status::deferred:  EVENT_INFO("线程销毁 -> 还未执行"); break;
                    default: break;
                }
            }

            if (nullptr != event_thread)
            {
                if (event_thread->joinable())
                {
                    event_thread->join();
                }
            }

            return RemoveStatus(Status::Init) && RemoveStatus(Status::Exec) && ChangeStatus(Status::Exit);
        }
        return false;
    }

    EventLoop* EventLoopThread::GetEventLoop()
    {
        if (ExistsUnlade() || ExistsStoped() || ExistsExited())
        {
            return nullptr;
        }

        if (ExistsInited() && ExistsRuning())
        {
            return nullptr == event_loops ? event_basic : event_loops.get();
        }
        return nullptr;
    }

    void EventLoopThread::ExecDispatch(const EventThreadStarHandler& star, const EventThreadStopHandler& stop)
    {
        if (nullptr == event_loops)
        {
            event_loops.reset(new EventLoop());
        }

        if (event_loops->InitEventBasic() && event_loops->ExecDispatch(EVLOOP_NONBLOCK))
        {
            event_prepare.release();
        }

        if (0 == ChangeStatus(Status::Exec))
        {
            EVENT_ERROR("修改状态发生异常错误");
        }

        if (event_loops->RunInQueue(std::move([&] { if (star) { if (0 == star(event_loops.get(), event_index)) { if (nullptr != event_basic) { return event_basic->RunInQueue(std::bind(&EventLoopThread::DestroyThread, this, std::chrono::milliseconds(3000))); } } } return false; })))
        {
            EVENT_INFO("thread run queue exec start event");
        }

        if (0 == event_loops->ExecDispatch(0))
        {
            EVENT_ERROR("thread event [0] loop error");
        }

        if (nullptr != stop)
        {
            stop(event_index);
        }

        if (ChangeStatus(Status::Stop))
        {
            EVENT_INFO("thread event loop stop");
        }

        // 转到主线程将线程池内的 当前线程对象 销毁，因为该对象创建时就是由主线程创建的，遵循操作系统销毁也由同一线程销毁
        if (nullptr != event_destroy && RunInQueue(std::move(std::bind(event_destroy, event_index))))
        {
            EVENT_INFO("异步销毁线程");
        }
    }
}