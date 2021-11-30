#include <network/tcp/tcp_session.h>
#include <network/tcp/tcp_message.h>
#include <event_loop.h>
#include <event_timer.h>
namespace Evpp
{
    TcpSession::TcpSession(EventLoop* basic, evutil_socket_t fd, struct sockaddr* addr, u96 index, const InterfaceMessage& message_callback, const InterfaceDiscons& discons_callback) :
        EventLoopImpl(basic),
        event_basic(basic),
        event_descriptor(fd),
        event_index(index),
        event_message(message_callback),
        event_discons(discons_callback),
        event_address(),
        event_port(0),
        event_timer(std::make_shared<EventTimer>(basic)),
        tcp_message(std::make_shared<TcpMessage>(basic,
                                                 fd,
                                                 std::bind(&TcpSession::OnMessages, this, std::placeholders::_1, std::placeholders::_2),
                                                 std::bind(&TcpSession::OnDiscon, this)))
    {
        if (nullptr != addr)
        {
            char address[64] = {};
            {
                if (nullptr == evutil_inet_ntop(addr->sa_family,
                                                reinterpret_cast<const void*>(reinterpret_cast<char*>(addr) + (addr->sa_family == AF_INET ? offsetof(sockaddr_in, sin_addr) : addr->sa_family == AF_INET6 ? offsetof(sockaddr_in6, sin6_addr) : AF_UNSPEC)),
                                                address,
                                                std::size(address)))
                {
                    EVENT_ERROR("failed to initialize network address");
                }

                if (event_address.empty())
                {
                    event_address += address;
                    event_address += ':';
                    event_address += std::to_string(static_cast<u32>(htons(addr->sa_family == AF_INET ? reinterpret_cast<sockaddr_in*>(addr)->sin_port : addr->sa_family == AF_INET6 ? reinterpret_cast<sockaddr_in6*>(addr)->sin6_port : 0)));
                }
            }
        }
        else
        {
            EVENT_ERROR("struct sockaddr* addr pointer address is empty");
        }
    }

    TcpSession::~TcpSession()
    {

    }

    bool TcpSession::InitialSession()
    {
        if (nullptr != tcp_message)
        {
            return tcp_message->InitialMessage();
        }
        return false;
    }

    bool TcpSession::Send(const char* buffer, u96 size)
    {
        if (nullptr == tcp_message)
        {
            return false;
        }
        return tcp_message->Send(buffer, size);
    }

    bool TcpSession::Send(const std::string& buffer)
    {
        if (nullptr == tcp_message)
        {
            return false;
        }
        return tcp_message->Send(buffer);
    }

    bool TcpSession::Close(const Handler& callback)
    {
        if (nullptr == tcp_message)
        {
            return false;
        }
        return tcp_message->Close(callback);
    }

    const std::string& TcpSession::GetHost()
    {
        return event_address;
    }

    bool TcpSession::OnMessages(EventLoop* loop, const std::shared_ptr<EventBuffer>& buffer)
    {
        if (nullptr == event_message)
        {
            return true;
        }

        return event_message(loop, shared_from_this(), buffer, event_index);
    }

    void TcpSession::OnDiscon()
    {
        if (nullptr != event_discons)
        {
            event_discons(event_basic, shared_from_this(), event_index);
        }
    }
}