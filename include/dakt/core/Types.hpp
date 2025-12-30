#pragma once

// ============================================================================
// DaktLib Core - Types
// Fundamental types using C++23 std::expected, std::optional, std::span
// ============================================================================

#include "Macros.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace dakt::core
{

// ============================================================================
// Fixed-width Integer Aliases
// ============================================================================

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// Use std::byte from C++17
using byte = std::byte;

// ============================================================================
// String Types
// ============================================================================

using String = std::string;
using StringView = std::string_view;

// ============================================================================
// Optional Type (C++23 std::optional with monadic operations)
// ============================================================================

template <typename T>
using Option = std::optional<T>;

inline constexpr std::nullopt_t none = std::nullopt;

template <typename T>
[[nodiscard]] constexpr Option<std::decay_t<T>> some(T&& value)
{
    return Option<std::decay_t<T>>(std::forward<T>(value));
}

// ============================================================================
// Unit Type (for Result<void, E>)
// ============================================================================

struct Unit
{
    constexpr auto operator<=>(const Unit&) const = default;
};

inline constexpr Unit unit{};

// ============================================================================
// Error Types
// ============================================================================

enum class ErrorCode : u32
{
    None = 0,
    Unknown,
    InvalidArgument,
    NullPointer,
    OutOfMemory,
    OutOfBounds,
    InvalidState,
    NotFound,
    AlreadyExists,
    AccessDenied,
    NotSupported,
    Timeout,
    Cancelled,
    IoError,
    ParseError,
    FormatError,
    EncryptionError,
    DecryptionError,
};

struct Error
{
    ErrorCode code = ErrorCode::None;
    String message;

    constexpr Error() = default;
    constexpr Error(ErrorCode c) : code(c) {}
    Error(ErrorCode c, StringView msg) : code(c), message(msg) {}
    Error(ErrorCode c, String&& msg) : code(c), message(std::move(msg)) {}

    [[nodiscard]] constexpr bool isOk() const noexcept { return code == ErrorCode::None; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return code != ErrorCode::None; }

    [[nodiscard]] constexpr auto operator<=>(const Error& other) const = default;
};

// ============================================================================
// Result Type (C++23 std::expected)
// ============================================================================

template <typename T, typename E = Error>
using Result = std::expected<T, E>;

// Convenience for void results
template <typename E = Error>
using VoidResult = Result<Unit, E>;

// Common Result aliases
template <typename T>
using GenericResult = Result<T, Error>;

using VoidGenericResult = Result<Unit, Error>;

// Helper to create error results
template <typename E>
[[nodiscard]] constexpr auto makeError(E&& error)
{
    return std::unexpected(std::forward<E>(error));
}

[[nodiscard]] inline auto makeError(ErrorCode code, StringView message = {})
{
    return std::unexpected(Error{code, String(message)});
}

// ============================================================================
// Span Types (C++20 std::span)
// ============================================================================

template <typename T, std::size_t Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

using ByteSpan = Span<byte>;
using ConstByteSpan = Span<const byte>;

// Helper to create byte spans from any contiguous data
template <typename T>
    requires std::is_trivially_copyable_v<T>
[[nodiscard]] constexpr ConstByteSpan asBytes(const T& value) noexcept
{
    return std::as_bytes(Span<const T, 1>(&value, 1));
}

template <typename T>
    requires std::is_trivially_copyable_v<T>
[[nodiscard]] constexpr ByteSpan asWritableBytes(T& value) noexcept
{
    return std::as_writable_bytes(Span<T, 1>(&value, 1));
}

// ============================================================================
// Concepts
// ============================================================================

template <typename T>
concept Integral = std::integral<T>;

template <typename T>
concept FloatingPoint = std::floating_point<T>;

template <typename T>
concept Numeric = Integral<T> || FloatingPoint<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

template <typename T, typename... Args>
concept ConstructibleFrom = std::constructible_from<T, Args...>;

template <typename From, typename To>
concept ConvertibleTo = std::convertible_to<From, To>;

template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Stringable = requires(T a) {
    { std::to_string(a) } -> std::convertible_to<std::string>;
} || requires(T a) {
    { a.toString() } -> std::convertible_to<std::string>;
};

// ============================================================================
// Type Utilities (C++23)
// ============================================================================

// Convert enum to underlying type (C++23 std::to_underlying)
template <Enum E>
[[nodiscard]] constexpr auto toUnderlying(E e) noexcept
{
    return std::to_underlying(e);
}

// ============================================================================
// InPlace Tags
// ============================================================================

using std::in_place;
using std::in_place_index;
using std::in_place_index_t;
using std::in_place_t;
using std::in_place_type;
using std::in_place_type_t;

// ============================================================================
// Nullable Pointer Wrapper (for explicit null semantics)
// ============================================================================

template <typename T>
    requires std::is_pointer_v<T>
class NotNull
{
public:
    constexpr NotNull(T ptr) : m_ptr(ptr) { DAKT_ASSERT_MSG(ptr != nullptr, "NotNull initialized with nullptr"); }

    // Prevent nullptr construction
    NotNull(std::nullptr_t) = delete;

    [[nodiscard]] constexpr T get() const noexcept { return m_ptr; }
    [[nodiscard]] constexpr operator T() const noexcept { return m_ptr; }
    [[nodiscard]] constexpr auto operator->() const noexcept { return m_ptr; }
    [[nodiscard]] constexpr auto& operator*() const noexcept { return *m_ptr; }

private:
    T m_ptr;
};

template <typename T>
NotNull(T) -> NotNull<T>;

// ============================================================================
// Strong Typedef Helper
// ============================================================================

template <typename T, typename Tag>
class StrongType
{
public:
    using ValueType = T;

    constexpr StrongType() = default;
    constexpr explicit StrongType(T value) : m_value(std::move(value)) {}

    [[nodiscard]] constexpr T& value() & noexcept { return m_value; }
    [[nodiscard]] constexpr const T& value() const& noexcept { return m_value; }
    [[nodiscard]] constexpr T&& value() && noexcept { return std::move(m_value); }

    [[nodiscard]] constexpr explicit operator T&() & noexcept { return m_value; }
    [[nodiscard]] constexpr explicit operator const T&() const& noexcept { return m_value; }

    constexpr auto operator<=>(const StrongType&) const = default;

private:
    T m_value{};
};

// ============================================================================
// Defer Type Deduction
// ============================================================================

// Helper for auto return type deduction in lambdas
template <typename F>
struct DeferredCall
{
    F func;
    constexpr auto operator()() const { return func(); }
};

template <typename F>
DeferredCall(F) -> DeferredCall<F>;

}  // namespace dakt::core