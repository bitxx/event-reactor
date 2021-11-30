#include <event_buffer.h>
#include <event_socket.h>
namespace Evpp
{
    /// @code
    /// +-------------------+------------------+------------------+
    /// | prependable bytes |  readable bytes  |  writable bytes  |
    /// |                   |     (CONTENT)    |                  |
    /// +-------------------+------------------+------------------+
    /// |                   |                  |                  |
    /// 0      <=      readerIndex   <=   writerIndex    <=    size
    /// @endcode

    EventBuffer::EventBuffer(u96 prepend, u96 lengths) :
        event_buffer(prepend + lengths),
        event_prepend(prepend),
        event_lengths(lengths),
        event_reader(prepend),
        event_writer(prepend),
        event_insert(0),
        event_actual(0)
    {

    }

    EventBuffer::~EventBuffer()
    {

    }

    i96 EventBuffer::ReadableBytes() const
    {
        return static_cast<i96>(event_writer - event_reader);
    }

    u96 EventBuffer::WritableBytes() const
    {
        return event_buffer.size() - event_writer;
    }

    u96 EventBuffer::PrependsBytes() const
    {
        return event_prepend;
    }

    u96 EventBuffer::CurrentReader() const
    {
        return event_reader;
    }

    u96 EventBuffer::CurrentWriter() const
    {
        return event_writer;
    }

    u96 EventBuffer::CurrentInsert() const
    {
        return event_insert;
    }

    u96 EventBuffer::CurrentActual() const
    {
        return event_actual;
    }

    i16 EventBuffer::PeekInt16(u96 pos) const
    {
        if (sizeof(i16) <= ReadableBytes() || pos)
        {
            return __bswap_16__(*reinterpret_cast<const i16*>(Peek(pos)));
        }
        return -1;
    }

    i32 EventBuffer::PeekInt32(u96 pos) const
    {
        if (sizeof(i32) <= ReadableBytes() || pos)
        {
            return __bswap_32__ (*reinterpret_cast<const i32*>(Peek(pos)));
        }
        return -1;
    }

    i64 EventBuffer::PeekInt64(u96 pos) const
    {
        if (sizeof(i64) <= ReadableBytes() || pos)
        {
            return __bswap_64__ (*reinterpret_cast<const i64*>(Peek(pos)));
        }
        return -1;
    }

    i96 EventBuffer::PeekInt96(u96 pos) const
    {
        if (sizeof(i96) <= ReadableBytes() || pos)
        {
            return __bswap_96__ (*reinterpret_cast<const i96*>(Peek(pos)));
        }
        return -1;
    }

    i16 EventBuffer::ReadInt16()
    {
        if (sizeof(i16) <= ReadableBytes())
        {
            if (Skip(sizeof(i16)))
            {
                return PeekInt16(sizeof(i16));
            }
        }
        return -1;
    }

    i32 EventBuffer::ReadInt32()
    {
        if (sizeof(i32) <= ReadableBytes())
        {
            if (Skip(sizeof(i32)))
            {
                return PeekInt32(sizeof(i32));
            }
        }
        return -1;
    }

    i64 EventBuffer::ReadInt64()
    {
        if (sizeof(i64) <= ReadableBytes())
        {
            if (Skip(sizeof(i64)))
            {
                return PeekInt64(sizeof(i64));
            }
        }
        return -1;
    }

    i96 EventBuffer::ReadInt96()
    {
        if (sizeof(i96) <= ReadableBytes())
        {
            if (Skip(sizeof(i96)))
            {
                return PeekInt64(sizeof(i96));
            }
        }
        return -1;
    }

    std::string EventBuffer::ReadString(u96 lengths)
    {
        if (lengths <= static_cast<u96>(ReadableBytes()))
        {
            if (Skip(lengths))
            {
                return std::string(reinterpret_cast<const char*>(Peek()), lengths);
            }
        }
        return std::string();
    }

    std::string EventBuffer::ReadStringAll()
    {
        return ReadString(ReadableBytes());
    }

