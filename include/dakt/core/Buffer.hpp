#pragma once

// ============================================================================
// DaktLib Core - Buffer
// Binary buffer and byte stream utilities using C++23 features
// ============================================================================

#include "Macros.hpp"
#include "Memory.hpp"
#include "Platform.hpp"
#include "Types.hpp"

#include <concepts>
#include <cstring>
#include <vector>

namespace dakt::core
{

// ============================================================================
// Buffer - Dynamic byte buffer
// ============================================================================

class Buffer
{
public:
    Buffer() = default;
    explicit Buffer(usize initialCapacity);
    Buffer(const void* data, usize size);
    Buffer(ConstByteSpan span);

    Buffer(const Buffer& other) = default;
    Buffer(Buffer&& other) noexcept = default;
    Buffer& operator=(const Buffer& other) = default;
    Buffer& operator=(Buffer&& other) noexcept = default;

    ~Buffer() = default;

    // Data access
    [[nodiscard]] byte* data() noexcept { return m_data.data(); }
    [[nodiscard]] const byte* data() const noexcept { return m_data.data(); }

    [[nodiscard]] byte& operator[](usize index) { return m_data[index]; }
    [[nodiscard]] const byte& operator[](usize index) const { return m_data[index]; }

    [[nodiscard]] ByteSpan span() noexcept { return ByteSpan(m_data); }
    [[nodiscard]] ConstByteSpan span() const noexcept { return ConstByteSpan(m_data); }

    // Implicit conversion to span
    [[nodiscard]] operator ByteSpan() noexcept { return span(); }
    [[nodiscard]] operator ConstByteSpan() const noexcept { return span(); }

    // Size operations
    [[nodiscard]] usize size() const noexcept { return m_data.size(); }
    [[nodiscard]] usize capacity() const noexcept { return m_data.capacity(); }
    [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }

    void resize(usize newSize);
    void resize(usize newSize, byte value);
    void reserve(usize newCapacity);
    void shrinkToFit();
    void clear();

    // Content modification
    void assign(const void* data, usize size);
    void assign(ConstByteSpan span);

    void append(const void* data, usize size);
    void append(ConstByteSpan span);
    void append(byte value);
    void append(byte value, usize count);

    void insert(usize pos, const void* data, usize size);
    void insert(usize pos, ConstByteSpan span);

    void erase(usize pos, usize count);

    // Fill
    void fill(byte value);
    void fill(byte value, usize start, usize count);
    void zero();

    // Subview
    [[nodiscard]] ConstByteSpan subspan(usize offset, usize count) const;
    [[nodiscard]] ByteSpan subspan(usize offset, usize count);

    // Iterators
    [[nodiscard]] auto begin() noexcept { return m_data.begin(); }
    [[nodiscard]] auto end() noexcept { return m_data.end(); }
    [[nodiscard]] auto begin() const noexcept { return m_data.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_data.end(); }
    [[nodiscard]] auto cbegin() const noexcept { return m_data.cbegin(); }
    [[nodiscard]] auto cend() const noexcept { return m_data.cend(); }

private:
    std::vector<byte> m_data;
};

// ============================================================================
// BufferReader - Read data from a buffer
// ============================================================================

class BufferReader
{
public:
    explicit BufferReader(ConstByteSpan data);
    BufferReader(const void* data, usize size);

    // Position
    [[nodiscard]] usize position() const noexcept { return m_pos; }
    [[nodiscard]] usize size() const noexcept { return m_data.size(); }
    [[nodiscard]] usize remaining() const noexcept { return m_data.size() - m_pos; }
    [[nodiscard]] bool eof() const noexcept { return m_pos >= m_data.size(); }

    void seek(usize pos);
    void skip(usize count);
    void rewind() { m_pos = 0; }

    // Read raw bytes
    bool read(void* dest, usize size);
    [[nodiscard]] ConstByteSpan readSpan(usize size);

    // Read primitives (little endian by default)
    template <TriviallyCopyable T>
    [[nodiscard]] Option<T> read()
    {
        if (remaining() < sizeof(T))
            return none;

        T value;
        std::memcpy(&value, m_data.data() + m_pos, sizeof(T));
        m_pos += sizeof(T);
        return some(value);
    }

    // Read with explicit endianness
    template <Integral T>
    [[nodiscard]] Option<T> readLE()
    {
        if (auto result = read<T>())
        {
            return some(fromLittleEndian(*result));
        }
        return none;
    }

    template <Integral T>
    [[nodiscard]] Option<T> readBE()
    {
        if (auto result = read<T>())
        {
            return some(fromBigEndian(*result));
        }
        return none;
    }

