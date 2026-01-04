# DaktLib-Core Architecture

> Header-only C++23 module providing shared interfaces for the DaktLib ecosystem.

## Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         DaktLib Modules                             │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐        │
│  │ Logger  │ │ Events  │ │ Export  │ │   OCR   │ │   ...   │        │
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘        │
│       │           │           │           │           │             │
│       └───────────┴───────────┴─────┬─────┴───────────┘             │
│                                     ▼                               │
│                         ┌───────────────────┐                       │
│                         │    DaktLib-Core   │                       │
│                         │  (Header-Only)    │                       │
│                         └───────────────────┘                       │
└─────────────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
DaktLib-Core/
├── include/
│   └── dakt/
│       └── core/
│           ├── interfaces/
│           │   ├── ILogger.hpp
│           │   ├── IAllocator.hpp
│           │   ├── IEventBus.hpp
│           │   ├── ISerializable.hpp
│           │   └── IRegionProvider.hpp
│           ├── types/
│           │   ├── Result.hpp
│           │   ├── Span.hpp
│           │   └── StringView.hpp
│           ├── concepts/
│           │   └── CoreConcepts.hpp
│           └── Core.hpp              # Aggregate header
├── tests/
│   └── unit/
├── CMakeLists.txt
├── ARCHITECTURE.md
└── TODO.md
```

## Namespace Layout

```cpp
namespace dakt::core {
    // Interfaces
    struct ILogger;
    struct IAllocator;
    struct IEventBus;
    struct ISerializable;
    struct IRegionProvider;
    
    // Types
    template<typename T, typename E> class Result;
    template<typename T> class Span;
    class StringView;
    
    // Concepts
    template<typename T> concept Loggable;
    template<typename T> concept Serializable;
}
```

## Interface Specifications

### ILogger
```cpp
enum class Severity { Trace, Debug, Info, Warn, Error, Fatal };

struct ILogger {
    virtual ~ILogger() = default;
    virtual void log(Severity level, StringView msg) = 0;
    
    template<typename... Args>
    void log(Severity level, std::format_string<Args...> fmt, Args&&... args);
    
    virtual void flush() = 0;
    virtual void setMinSeverity(Severity level) = 0;
};
```

### IAllocator
```cpp
struct IAllocator {
    virtual ~IAllocator() = default;
    virtual void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) = 0;
    virtual void deallocate(void* ptr, std::size_t size) = 0;
    virtual void* reallocate(void* ptr, std::size_t oldSize, std::size_t newSize) = 0;
};
```

### IEventBus
```cpp
using EventId = std::uint64_t;
using SubscriptionToken = std::uint64_t;

struct IEventBus {
    virtual ~IEventBus() = default;
    virtual void publish(EventId id, Span<const std::byte> payload) = 0;
    virtual SubscriptionToken subscribe(EventId id, std::function<void(Span<const std::byte>)> handler) = 0;
    virtual void unsubscribe(SubscriptionToken token) = 0;
};
```

### ISerializable
```cpp
struct ISerializable {
    virtual ~ISerializable() = default;
    virtual Result<std::vector<std::byte>, std::string> serialize() const = 0;
    virtual Result<void, std::string> deserialize(Span<const std::byte> data) = 0;
};
```

### IRegionProvider
```cpp
struct Rect { int x, y, width, height; };

struct IRegionProvider {
    virtual ~IRegionProvider() = default;
    virtual Result<Rect, std::string> getRegion(StringView name) const = 0;
    virtual std::vector<StringView> listRegions() const = 0;
};
```

### Result<T, E>
```cpp
template<typename T, typename E>
class Result {
public:
    // Constructors
    static Result ok(T value);
    static Result err(E error);
    
    // Accessors
    [[nodiscard]] bool isOk() const noexcept;
    [[nodiscard]] bool isErr() const noexcept;
    T& value() &;
    const T& value() const&;
    E& error() &;
    const E& error() const&;
    
    // Monadic operations
    template<typename F> 
    auto map(F&& f) -> Result<std::invoke_result_t<F, T>, E>;
    
    template<typename F> 
    auto andThen(F&& f) -> std::invoke_result_t<F, T>;
    
    template<typename F> 
    auto orElse(F&& f) -> Result<T, std::invoke_result_t<F, E>>;
    
    T valueOr(T defaultVal) const;
    
    // Operators
    explicit operator bool() const noexcept { return isOk(); }
};

// Specialization for void
template<typename E> 
class Result<void, E>;
```

### Span<T>
```cpp
template<typename T>
class Span {
public:
    constexpr Span() noexcept = default;
    constexpr Span(T* data, std::size_t size) noexcept;
    
    template<std::size_t N> 
    constexpr Span(T (&arr)[N]) noexcept;
    
    template<typename Container>
    constexpr Span(Container& c) noexcept;
    