    bool EventBuffer::SearchDelimiters(const std::string& delimiters, u96& lengths)
    {
        if (delimiters.empty())
        {
            return false;
        }
        // 开始查找一个分隔符，直到末端，统计出该段数据的长度。
        lengths = [&](const unsigned char* search_point)
        {
            return [&](const unsigned char* writer_point)
            {
                if (search_point == writer_point)
                {
                    return static_cast<u96>(0);
                }

                if (static_cast<u96>(writer_point - PrependsBytes() - search_point) == delimiters.size() ||
                    static_cast<u96>(search_point - reinterpret_cast<const unsigned char*>(Peek()) + writer_point - search_point) == event_writer
                    )
                {
                    return static_cast<u96>(search_point - reinterpret_cast<const unsigned char*>(Peek()) + delimiters.size());
                }

                return static_cast<u96>(0);
            }(Data() + event_writer + PrependsBytes());
        }(std::search(reinterpret_cast<const unsigned char*>(Peek()), reinterpret_cast<const unsigned char*>(Peek()) + event_writer, delimiters.begin(), delimiters.end()));
        return 0 != lengths && event_writer >= lengths;
    }

    bool EventBuffer::Append(const std::string& data)
    {
        if (data.empty())
        {
            return false;
        }

        return Append(data.c_str(), data.size());
    }

    bool EventBuffer::Append(const char* data, u96 lengths)
    {
        if (nullptr == data || 0 == lengths)
        {
            return false;
        }

        return Append(static_cast<const void*>(data), lengths);
    }

    bool EventBuffer::Append(const void* data, u96 lengths)
    {
        if (nullptr != data || 0 != lengths)
        {
            if (lengths > WritableBytes() && MakeMemorySpace(lengths))
            {
                EVENT_TRACE("开辟内存空间: %zu Bytes", lengths);
            }

            if ([&](unsigned char* dest) -> bool
                {
                    if (lengths + dest == std::copy(reinterpret_cast<const unsigned char*>(data),
                                                    reinterpret_cast<const unsigned char*>(data) + lengths,
                                                    dest))
                    {
                        event_writer += lengths;
                        event_actual += lengths;
                        event_insert = lengths;
                    }
                    else
                    {
                        assert(0);
                    }
                    return true;
                }(Data() + event_writer))
            {
                return true;
            }
        }

        return false;
    }

    const char* EventBuffer::Peek() const
    {
        return reinterpret_cast<const char*>(Data() + event_reader);
    }

    const char* EventBuffer::Peek(u96 pos) const
    {
        if (event_reader >= event_prepend)
        {
            return reinterpret_cast<const char*>(Data() + event_reader + pos);
        }
        return reinterpret_cast<const char*>(Data() + event_reader - pos);
    }

    bool EventBuffer::Skip(u96 lengths)
    {
        return [&, this](u96 readbytes) -> bool
        {
            if (lengths > readbytes)
            {
                return false;
            }
            // 当前数据长度 < 可写长度 - 可读长度
            // 64         <   1024  -    8
            if (lengths < readbytes)
            {
                event_reader += lengths;
                return true;
            }
            // 当前数据长度 == 可写长度 - 可读长度
            // 1016       ==   1024  -   8
            if (lengths == readbytes)
            {
                event_reader = event_prepend;
                event_writer = event_prepend;
                event_actual = 0;
                return true;
            }
            return false;
        }(ReadableBytes());
    }

    void EventBuffer::SkipAll()
    {
        event_reader = event_prepend;
        event_writer = event_prepend;
        event_actual = 0;
    };

    unsigned char* EventBuffer::Data()
    {
        return event_buffer.data();
    }

    const unsigned char* EventBuffer::Data() const
    {
        return event_buffer.data();
    }

    bool EventBuffer::MakeMemorySpace(u96 lengths)
    {
        try
        {
            if (WritableBytes() + PrependsBytes() < lengths + event_prepend)
            {
                event_buffer.resize(event_writer + lengths);
            }
            else
            {
                assert(event_prepend < event_reader);

                std::copy(event_buffer.begin() + event_reader,
                          event_buffer.begin() + event_writer,
                          event_buffer.begin() + event_prepend);

                event_reader = event_prepend;
                event_writer = event_reader + event_writer + event_reader;
                event_actual = event_writer - event_prepend;
            }
            return true;
        }
        catch (const std::system_error& ex)
        {
            EVENT_ERROR(ex.what());
        }
        return false;
    }
}