#ifndef __EVENT_BUFFER_H__
#define __EVENT_BUFFER_H__
#include <event_core.h>
namespace Evpp
{
    class EventBuffer
    {
    public:
        explicit EventBuffer(u96 prepend = 8, u96 lengths = 1024);
        virtual ~EventBuffer();
    public:
        // 可读字节
        i96 ReadableBytes() const;
        // 可写字节
        u96 WritableBytes() const;
        // 预留字节
        u96 PrependsBytes() const;
        // 当前读取
        u96 CurrentReader() const;
        // 当前写入
        u96 CurrentWriter() const;
        // 本次写入
        u96 CurrentInsert() const;
        // 实际写入
        u96 CurrentActual() const;
    public:
        // 查看数字
        i16 PeekInt16(u96 pos = 0) const;
        i32 PeekInt32(u96 pos = 0) const;
        i64 PeekInt64(u96 pos = 0) const;
        i96 PeekInt96(u96 pos = 0) const;
        // 读取数字
        i16 ReadInt16();
        i32 ReadInt32();
        i64 ReadInt64();
        i96 ReadInt96();
    public:
        // 读取文本
        std::string ReadString(u96 lengths);
        std::string ReadStringAll();
    public:
        // 搜索结尾 or 搜索特征 找到返回 true 并且 lengths 是数据长度
        bool SearchDelimiters(const std::string& delimiters, u96& lengths);
    public:
        bool Append(const std::string& data);
        bool Append(const char* data, u96 lengths);
        bool Append(const void* data, u96 lengths);
    public:
        // 跳过数据
        bool                    Skip(u96 lengths);
        // 跳过全部数据
        void                    SkipAll();
        // 查看数据
        const char*             Peek() const;
        const char*             Peek(u96 pos) const;
        // std::begin
        unsigned char*          Data();
        // std::begin
        const unsigned char*    Data() const;
    private:
        bool MakeMemorySpace(u96 lengths);
    private:
        std::vector<unsigned char>                                                              event_buffer;
    private:
        u96                                                                                     event_prepend;
        u96                                                                                     event_lengths;
    private:
        u96                                                                                     event_reader;
        u96                                                                                     event_writer;
        u96                                                                                     event_insert;
        u96                                                                                     event_actual;
    };
}
#endif // __EVENT_BUFFER_H__