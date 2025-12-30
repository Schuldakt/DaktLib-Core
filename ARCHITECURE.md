# Core Module Architecture

## Overview

The Core module provides foundational types and utilities used by all other DaktLib modules. It has **zero external dependencies** beyond the C++23 standard library.

## Component Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                           Core Module                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                  │
│  │   Macros    │  │    Types    │  │  Platform   │                  │
│  │  (defines,  │  │  (Result,   │  │ (OS detect, │                  │
│  │  asserts)   │  │  Option,    │  │  byte swap, │                  │
│  │             │  │  concepts)  │  │  bit ops)   │                  │
│  └─────────────┘  └─────────────┘  └─────────────┘                  │
│         │               │               │                           │
│         └───────────────┼───────────────┘                           │
│                         │                                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                  │
│  │   Memory    │  │   String    │  │   Buffer    │                  │
│  │ (allocators,│  │  (utils,    │  │  (binary    │                  │
│  │  smart ptr) │  │  encoding)  │  │  read/write)│                  │
│  └─────────────┘  └─────────────┘  └─────────────┘                  │
│                                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                  │
│  │    Hash     │  │    Time     │  │ FileSystem  │                  │
│  │  (FNV, CRC, │  │ (stopwatch, │  │  (paths,    │                  │
│  │  xxhash)    │  │  timestamp) │  │  file I/O)  │                  │
│  └─────────────┘  └─────────────┘  └─────────────┘                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## File Structure

```
Core/
├── include/dakt/core/
│   ├── Core.hpp          # Main header (includes all)
│   ├── Macros.hpp        # Compiler detection, assertions, utilities
│   ├── Types.hpp         # Result, Option, Span, integer types, concepts
│   ├── Platform.hpp      # OS abstraction, byte swapping, bit operations
│   ├── Memory.hpp        # Allocators, smart pointers
│   ├── String.hpp        # String utilities, encoding
│   ├── Buffer.hpp        # Binary buffer, reader/writer
│   ├── Hash.hpp          # Hash functions
│   ├── Time.hpp          # Time utilities
│   └── FileSystem.hpp    # File operations, paths
├── src/
│   ├── Platform.cpp
│   ├── Memory.cpp
│   ├── String.cpp
│   ├── Buffer.cpp
│   ├── Hash.cpp
│   ├── Time.cpp
│   └── FileSystem.cpp
├── ARCHITECTURE.md
├── TODO.md
└── CMakeLists.txt
```

## Component Details

### Macros.hpp

Foundation macros and utilities:

| Category | Items |
|----------|-------|
| Version | `DAKT_VERSION_MAJOR/MINOR/PATCH` |
| Compiler Detection | `DAKT_COMPILER_MSVC/GCC/CLANG` |
| Architecture | `DAKT_ARCH_X64/X86/ARM64`, `DAKT_ARCH_64BIT/32BIT` |
| Attributes | `DAKT_FORCEINLINE`, `DAKT_NOINLINE`, `DAKT_LIKELY/UNLIKELY` |
| C++23 | `DAKT_ASSUME()`, `DAKT_UNREACHABLE()` |
| Assertions | `DAKT_ASSERT()`, `DAKT_ASSERT_MSG()`, `DAKT_VERIFY()` |
| Utilities | `DAKT_UNUSED()`, `DAKT_STRINGIFY()`, `DAKT_SCOPE_EXIT()` |

**Implementation Notes:**
- Assertions use `std::source_location` for file/line info
- `DAKT_UNREACHABLE()` maps to `std::unreachable()` (C++23)
- `DAKT_ASSUME()` maps to `[[assume(expr)]]` (C++23)

### Types.hpp

Core type definitions leveraging C++23:

```cpp
// Integer aliases
using i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, usize, isize;
using byte = std::byte;

// Optional (C++23 std::optional with monadic ops)
template<typename T>
using Option = std::optional<T>;
inline constexpr std::nullopt_t none = std::nullopt;

// Result (C++23 std::expected)
template<typename T, typename E = Error>
using Result = std::expected<T, E>;

// Span (C++20 std::span)
template<typename T>
using Span = std::span<T>;
using ByteSpan = Span<byte>;
using ConstByteSpan = Span<const byte>;
```

**Concepts Defined:**
- `Integral`, `FloatingPoint`, `Numeric`
- `TriviallyCopyable`, `TriviallyDestructible`
- `ConstructibleFrom`, `ConvertibleTo`
- `Hashable`, `Stringable`

**Utilities:**
- `Unit` - Unit type for `Result<void, E>`
- `NotNull<T*>` - Non-null pointer wrapper
- `StrongType<T, Tag>` - Strong typedef helper
- `toUnderlying()` - Enum to underlying type (C++23 `std::to_underlying`)

### Platform.hpp

OS abstraction and low-level utilities:

| Function | Description |
|----------|-------------|
| `getSystemInfo()` | CPU count, memory, OS version |
| `getEnv()/setEnv()` | Environment variables |
| `getCurrentDirectory()` | Working directory |
| `getExecutablePath()` | Current exe path |
| `getHomeDirectory()` | User home |
| `getTempDirectory()` | Temp folder |
| `getAppDataDirectory()` | Per-user app data |
| `sleepMs()/sleepUs()` | Thread sleep |
| `virtualAlloc()/virtualFree()` | Virtual memory |
| `loadLibrary()/getLibrarySymbol()` | Dynamic loading |

**Byte Swapping (C++23):**
```cpp
template<Integral T>
constexpr T byteSwap(T value);           // std::byteswap

template<Integral T>
constexpr T toLittleEndian(T value);     // Conditional byteswap
template<Integral T>
constexpr T toBigEndian(T value);
```

**Bit Operations (C++20 `<bit>`):**
```cpp
countLeadingZeros(), countTrailingZeros(), popCount()
isPowerOfTwo(), nextPowerOfTwo(), prevPowerOfTwo()
bitWidth(), rotateLeft(), rotateRight(), bitCast()
```

### Memory.hpp

Memory management utilities:

**Allocator Interface:**
```cpp
class IAllocator {
    virtual void* allocate(usize size, usize alignment) = 0;
    virtual void deallocate(void* ptr, usize size) = 0;
    virtual void* reallocate(void* ptr, usize old, usize new, usize align) = 0;
    
    template<typename T, typename... Args>
    T* create(Args&&... args);  // Allocate + construct
    
    template<typename T>
    void destroy(T* ptr);       // Destruct + deallocate
};
```

**Allocator Implementations:**

| Allocator | Use Case |
|-----------|----------|
| `HeapAllocator` | General purpose (aligned_alloc) |
| `ArenaAllocator` | Bulk allocation, single reset |
| `PoolAllocator` | Fixed-size blocks, frequent alloc/free |

**Smart Pointers:**
```cpp
template<derived_from<RefCounted> T>
class Ref;  // Intrusive reference counting

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using Shared = std::shared_ptr<T>;
```

**PMR Integration:**
```cpp
class PmrAllocatorAdapter : public std::pmr::memory_resource;
```

### String.hpp

String utilities with C++23 features:

**Operations:**
- Trimming: `trimLeft()`, `trimRight()`, `trim()`
- Case: `toLower()`, `toUpper()`, `equalsIgnoreCase()`
- Search: `contains()` (C++23), `startsWith()`, `endsWith()`
- Split/Join: `split()`, `splitLines()`, `join()`
- Modify: `replace()`, `remove()`, `repeat()`, `reverse()`
- Padding: `padLeft()`, `padRight()`, `center()`

**Parsing:**
```cpp
template<Integral T>
Option<T> parseInt(StringView str, int base = 10);

template<FloatingPoint T>
Option<T> parseFloat(StringView str);

Option<bool> parseBool(StringView str);
```

**Formatting (C++20 `std::format`):**
```cpp
template<typename... Args>
String format(std::format_string<Args...> fmt, Args&&... args);
```

**Encoding:**
- `toHex()` / `fromHex()` - Hexadecimal
- `toBase64()` / `fromBase64()` - Base64
- `utf8::toWide()` / `utf8::fromWide()` - UTF-16 conversion

### Buffer.hpp

Binary data handling:

**Buffer Class:**
```cpp
class Buffer {
    // Dynamic byte vector with span interface
    ByteSpan span();
    void append(ConstByteSpan data);
    void resize(usize size);
};
```

**BufferReader:**
```cpp
class BufferReader {
    template<TriviallyCopyable T>
    Option<T> read();           // Read any trivially copyable type
    
    Option<u32> readU32();      // Little-endian
    Option<u32> readU32BE();    // Big-endian
    Option<String> readNullTerminatedString();
    
    template<TriviallyCopyable T>
    Option<T> peek();           // Read without advancing
};
```

**BufferWriter:**
```cpp
class BufferWriter {
    template<TriviallyCopyable T>
    void write(T value);
    
    void writeLE<T>(T value);   // Explicit little-endian
    void writeBE<T>(T value);   // Explicit big-endian
    void writeNullTerminatedString(StringView str);
    void align(usize alignment);
    
    Buffer toBuffer();
};
```

### Hash.hpp

Hash functions:

| Function | Speed | Quality | Use Case |
|----------|-------|---------|----------|
| `fnv1a32/64` | Fast | Good | Hash tables |
| `crc32` | Medium | High | Checksums |
| `xxhash32/64` | Very Fast | Excellent | General purpose |
| `murmur3_32/128` | Fast | Excellent | Hash tables |

**Compile-time Hashing:**
```cpp
consteval u32 constHash32(const char* str, usize len);

// Usage with literal operator
switch (hash::fnv1a32(input)) {
    case "open"_hash32: ...
    case "close"_hash32: ...
}
```