    [[nodiscard]] constexpr T* data() const noexcept;
    [[nodiscard]] constexpr std::size_t size() const noexcept;
    [[nodiscard]] constexpr bool empty() const noexcept;
    
    [[nodiscard]] constexpr T& operator[](std::size_t idx) const;
    
    [[nodiscard]] constexpr Span subspan(std::size_t offset, std::size_t count = npos) const;
    
    [[nodiscard]] constexpr T* begin() const noexcept;
    [[nodiscard]] constexpr T* end() const noexcept;
    
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
};
```

### StringView
```cpp
class StringView {
public:
    constexpr StringView() noexcept = default;
    constexpr StringView(const char* str) noexcept;
    constexpr StringView(const char* str, std::size_t len) noexcept;
    constexpr StringView(const std::string& str) noexcept;
    
    [[nodiscard]] constexpr const char* data() const noexcept;
    [[nodiscard]] constexpr std::size_t size() const noexcept;
    [[nodiscard]] constexpr std::size_t length() const noexcept;
    [[nodiscard]] constexpr bool empty() const noexcept;
    
    [[nodiscard]] constexpr char operator[](std::size_t idx) const;
    
    [[nodiscard]] constexpr StringView substr(std::size_t pos, std::size_t count = npos) const;
    [[nodiscard]] constexpr std::size_t find(char c, std::size_t pos = 0) const noexcept;
    [[nodiscard]] constexpr std::size_t find(StringView sv, std::size_t pos = 0) const noexcept;
    
    [[nodiscard]] constexpr bool operator==(StringView other) const noexcept;
    [[nodiscard]] constexpr auto operator<=>(StringView other) const noexcept;
    
    // Conversion
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] constexpr operator std::string_view() const noexcept;
    
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
};
```

## Concepts

```cpp
namespace dakt::core {

template<typename T>
concept Loggable = requires(T t) {
    { t.toString() } -> std::convertible_to<std::string>;
} || std::convertible_to<T, std::string>;

template<typename T>
concept Serializable = std::derived_from<T, ISerializable>;

template<typename T>
concept Allocatable = requires(T alloc, void* ptr, std::size_t size) {
    { alloc.allocate(size) } -> std::same_as<void*>;
    { alloc.deallocate(ptr, size) } -> std::same_as<void>;
};

template<typename T>
concept RegionProvider = requires(T provider, StringView name) {
    { provider.getRegion(name) } -> std::same_as<Result<Rect, std::string>>;
    { provider.listRegions() } -> std::convertible_to<std::vector<StringView>>;
};

} // namespace dakt::core
```

## Design Principles

| Principle | Rationale |
|-----------|-----------|
| Header-only | Zero link-time dependencies, simple integration |
| Pure interfaces | Virtual dispatch for runtime polymorphism |
| No exceptions | `Result<T,E>` for explicit error handling |
| Minimal STL | Custom `Span`/`StringView` to reduce header bloat |
| C++23 | `std::format`, concepts, `[[nodiscard]]`, `constexpr` |

## CMake Integration

```cmake
# DaktLib-Core/CMakeLists.txt
cmake_minimum_required(VERSION 4.2.1)
project(DaktLib-Core VERSION 1.0.0 LANGUAGES CXX)

add_library(DaktCore INTERFACE)
add_library(Dakt::Core ALIAS DaktCore)

target_include_directories(DaktCore INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(DaktCore INTERFACE cxx_std_23)

# Consumer usage
# find_package(DaktLib-Core REQUIRED)
# target_link_libraries(MyModule INTERFACE Dakt::Core)
```

## Default Implementations

DaktLib-Core also provides minimal default implementations:

### NullLogger
```cpp
struct NullLogger : ILogger {
    void log(Severity, StringView) override {}
    void flush() override {}
    void setMinSeverity(Severity) override {}
};
```

### SystemAllocator
```cpp
struct SystemAllocator : IAllocator {
    void* allocate(std::size_t size, std::size_t alignment) override {
        return ::operator new(size, std::align_val_t{alignment});
    }
    void deallocate(void* ptr, std::size_t size) override {
        ::operator delete(ptr, size);
    }
    void* reallocate(void* ptr, std::size_t oldSize, std::size_t newSize) override {
        void* newPtr = allocate(newSize, alignof(std::max_align_t));
        std::memcpy(newPtr, ptr, std::min(oldSize, newSize));
        deallocate(ptr, oldSize);
        return newPtr;
    }
};
```

## Platform Support

| Platform | Compiler | Min Version |
|----------|----------|-------------|
| Windows | MSVC | 19.36+ |
| Windows | Clang | 16+ |
| Linux | GCC | 13+ |
| Linux | Clang | 16+ |
| macOS | AppleClang | 15+ |
