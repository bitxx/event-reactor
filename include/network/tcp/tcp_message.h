#ifndef __EVENT_NEWS_H__
#define __EVENT_NEWS_H__
#include <event_core.h>
#include <memory>
namespace Evpp
{
    class EventLoop;
    class EventForward;
    class EventBuffer;
    class TcpMessage : public EventLoopImpl
    {
    public:
        explicit TcpMessage(EventLoop* basic,
                            evutil_socket_t fd,
                            const Details::InterfaceMessages& messages,
                            const Details::InterfaceDiscon& discon);
        virtual ~TcpMessage();
    public:
        bool InitialMessage();
    public:
        bool Send(const char* buffer, u96 size);
        bool Send(const std::string& buffer);
    private:
        bool DoSend(const std::shared_ptr<EventBuffer>& buffer, u96 len);
        bool DoSend(const char* buffer, u96 len);
    public:
        bool Close(const Handler& callback = Handler());
    private:
        void OnReadsNotify(struct bufferevent *bev);
        void OnEventNotify(struct bufferevent *bev, short events);
    private:
        EventLoop*                                                                              event_basic;
        evutil_socket_t                                                                         event_descriptor;
    private:
        Details::InterfaceMessages                                                              event_messages;
        Details::InterfaceDiscon                                                                event_discon;
        struct bufferevent*                                                                     event_reads_data;
        struct evbuffer *                                                                       event_write_data;
    private:
        std::shared_ptr<EventBuffer>                                                            event_reads_buff;
        std::shared_ptr<EventBuffer>                                                            event_write_buff;
        std::mutex                                                                              event_write_lock;
    };
}
#endif // __EVENT_NEWS_H__