**Hash Combining:**
```cpp
u64 combined = hash::combineHashes(hash1, hash2, hash3);
```

### Time.hpp

Time utilities using C++20/23 chrono:

**Duration Types:**
```cpp
using Nanoseconds = std::chrono::nanoseconds;
using Milliseconds = std::chrono::milliseconds;
using Days = std::chrono::days;  // C++20
```

**Stopwatch:**
```cpp
class Stopwatch {
    void start(), stop(), reset(), restart();
    i64 elapsedMillis();
    f64 elapsedSeconds();
    static Stopwatch startNew();
};
```

**Timer Classes:**
- `ScopedTimer` - RAII timing with callback
- `Timer` - Periodic interval timer
- `FrameTimer` - Game loop timing with FPS calculation

**Timestamp:**
```cpp
struct Timestamp {
    i32 year, month, day, hour, minute, second, millisecond;
};

Timestamp localTime();
Timestamp utcTime();
String formatISO8601(const Timestamp& ts);
```

### FileSystem.hpp

File operations wrapping `std::filesystem`:

**Path Utilities:**
```cpp
namespace path {
    String normalize(StringView path);
    String join(StringView a, StringView b);
    String parent(StringView path);
    String filename(StringView path);
    String extension(StringView path);
    bool isAbsolute(StringView path);
}
```

**File Operations:**
```cpp
namespace fs {
    bool exists(StringView path);
    bool isFile(StringView path);
    bool isDirectory(StringView path);
    
    GenericResult<Buffer> readFile(StringView path);
    GenericResult<String> readTextFile(StringView path);
    GenericResult<Unit> writeFile(StringView path, ConstByteSpan data);
    
    GenericResult<Unit> createDirectory(StringView path);
    GenericResult<Unit> deleteFile(StringView path);
    GenericResult<Unit> copyFile(StringView src, StringView dst);
}
```

**Directory Traversal:**
```cpp
enum class TraversalAction { Continue, Skip, Stop };

GenericResult<Unit> walkDirectory(
    StringView path,
    std::function<TraversalAction(const FileInfo&, int depth)> callback,
    bool recursive = true
);
```

**Memory Mapped Files:**
```cpp
class MemoryMappedFile {
    static GenericResult<MemoryMappedFile> open(StringView path);
    ByteSpan span();
    GenericResult<Unit> flush();
};
```

## Thread Safety

| Component | Thread Safety |
|-----------|---------------|
| Types (Result, Option, Span) | Safe for independent instances |
| HeapAllocator | Thread-safe (uses OS allocator) |
| ArenaAllocator | NOT thread-safe |
| PoolAllocator | NOT thread-safe |
| String utilities | Thread-safe (stateless) |
| Buffer | Safe for independent instances |
| BufferReader/Writer | NOT thread-safe |
| Hash functions | Thread-safe (stateless) |
| Time utilities | Thread-safe |
| File operations | Thread-safe for different files |
| MemoryMappedFile | NOT thread-safe |

## Performance Considerations

1. **Small String Optimization** - Uses `std::string` which has SSO
2. **Move Semantics** - All types are move-aware
3. **Inline Functions** - Critical paths marked `DAKT_FORCEINLINE`
4. **constexpr** - Compile-time evaluation where possible
5. **Cache Locality** - `Span<T>` for contiguous data access
6. **No Virtual Dispatch** - Templates preferred for hot paths

## Error Handling

```cpp
// Pattern 1: Early return
auto loadFile(StringView path) -> Result<Buffer, Error> {
    if (!fs::exists(path)) {
        return makeError(ErrorCode::NotFound, "File not found");
    }
    return fs::readFile(path);
}

// Pattern 2: Monadic chaining (C++23)
auto result = readFile(path)
    .and_then(parseJson)
    .transform(extractData)
    .or_else(handleError);

// Pattern 3: Match/visit
result.match(
    [](const Data& d) { process(d); },
    [](const Error& e) { log(e); }
);
```

## Usage Examples

```cpp
#include <dakt/core/Core.hpp>

using namespace dakt::core;

// Read and process a file
auto processFile(StringView path) -> Result<String, Error> {
    // Read file
    auto buffer = fs::readFile(path);
    if (!buffer) return std::unexpected(buffer.error());
    
    // Parse as binary
    BufferReader reader(buffer->span());
    
    auto magic = reader.readU32();
    if (!magic || *magic != 0x12345678) {
        return makeError(ErrorCode::ParseError, "Invalid magic");
    }
    
    auto name = reader.readNullTerminatedString();
    if (!name) {
        return makeError(ErrorCode::ParseError, "Missing name");
    }
    
    return *name;
}

// Timing example
void benchmark() {
    auto sw = Stopwatch::startNew();
    
    // ... work ...
    
    auto elapsed = sw.elapsedMillis();
    log::info("Completed in {} ms", elapsed);
}
```