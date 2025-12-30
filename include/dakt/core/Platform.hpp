#pragma once

// ============================================================================
// Daktlib Core - Platform
// Platform detection and OS abstractionusing C++23 features
// ============================================================================

#include "Macros.hpp"
#include "Types.hpp"

#include <bit>
#include <string>
#include <string_view>

// ============================================================================
// Platform Detection (should be set by CMake, but fallback here)
// ============================================================================

#if !defined(DAKT_PLATFORM_WINDOWS) && !defined(DAKT_PLATFORM_LINUX) && !defined(DAKT_PLATFORM_MACOS)
    #if defined(_WIN32) || defined(_WIN64)
        #define DAKT_PLATFORM_WINDOWS
    #elif defined(__linux__)
        #define DAKT_PLATFORM_LINUX
    #elif defined(__APPLE__) || defined(__MACH__)
        #define DAKT_PLATFORM_MACOS
    #else
        #error "Unsupported platform"
    #endif
#endif

// ============================================================================
// Platform-specific Includes
// ============================================================================

#if defined(DAKT_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

namespace dakt::core
{

// ============================================================================
// Platform Identification
// ============================================================================

enum class Platform : u8
{
    Windows,
    Linux,
    MacOS,
    Unknown
};

enum class Architecture : u8
{
    X86,
    X64,
    ARM,
    ARM64,
    Unknown
};

// ============================================================================
// Platform Information (Compile-time) using C++20 std::endian
// ============================================================================

#if defined(DAKT_PLATFORM_WINDOWS)
inline constexpr Platform CurrentPlatform = Platform::Windows;
inline constexpr const char* PlatformName = "Windows";
inline constexpr const char* PathSeparator = "\\";
inline constexpr const char PathSeparatorChar = '\\';
inline constexpr const char* LineEnding = "\r\n";
#elif defined(DAKT_PLATFORM_LINUX)
inline constexpr Platform CurrentPlatform = Platform::Linux;
inline constexpr const char* PlatformName = "Linux";
inline constexpr const char* PathSeparator = "/";
inline constexpr char PathSeparatorChar = '/';
inline constexpr const char* LineEnding = "\n";
#elif defined(DAKT_PLATFORM_MACOS)
inline constexpr Platform CurrentPlatform = Platform::MacOS;
inline constexpr const char* PlatformName = "macOS";
inline constexpr const char* PathSeparator = "/";
inline constexpr char PathSeparatorChar = '/';
inline constexpr const char* LineEnding = "\n";
#else
inline constexpr Platform CurrentPlatform = Platform::Unknown;
inline constexpr const char* PlatformName = "Unknown";
inline constexpr const char* PathSeparator = "/";
inline constexpr char PathSeparatorChar = '/';
inline constexpr const char* LineEnding = "\n";
#endif

#if defined(DAKT_ARCH_X64)
inline constexpr Architecture CurrentArchitecture = Architecture::X64;
inline constexpr const char* ArchitectureName = "x64";
#elif defined(DAKT_ARCH_X86)
inline constexpr Architecture CurrentArchitecture = Architecture::X86;
inline constexpr const char* ArchitectureName = "x86";
#elif defined(DAKT_ARCH_ARM64)
inline constexpr Architecture CurrentArchitecture = Architecture::ARM64;
inline constexpr const char* ArchitectureName = "ARM64";
#elif defined(DAKT_ARCH_ARM)
inline constexpr Architecture CurrentArchitecture = Architecture::ARM;
inline constexpr const char* ArchitectureName = "ARM";
#else
inline constexpr Architecture CurrentArchitecure = Architecture::Unknown;
inline constexpr const char* ArchitecureName = "Unknown";
#endif

#if defined(DAKT_ARCH_64BIT)
inline constexpr bool Is64Bit = true;
inline constexpr bool Is32Bit = false;
#else
inline constexpr bool Is64Bit = false;
inline constexpr bool Is32Bit = true;
#endif

// C++20 std::endian
inline constexpr std::endian NativeEndian = std::endian::native;
inline constexpr bool IsLittleEndian = (std::endian::native == std::endian::little);
inline constexpr bool IsBigEndian = (std::endian::native == std::endian::big);

// ============================================================================
// System Infromation (Runtime)
// ============================================================================

struct SystemInfo
{
    u32 processorCount;
    u64 pageSize;
    u64 totalPhysicalMemory;
    u64 availablePhysicalMemory;
    String computerName;
    String userName;
    String osVersion;
};

// Get system information (implemented per-platform)
DAKT_API SystemInfo getSystemInfo();

// ============================================================================
// Environment
// ============================================================================

// Get environment variable
DAKT_API Option<String> getEnv(StringView name);

// Set environment variable
DAKT_API bool setEnv(StringView name, StringView value);

// Remove environment variable
DAKT_API bool unsetEnv(StringView name);

// ============================================================================
// Paths
// ============================================================================

// Get current working directory
DAKT_API String getCurrentDirectory();

// Set current working directory
DAKT_API bool setCurrentDirectory(StringView path);

// Get executable path
DAKT_API String getExecutablePath();

// Get executable directory
DAKT_API String getExecutableDirectory();

// Get user home directory
DAKT_API String getHomeDirectory();

// Get temp directory
DAKT_API String getTempDirectory();

// Get app data directory (per-user application data)
DAKT_API String getAppDataDirectory();

// ============================================================================
// Process
// ============================================================================

// Get current process ID
DAKT_API u32 getProcessId();

// Get current thread ID
DAKT_API u32 getThreadId();

// Sleep for specified milliseconds
DAKT_API void sleepMs(u32 milliseconds);

// Sleep for specified microseconds
DAKT_API void sleepUs(u32 microseconds);

// Yield to other threads
DAKT_API void yield();

// ============================================================================
// Console
// ============================================================================

enum class ConsoleColor : u8
{
    Black = 0,
    DarkBlue,
    DarkGreen,
    DarkCyan,
    DarkRed,
    DarkMagenta,
    DarkYellow,
    Gary,
    DarkGray,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Yellow,
    White,
    Default = 255
};

// Set console text color
DAKT_API void setConsoleColor(ConsoleColor foreground, ConsoleColor background = ConsoleColor::Default);

// Reset console to default colors
DAKT_API void resetConsoleColor();

// Enable ANSI escape codes (Windows 10+)
DAKT_API bool enableAnsiEscapeCodes();

// ============================================================================
// Memory
// ============================================================================

// Virtual memory allocation
DAKT_API void* virtualAlloc(usize size);
DAKT_API void virtualFree(void* ptr, usize Size);

// Memory protection flags
enum class MemoryProtection : u32
{
    NoAccess = 0,
    Read = 1,
    Write = 2,
    Execute = 4,
    ReadWrite = Read | Write,
    ReadExecute = Read | Execute,
    ReadWriteExecute = Read | Write | Execute
};

DAKT_API bool virtualProtect(void* ptr, usize size, MemoryProtection protection);

// ============================================================================
// Dynamic Library
// ============================================================================

using LibraryHandle = void*;

DAKT_API LibraryHandle loadLibrary(StringView path);
DAKT_API void freeLibrary(LibraryHandle handle);
DAKT_API void* getLibrarySymbol(LibraryHandle handle, StringView name);

template <typename T>
T getLibraryFunction(LibraryHandle handle, StringView name)
{
    return reinterpret_cast<T>(getLibrarySymbol(handle, name));
}

// ============================================================================
// Byte Swapping (C++23 std::byteswap)
// ============================================================================

template <Integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T byteSwap(T value) noexcept
{
    return std::byteswap(value);
}

// ============================================================================
// Endianness Conversion (using C++23)
// ============================================================================

template <Integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T toLittleEndian(T value) noexcept
{
    if constexpr (std::endian::native == std::endian::big)
    {
        return std::byteswap(value);
    }
    return value;
}

template <Integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T toBigEndian(T value) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return std::byteswap(value);
    }
    return value;
}