    // Convenience read methods
    [[nodiscard]] Option<i8> readI8() { return read<i8>(); }
    [[nodiscard]] Option<u8> readU8() { return read<u8>(); }
    [[nodiscard]] Option<i16> readI16() { return readLE<i16>(); }
    [[nodiscard]] Option<u16> readU16() { return readLE<u16>(); }
    [[nodiscard]] Option<i32> readI32() { return readLE<i32>(); }
    [[nodiscard]] Option<u32> readU32() { return readLE<u32>(); }
    [[nodiscard]] Option<i64> readI64() { return readLE<i64>(); }
    [[nodiscard]] Option<u64> readU64() { return readLE<u64>(); }
    [[nodiscard]] Option<f32> readF32() { return read<f32>(); }
    [[nodiscard]] Option<f64> readF64() { return read<f64>(); }

    [[nodiscard]] Option<i16> readI16BE() { return readBE<i16>(); }
    [[nodiscard]] Option<u16> readU16BE() { return readBE<u16>(); }
    [[nodiscard]] Option<i32> readI32BE() { return readBE<i32>(); }
    [[nodiscard]] Option<u32> readU32BE() { return readBE<u32>(); }
    [[nodiscard]] Option<i64> readI64BE() { return readBE<i64>(); }
    [[nodiscard]] Option<u64> readU64BE() { return readBE<u64>(); }

    // Read strings
    [[nodiscard]] Option<String> readString(usize length);
    [[nodiscard]] Option<String> readNullTerminatedString();
    [[nodiscard]] Option<String> readNullTerminatedString(usize maxLength);
    [[nodiscard]] Option<String> readLengthPrefixedString();  // u32 length prefix

    // Read into buffer
    [[nodiscard]] Option<Buffer> readBuffer(usize size);

    // Peek without advancing position
    template <TriviallyCopyable T>
    [[nodiscard]] Option<T> peek() const
    {
        if (remaining() < sizeof(T))
            return none;
        T value;
        std::memcpy(&value, m_data.data() + m_pos, sizeof(T));
        return some(value);
    }

    [[nodiscard]] Option<byte> peekByte() const { return peek<byte>(); }

    // Get remaining data as span
    [[nodiscard]] ConstByteSpan remainingSpan() const { return m_data.subspan(m_pos); }

private:
    ConstByteSpan m_data;
    usize m_pos = 0;
};

// ============================================================================
// BufferWriter - Write data to a buffer
// ============================================================================

class BufferWriter
{
public:
    BufferWriter();
    explicit BufferWriter(usize initialCapacity);
    explicit BufferWriter(Buffer& buffer);  // Write to existing buffer

    // Position
    [[nodiscard]] usize position() const noexcept { return m_pos; }
    [[nodiscard]] usize size() const noexcept;

    void seek(usize pos);
    void skip(usize count);
    void rewind() { m_pos = 0; }

    // Write raw bytes
    void write(const void* data, usize size);
    void write(ConstByteSpan span);

    // Write primitives (little endian by default)
    template <TriviallyCopyable T>
    void write(T value)
    {
        ensureCapacity(m_pos + sizeof(T));
        std::memcpy(dataPtr() + m_pos, &value, sizeof(T));
        m_pos += sizeof(T);
        updateSize();
    }

    // Write with explicit endianness
    template <Integral T>
    void writeLE(T value)
    {
        write(toLittleEndian(value));
    }

    template <Integral T>
    void writeBE(T value)
    {
        write(toBigEndian(value));
    }

    // Convenience write methods
    void writeI8(i8 value) { write(value); }
    void writeU8(u8 value) { write(value); }
    void writeI16(i16 value) { writeLE(value); }
    void writeU16(u16 value) { writeLE(value); }
    void writeI32(i32 value) { writeLE(value); }
    void writeU32(u32 value) { writeLE(value); }
    void writeI64(i64 value) { writeLE(value); }
    void writeU64(u64 value) { writeLE(value); }
    void writeF32(f32 value) { write(value); }
    void writeF64(f64 value) { write(value); }

    void writeI16BE(i16 value) { writeBE(value); }
    void writeU16BE(u16 value) { writeBE(value); }
    void writeI32BE(i32 value) { writeBE(value); }
    void writeU32BE(u32 value) { writeBE(value); }
    void writeI64BE(i64 value) { writeBE(value); }
    void writeU64BE(u64 value) { writeBE(value); }

    // Write strings
    void writeString(StringView str);
    void writeNullTerminatedString(StringView str);
    void writeLengthPrefixedString(StringView str);  // u32 length prefix

    // Write padding/fill
    void writePadding(usize count, byte value = byte{0});
    void writeZeros(usize count) { writePadding(count, byte{0}); }

    // Align position to boundary
    void align(usize alignment, byte padValue = byte{0});

    // Get result
    [[nodiscard]] Buffer toBuffer();
    [[nodiscard]] ConstByteSpan span() const;

private:
    void ensureCapacity(usize minCapacity);
    void updateSize();
    [[nodiscard]] byte* dataPtr();

    Buffer m_ownedBuffer;
    Buffer* m_buffer = nullptr;
    usize m_pos = 0;
};

}  // namespace dakt::core