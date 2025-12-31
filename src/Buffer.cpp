// ============================================================================
// DaktLib Core - Buffer Implementation
// ============================================================================

#include <dakt/core/Buffer.hpp>

#include <cstring>
#include <stdexcept>

namespace dakt::core
{

// ============================================================================
// Buffer
// ============================================================================

Buffer::Buffer(usize initialCapacity)
{
    m_data.reserve(initialCapacity);
}

Buffer::Buffer(const void* data, usize size)
    : m_data(static_cast<const byte*>(data), static_cast<const byte*>(data) + size)
{
}

Buffer::Buffer(ConstByteSpan span)
    : m_data(span.begin(), span.end())
{
}

void Buffer::resize(usize newSize)
{
    m_data.resize(newSize);
}

void Buffer::resize(usize newSize, byte value)
{
    m_data.resize(newSize, value);
}

void Buffer::reserve(usize newCapacity)
{
    m_data.reserve(newCapacity);
}

void Buffer::shrinkToFit()
{
    m_data.shrink_to_fit();
}

void Buffer::clear()
{
    m_data.clear();
}

void Buffer::assign(const void* data, usize size)
{
    m_data.assign(static_cast<const byte*>(data), static_cast<const byte*>(data) + size);
}

void Buffer::assign(ConstByteSpan span)
{
    m_data.assign(span.begin(), span.end());
}

void Buffer::append(const void* data, usize size)
{
    auto ptr = static_cast<const byte*>(data);
    m_data.insert(m_data.end(), ptr, ptr + size);
}

void Buffer::append(ConstByteSpan span)
{
    m_data.insert(m_data.end(), span.begin(), span.end());
}

void Buffer::append(byte value)
{
    m_data.push_back(value);
}

void Buffer::append(byte value, usize count)
{
    m_data.insert(m_data.end(), count, value);
}

void Buffer::insert(usize pos, const void* data, usize size)
{
    auto ptr = static_cast<const byte*>(data);
    m_data.insert(m_data.begin() + static_cast<isize>(pos), ptr, ptr + size);
}

void Buffer::insert(usize pos, ConstByteSpan span)
{
    m_data.insert(m_data.begin() + static_cast<isize>(pos), span.begin(), span.end());
}

void Buffer::erase(usize pos, usize count)
{
    auto start = m_data.begin() + static_cast<isize>(pos);
    m_data.erase(start, start + static_cast<isize>(count));
}

void Buffer::fill(byte value)
{
    std::fill(m_data.begin(), m_data.end(), value);
}

void Buffer::fill(byte value, usize start, usize count)
{
    std::fill_n(m_data.begin() + static_cast<isize>(start), count, value);
}

void Buffer::zero()
{
    fill(byte{0});
}

ConstByteSpan Buffer::subspan(usize offset, usize count) const
{
    return ConstByteSpan(m_data).subspan(offset, count);
}

ByteSpan Buffer::subspan(usize offset, usize count)
{
    return ByteSpan(m_data).subspan(offset, count);
}

// ============================================================================
// BufferReader
// ============================================================================

BufferReader::BufferReader(ConstByteSpan data)
    : m_data(data), m_pos(0)
{
}

BufferReader::BufferReader(const void* data, usize size)
    : m_data(static_cast<const byte*>(data), size), m_pos(0)
{
}

void BufferReader::seek(usize pos)
{
    m_pos = std::min(pos, m_data.size());
}

void BufferReader::skip(usize count)
{
    m_pos = std::min(m_pos + count, m_data.size());
}

bool BufferReader::read(void* dest, usize size)
{
    if (remaining() < size)
        return false;
    
    std::memcpy(dest, m_data.data() + m_pos, size);
    m_pos += size;
    return true;
}

ConstByteSpan BufferReader::readSpan(usize size)
{
    if (remaining() < size)
        return {};
    
    auto result = m_data.subspan(m_pos, size);
    m_pos += size;
    return result;
}

Option<String> BufferReader::readString(usize length)
{
    if (remaining() < length)
        return none;
    
    String result(reinterpret_cast<const char*>(m_data.data() + m_pos), length);
    m_pos += length;
    return some(std::move(result));
}

Option<String> BufferReader::readNullTerminatedString()
{
    auto start = m_pos;
    while (m_pos < m_data.size() && m_data[m_pos] != byte{0})
    {
        ++m_pos;
    }
    
    if (m_pos >= m_data.size())
    {
        m_pos = start;
        return none;
    }
    
    String result(reinterpret_cast<const char*>(m_data.data() + start), m_pos - start);
    ++m_pos;  // Skip null terminator
    return some(std::move(result));
}

Option<String> BufferReader::readNullTerminatedString(usize maxLength)
{
    auto start = m_pos;
    auto limit = std::min(m_pos + maxLength, m_data.size());
    
    while (m_pos < limit && m_data[m_pos] != byte{0})
    {
        ++m_pos;
    }
    
    String result(reinterpret_cast<const char*>(m_data.data() + start), m_pos - start);
    
    // Skip null terminator if present
    if (m_pos < m_data.size() && m_data[m_pos] == byte{0})
    {
        ++m_pos;
    }
    
    return some(std::move(result));
}

Option<String> BufferReader::readLengthPrefixedString()
{
    auto len = read<u32>();
    if (!len)
        return none;
    
    return readString(*len);
}

Option<Buffer> BufferReader::readBuffer(usize size)
{
    if (remaining() < size)
        return none;
    
    Buffer result(m_data.data() + m_pos, size);
    m_pos += size;
    return some(std::move(result));
}

// ============================================================================
// BufferWriter
// ============================================================================

BufferWriter::BufferWriter()
    : m_buffer(&m_ownedBuffer), m_pos(0)
{
}

BufferWriter::BufferWriter(usize initialCapacity)
    : m_ownedBuffer(initialCapacity), m_buffer(&m_ownedBuffer), m_pos(0)
{
}

BufferWriter::BufferWriter(Buffer& buffer)
    : m_buffer(&buffer), m_pos(buffer.size())
{
}

usize BufferWriter::size() const noexcept
{
    return m_buffer->size();
}

void BufferWriter::seek(usize pos)
{
    m_pos = pos;
}

void BufferWriter::skip(usize count)
{
    m_pos += count;
    ensureCapacity(m_pos);
    updateSize();
}

void BufferWriter::write(const void* data, usize size)
{
    ensureCapacity(m_pos + size);
    std::memcpy(dataPtr() + m_pos, data, size);
    m_pos += size;
    updateSize();
}

void BufferWriter::write(ConstByteSpan span)
{
    write(span.data(), span.size());
}

void BufferWriter::writeString(StringView str)
{
    write(str.data(), str.size());
}

void BufferWriter::writeNullTerminatedString(StringView str)
{
    write(str.data(), str.size());
    write<byte>(byte{0});
}

void BufferWriter::writeLengthPrefixedString(StringView str)
{
    write<u32>(static_cast<u32>(str.size()));
    write(str.data(), str.size());
}

void BufferWriter::writePadding(usize count, byte value)
{
    ensureCapacity(m_pos + count);
    std::fill_n(dataPtr() + m_pos, count, value);
    m_pos += count;
    updateSize();
}

void BufferWriter::align(usize alignment, byte padValue)
{
    auto remainder = m_pos % alignment;
    if (remainder != 0)
    {
        writePadding(alignment - remainder, padValue);
    }
}

Buffer BufferWriter::toBuffer()
{
    if (m_buffer == &m_ownedBuffer)
    {
        return std::move(m_ownedBuffer);
    }
    else
    {
        return Buffer(m_buffer->span());
    }
}

ConstByteSpan BufferWriter::span() const
{
    return m_buffer->span();
}

void BufferWriter::ensureCapacity(usize minCapacity)
{
    if (m_buffer->capacity() < minCapacity)
    {
        m_buffer->reserve(std::max(minCapacity, m_buffer->capacity() * 2));
    }
}

void BufferWriter::updateSize()
{
    if (m_pos > m_buffer->size())
    {
        m_buffer->resize(m_pos);
    }
}

byte* BufferWriter::dataPtr()
{
    return m_buffer->data();
}

}  // namespace dakt::core