template <Integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T fromLittleEndian(T value) noexcept
{
    return toLittleEndian(value);  // Same operation
}

template <Integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T fromBigEndian(T value) noexcept
{
    return toBigEndian(value);  // Same operation
}

// ============================================================================
// Bit Operations (C++20 <bit>)
// ============================================================================

// Count leading zeros
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr int countLeadingZeros(T value) noexcept
{
    return std::countl_zero(value);
}

// Count trailing zeros
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr int countTrailingZeros(T value) noexcept
{
    return std::countr_zero(value);
}

// Count set bits (popcount)
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr int popCount(T value) noexcept
{
    return std::popcount(value);
}

// Check if power of two
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr bool isPowerOfTwo(T value) noexcept
{
    return std::has_single_bit(value);
}

// Round up to next power of two
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T nextPowerOfTwo(T value) noexcept
{
    return std::bit_ceil(value);
}

// Round down to previous power of two
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T prevPowerOfTwo(T value) noexcept
{
    return std::bit_floor(value);
}

// Bit width (number of bits needed to represent value)
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr int bitWidth(T value) noexcept
{
    return std::bit_width(value);
}

// Rotate left
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T rotateLeft(T value, int count) noexcept
{
    return std::rotl(value, count);
}

// Rotate right
template <std::unsigned_integral T>
[[nodiscard]] DAKT_FORCEINLINE constexpr T rotateRight(T value, int count) noexcept
{
    return std::rotr(value, count);
}

// Bit cast (C++ 20)
template <typename To, typename From>
    requires(sizeof(To) == sizeof(From)) && TriviallyCopyable<To> && TriviallyCopyable<From>
[[nodiscard]] DAKT_FORCEINLINE constexpr To bitCast(const From& value) noexcept
{
    return std::bit_cast<To>(value);
}
}  // namespace dakt::core
