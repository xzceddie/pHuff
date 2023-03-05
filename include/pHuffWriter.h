#include <cstdio>
#include <cstddef>
#include <cstdint>



struct HuffSeg
{
    std::uint8_t m_writeLen;
    std::uint64_t m_Coded;
};


struct BufferBasicInfo
{
    std::size_t m_buffLen;
    void* m_buffStartPtr;
};


class WriteBufferState
{
    void* m_currPtr;
    std::uint8_t m_currFilledLen;
    BufferBasicInfo m_buffInfo;
public:
    void* getCurWritePtr() const { return m_currPtr; }
    std::uint8_t getCurFilledLen() const { return m_currFilledLen; }
    BufferBasicInfo getBufferInfo() const { return m_buffInfo; }
};
