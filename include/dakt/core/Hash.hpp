#pragma once

// ============================================================================
// DaktLib Core - Hash
// Hash functions using C++23 features
// ============================================================================

#include "Macros.hpp"
#include "Types.hpp"

#include <concepts>
#include <cstring>
#include <functional>
#include <string_view>

namespace dakt::core
{

// ============================================================================
// Hash Concepts
// ============================================================================

template <typename T>
concept HashableType = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<usize>;
};

// ============================================================================
// FNV-1a Hash (Fast, good for hash tables)
// ============================================================================

namespace hash
{

// FNV-1a constants
inline constexpr u32 FNV32_PRIME = 0x01000193;
inline constexpr u32 FNV32_OFFSET = 0x811c9dc5;
inline constexpr u64 FNV64_PRIME = 0x00000100000001B3ull;
inline constexpr u64 FNV64_OFFSET = 0xcbf29ce484222325ull;

// FNV-1a 32-bit
[[nodiscard]] DAKT_FORCEINLINE constexpr u32 fnv1a32(const void* data, usize size) noexcept
{
    const auto* bytes = static_cast<const byte*>(data);
    u32 hash = FNV32_OFFSET;
    for (usize i = 0; i < size; ++i)
    {
        hash ^= std::to_integer<u32>(bytes[i]);
        hash *= FNV32_PRIME;
    }
    return hash;
}

[[nodiscard]] DAKT_FORCEINLINE constexpr u32 fnv1a32(StringView str) noexcept
{
    u32 hash = FNV32_OFFSET;
    for (char c : str)
    {
        hash ^= static_cast<u32>(static_cast<unsigned char>(c));
        hash *= FNV32_PRIME;
    }
    return hash;
}

// FNV-1a 64-bit
[[nodiscard]] DAKT_FORCEINLINE constexpr u64 fnv1a64(const void* data, usize size) noexcept
{
    const auto* bytes = static_cast<const byte*>(data);
    u64 hash = FNV64_OFFSET;
    for (usize i = 0; i < size; ++i)
    {
        hash ^= std::to_integer<u64>(bytes[i]);
        hash *= FNV64_PRIME;
    }
    return hash;
}

[[nodiscard]] DAKT_FORCEINLINE constexpr u64 fnv1a64(StringView str) noexcept
{
    u64 hash = FNV64_OFFSET;
    for (char c : str)
    {
        hash ^= static_cast<u64>(static_cast<unsigned char>(c));
        hash *= FNV64_PRIME;
    }
    return hash;
}

// ============================================================================
// CRC32 (Good for checksums, file integrity)
// ============================================================================

// Get CRC32 lookup table
[[nodiscard]] DAKT_API const u32* getCRC32Table();

// CRC32 calculation
[[nodiscard]] DAKT_API u32 crc32(const void* data, usize size);
[[nodiscard]] DAKT_API u32 crc32(ConstByteSpan data);
[[nodiscard]] DAKT_API u32 crc32(StringView str);

// Incremental CRC32
[[nodiscard]] DAKT_API u32 crc32Update(u32 crc, const void* data, usize size);
[[nodiscard]] DAKT_API u32 crc32Finalize(u32 crc);

// ============================================================================
// XXHash (Very fast, high quality)
// ============================================================================

// XXHash32
[[nodiscard]] DAKT_API u32 xxhash32(const void* data, usize size, u32 seed = 0);
[[nodiscard]] DAKT_API u32 xxhash32(ConstByteSpan data, u32 seed = 0);
[[nodiscard]] DAKT_API u32 xxhash32(StringView str, u32 seed = 0);

// XXHash64
[[nodiscard]] DAKT_API u64 xxhash64(const void* data, usize size, u64 seed = 0);
[[nodiscard]] DAKT_API u64 xxhash64(ConstByteSpan data, u64 seed = 0);
[[nodiscard]] DAKT_API u64 xxhash64(StringView str, u64 seed = 0);

// ============================================================================
// MurmurHash3 (Good for hash tables, widely used)
// ============================================================================

[[nodiscard]] DAKT_API u32 murmur3_32(const void* data, usize size, u32 seed = 0);
DAKT_API void murmur3_128(const void* data, usize size, u32 seed, u64 out[2]);

// ============================================================================
// Compile-time String Hash (for switch statements, etc.)
// ============================================================================

[[nodiscard]] consteval u32 constHash32(const char* str, usize len) noexcept
{
    u32 hash = FNV32_OFFSET;
    for (usize i = 0; i < len; ++i)
    {
        hash ^= static_cast<u32>(static_cast<unsigned char>(str[i]));
        hash *= FNV32_PRIME;
    }
    return hash;
}

[[nodiscard]] consteval u64 constHash64(const char* str, usize len) noexcept
{
    u64 hash = FNV64_OFFSET;
    for (usize i = 0; i < len; ++i)
    {
        hash ^= static_cast<u64>(static_cast<unsigned char>(str[i]));
        hash *= FNV64_PRIME;
    }
    return hash;
}

// String literal hash operator
namespace literals
{
[[nodiscard]] consteval u32 operator""_hash32(const char* str, usize len) noexcept
{
    return constHash32(str, len);
}

[[nodiscard]] consteval u64 operator""_hash64(const char* str, usize len) noexcept
{
    return constHash64(str, len);
}

// Default hash (64-bit)
[[nodiscard]] consteval u64 operator""_hash(const char* str, usize len) noexcept
{
    return constHash64(str, len);
}
}  // namespace literals

// ============================================================================
// Hash Combiner (for combining multiple values)
// ============================================================================

[[nodiscard]] DAKT_FORCEINLINE constexpr u64 hashCombine(u64 seed, u64 value) noexcept
{
    // Based on boost::hash_combine
    return seed ^ (value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2));
}

[[nodiscard]] DAKT_FORCEINLINE constexpr u32 hashCombine(u32 seed, u32 value) noexcept
{
    return seed ^ (value + 0x9e3779b9u + (seed << 6) + (seed >> 2));
}

// Variadic hash combine
template <typename... Args>
    requires(std::convertible_to<Args, u64> && ...)
[[nodiscard]] constexpr u64 combineHashes(Args... hashes) noexcept
{
    u64 result = 0;
    ((result = hashCombine(result, static_cast<u64>(hashes))), ...);
    return result;
}

// ============================================================================
// Type Hasher (for use with std containers)
// ============================================================================

template <typename T>
struct Hash
{
    [[nodiscard]] constexpr u64 operator()(const T& value) const noexcept
    {
        if constexpr (TriviallyCopyable<T>)
        {
            return fnv1a64(&value, sizeof(T));
        }
        else if constexpr (HashableType<T>)
        {
            return static_cast<u64>(std::hash<T>{}(value));
        }
        else
        {
            static_assert(sizeof(T) == 0, "Type is not hashable");
        }
    }
};

// Specializations
template <>
struct Hash<String>
{
    [[nodiscard]] constexpr u64 operator()(const String& value) const noexcept { return fnv1a64(StringView(value)); }
};

template <>
struct Hash<StringView>
{
    [[nodiscard]] constexpr u64 operator()(StringView value) const noexcept { return fnv1a64(value); }
};

template <>
struct Hash<const char*>
{
    [[nodiscard]] u64 operator()(const char* value) const noexcept { return fnv1a64(value, std::strlen(value)); }
};

}  // namespace hash

// Import hash literals into dakt::core namespace
using namespace hash::literals;

}  // namespace dakt::core