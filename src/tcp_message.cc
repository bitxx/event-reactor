#include <network/tcp/tcp_message.h>
#include <event_loop.h>
#include <event_poll.h>
#include <event_buffer.h>
namespace Evpp
{
    TcpMessage::TcpMessage(EventLoop* basic,
                           evutil_socket_t fd,
                           const Details::InterfaceMessages& messages,
                           const Details::InterfaceDiscon& discon) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_descriptor(fd),
        event_messages(messages),
        event_discon(discon),
        event_reads_data(nullptr),
        event_write_data(evbuffer_new()),
        event_reads_buff(std::make_shared<EventBuffer>()),
        event_write_buff(std::make_shared<EventBuffer>())
    {
        EVENT_TRACE("TcpMessage::TcpMessage");
    }

    TcpMessage::~TcpMessage()
    {
        EVENT_TRACE("TcpMessage::~TcpMessage");
    }

    bool TcpMessage::InitialMessage()
    {
        /*
         * BEV_OPT_CLOSE_ON_FREE：#释放bufferevent时关闭底层传输端口。这将关闭底层套接字，释放底层bufferevent等。
         * BEV_OPT_THREADSAFE：#自动为bufferevent分配锁，这样就可以安全地在多个线程中使用bufferevent。
         * BEV_OPT_DEFER_CALLBACKS：#设置这个标志时，bufferevent延迟所有回调，如上所述。
         *
         * 默认情况下，如果设置bufferevent为线程安全的，则bufferevent会在调用用户提供的回调时进行锁定。
         * 设置这个选项会让libevent在执行回调的时候不进行锁定。
         * BEV_OPT_UNLOCK_CALLBACKS：
         */

        if (nullptr == event_reads_data)
        {
            event_reads_data = bufferevent_socket_new(event_basic->EventBasic(), event_descriptor, BEV_OPT_THREADSAFE);
        }

        if (nullptr == event_reads_data)
        {
            EVENT_INFO("TcpMessage 初始化失败");
        }
        else
        {
            EVENT_WARN("TcpMessage 初始化成功");
        }

        if (nullptr != event_reads_data)
        {
            bufferevent_setcb(event_reads_data,
                              [] (struct bufferevent* bev, void* data)
                              {
                                  if (nullptr != data)
                                  {
                                      TcpMessage* watcher = static_cast<TcpMessage*>(data);
                                      {
                                          if (nullptr == watcher)
                                          {
                                              return;
                                          }
                                          return watcher->OnReadsNotify(bev);
                                      }
                                  }
                              },
                              0, [] (struct bufferevent* bev, short events, void* data)
                              {
                                  if (nullptr != data)
                                  {
                                      TcpMessage* watcher = static_cast<TcpMessage*>(data);
                                      {
                                          if (nullptr == watcher)
                                          {
                                              return;
                                          }
                                          return watcher->OnEventNotify(bev, events);
                                      }
                                  }
                              }, this);

            return 0 == bufferevent_enable(event_reads_data, EV_READ | EV_WRITE | EV_PERSIST | EV_ET);
        }
        return false;
    }

    bool TcpMessage::Send(const char* buffer, u96 size)
    {
        if (nullptr == event_write_data || 0 == size)
        {
            return false;
        }

        std::unique_lock<std::mutex> lock(event_write_lock);
        {
            if (EventThread())
            {
                return DoSend(buffer, size);
            }

            if (nullptr != event_write_buff)
            {
                if (event_write_buff->Append(buffer, size))
                {
                    return RunInQueue(std::bind<bool(TcpMessage::*)(const std::shared_ptr<EventBuffer>&, u96)>(&TcpMessage::DoSend, this, event_write_buff, size));
                }
            }
        }

        return false;
    }

    bool TcpMessage::Send(const std::string& buffer)
    {
        if (buffer.empty())
        {
            return false;
        }

        return Send(buffer.data(), buffer.size());
    }

    bool TcpMessage::DoSend(const std::shared_ptr<EventBuffer>& buffer, u96 len)
    {
        if (nullptr != buffer || 0 == len)
        {
            std::unique_lock<std::mutex> lock(event_write_lock);
            {
                if (DoSend(buffer->Peek(), len))
                {
                    return buffer->Skip(len);
                }
            }
        }

        return false;
    }

    bool TcpMessage::DoSend(const char* buffer, u96 len)
    {
        if (0 == evbuffer_add(event_write_data, buffer, len))
        {
            return len == static_cast<u96>(evbuffer_write(event_write_data, event_descriptor));
        }

        return false;
    }

    bool TcpMessage::Close(const Handler& callback)
    {
        if (nullptr == event_reads_data)
        {
            return false;
        }

        if (CloseSocketHandler(event_descriptor))
        {
            if (0 == bufferevent_disable(event_reads_data, EV_READ | EV_WRITE | EV_PERSIST | EV_ET))
            {
                bufferevent_free(event_reads_data);
                event_reads_data = nullptr;

                evbuffer_free(event_write_data);
                event_write_data = nullptr;
            }

            if (nullptr != event_discon)
            {
                event_discon();
            }

            if (nullptr != callback)
            {
                callback();
            }

            return true;
        }
        return false;
    }

    void TcpMessage::OnReadsNotify(struct bufferevent* bev)
    {
        assert(EventThreadId() == EventCurrentThreadId());

        unsigned char msg[1024];
        {
            if (nullptr != bev)
            {
                // 循环读取
                while (evbuffer_get_length(bufferevent_get_input(bev)) && [&] (const unsigned char* buffer, u96 size)
                       {
                           if (0 == size)
                           {
                               return false;
                           }

                           return event_reads_buff->Append(buffer, size);
                       }(msg, bufferevent_read(bev, msg, std::size(msg))));

                if (nullptr != event_messages)
                {
                    try
                    {
                        if (0 == event_messages(event_basic, event_reads_buff))
                        {
                            if (Close())
                            {
                                return;
                            }
                        }
                    }
                    catch (...)
                    {

                    }
                }
            }
        }
    }

    void TcpMessage::OnEventNotify(struct bufferevent* bev, short events)
    {
        /*
         *  BEV_EVENT_READING：  读取操作时发生某事件，具体是哪种事件请看其他标志。
         *  BEV_EVENT_WRITING：  写入操作时发生某事件，具体是哪种事件请看其他标志。
         *  BEV_EVENT_ERROR：    操作时发生错误。关于错误的更多信息，请调用EVUTIL_SOCKET_ERROR()，获取错误号。
         *  BEV_EVENT_TIMEOUT：  发生超时。
         *  BEV_EVENT_EOF：      遇到文件结束指示。
         *  BEV_EVENT_CONNECTED：请求的连接过程已经完成。
         */

        if (events & BEV_EVENT_ERROR)
        {
            EVENT_ERROR("got an error from bufferevent: %d -> %s", EVUTIL_SOCKET_ERROR(), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        }

        if (BEV_EVENT_ERROR | BEV_EVENT_READING & events || BEV_EVENT_EOF | BEV_EVENT_READING | BEV_EVENT_WRITING & events)
        {
            if (Close())
            {
                return;
            }

            EVENT_ERROR("an unpredictable error occurred when closing a socket handle");
        }
    }
}