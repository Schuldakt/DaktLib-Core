#pragma once

// ============================================================================
// DaktLib Core - Macros
// Foundation macros using C++23 features
// ============================================================================

#include <cstdlib>
#include <cstdio>
#include <source_location>
#include <utility>

// ============================================================================
// Version
// ============================================================================

#define DAKT_VERSION_MAJOR 1
#define DAKT_VERSION_MINOR 0
#define DAKT_VERSION_PATCH 0
#define DAKT_VERSION ((DAKT_VERSION_MAJOR << 16) | (DAKT_VERSION_MINOR << 8) | DAKT_VERSION_PATCH)

// ============================================================================
// Compiler Detection
// ============================================================================

#if defined(_MSC_VER)
#define DAKT_COMPILER_MSVC
#define DAKT_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
#define DAKT_COMPILER_CLANG
#define DAKT_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define DAKT_COMPILER_GCC
#define DAKT_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#define DAKT_COMPILER_UNKNOWN
#define DAKT_COMPILER_VERSION 0
#endif

// ============================================================================
// Architecture Detection
// ============================================================================

#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#define DAKT_ARCH_X64
#define DAKT_ARCH_64BIT
#elif defined(_M_IX86) || defined(__i386__)
#define DAKT_ARCH_X86
#define DAKT_ARCH_32BIT
#elif defined(_M_ARM64) || defined(__aarch64__)
#define DAKT_ARCH_ARM64
#define DAKT_ARCH_64BIT
#elif defined(_M_ARM) || defined(__arm__)
#define DAKT_ARCH_ARM
#define DAKT_ARCH_32BIT
#endif

// ============================================================================
// Utility Macros
// ============================================================================

#define DAKT_UNUSED(x) (void)(x)

#define DAKT_STRINGIFY_IMPL(x) #x
#define DAKT_STRINGIFY(x) DAKT_STRINGIFY_IMPL(x)

#define DAKT_CONCAT_IMPL(a, b) a##b
#define DAKT_CONCAT(a, b) DAKT_CONCAT_IMPL(a, b)

#define DAKT_UNIQUE_NAME(prefix) DAKT_CONCAT(prefix, __LINE__)

// ============================================================================
// Compiler Hints (C++23)
// ============================================================================

#if defined(DAKT_COMPILER_MSVC)
#define DAKT_FORCEINLINE __forceinline
#define DAKT_NOINLINE __declspec(noinline)
#define DAKT_RESTRICT __restrict
#define DAKT_DEBUGBREAK() __debugbreak()
#define DAKT_FUNCTION_NAME __FUNCSIG__
#elif defined(DAKT_COMPILER_GCC) || defined(DAKT_COMPILER_CLANG)
#define DAKT_FORCEINLINE [[gnu::always_inline]] inline
#define DAKT_NOINLINE [[gnu::noinline]]
#define DAKT_RESTRICT __restrict__
#define DAKT_DEBUGBREAK() __builtin_trap()
#define DAKT_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define DAKT_FORCEINLINE inline
#define DAKT_NOINLINE
#define DAKT_RESTRICT
#define DAKT_DEBUGBREAK() std::abort()
#define DAKT_FUNCTION_NAME __func__
#endif

// C++23 [[assume]] for optimization hints
#define DAKT_ASSUME(expr) [[assume(expr)]]

// C++20 [[likely]] and [[unlikely]]
#define DAKT_LIKELY [[likely]]
#define DAKT_UNLIKELY [[unlikely]]

// C++23 std::unreachable()
#define DAKT_UNREACHABLE() std::unreachable()

// ============================================================================
// Export/Import Macros
// ============================================================================

#if defined(DAKT_SHARED_LIBRARY)
#if defined(DAKT_PLATFORM_WINDOWS)
#if defined(DAKT_BUILDING_LIBRARY)
#define DAKT_API __declspec(dllexport)
#else
#define DAKT_API __declspec(dllimport)
#endif
#else
#define DAKT_API [[gnu::visibility("default")]]
#endif
#else
#define DAKT_API
#endif

// ============================================================================
// Assertions (using std::source_location)
// ============================================================================

namespace dakt::core::detail {

    [[noreturn]] inline void assertFailed(
        const char* expr,
        std::source_location loc = std::source_location::current()
    ) {
        std::fprintf(stderr,
            "\n======== ASSERTION FAILED ========\n"
            "Expression: %s\n"
            "File:       %s\n"
            "Line:       %u\n"
            "Function:   %s\n"
            "==================================\n",
            expr, loc.file_name(), loc.line(), loc.function_name());
        DAKT_DEBUGBREAK();
        std::abort();
    }

    [[noreturn]] inline void assertFailedMsg(
        const char* expr,
        const char* msg,
        std::source_location loc = std::source_location::current()
    ) {
        std::fprintf(stderr,
            "\n======== ASSERTION FAILED ========\n"
            "Expression: %s\n"
            "Message:    %s\n"
            "File:       %s\n"
            "Line:       %u\n"
            "Function:   %s\n"
            "==================================\n",
            expr, msg, loc.file_name(), loc.line(), loc.function_name());
        DAKT_DEBUGBREAK();
        std::abort();
    }

} // namespace dakt::core::detail

#if defined(DAKT_DEBUG)
#define DAKT_ASSERT(expr) \
        do { \
            if (!(expr)) DAKT_UNLIKELY { \
                ::dakt::core::detail::assertFailed(#expr); \
            } \
        } while(0)

#define DAKT_ASSERT_MSG(expr, msg) \
        do { \
            if (!(expr)) DAKT_UNLIKELY { \
                ::dakt::core::detail::assertFailedMsg(#expr, msg); \
            } \
        } while(0)

#define DAKT_VERIFY(expr) DAKT_ASSERT(expr)
#define DAKT_VERIFY_MSG(expr, msg) DAKT_ASSERT_MSG(expr, msg)
#else
#define DAKT_ASSERT(expr) DAKT_ASSUME(expr)
#define DAKT_ASSERT_MSG(expr, msg) DAKT_ASSUME(expr)
#define DAKT_VERIFY(expr) (void)(expr)
#define DAKT_VERIFY_MSG(expr, msg) (void)(expr)
#endif

// ============================================================================
// Disable Copy/Move Macros
// ============================================================================

#define DAKT_NON_COPYABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete

#define DAKT_NON_MOVABLE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete

#define DAKT_NON_COPYABLE_NON_MOVABLE(ClassName) \
    DAKT_NON_COPYABLE(ClassName); \
    DAKT_NON_MOVABLE(ClassName)

#define DAKT_DEFAULT_MOVE(ClassName) \
    ClassName(ClassName&&) noexcept = default; \
    ClassName& operator=(ClassName&&) noexcept = default

#define DAKT_DEFAULT_COPY(ClassName) \
    ClassName(const ClassName&) = default; \
    ClassName& operator=(const ClassName&) = default

// ============================================================================
// Scope Guard
// ============================================================================

namespace dakt::core {

    template<std::invocable F>
    class ScopeGuard {
    public:
        constexpr explicit ScopeGuard(F&& func) noexcept
            : m_func(std::move(func)), m_active(true) {
        }

        constexpr ~ScopeGuard() { if (m_active) m_func(); }

        constexpr ScopeGuard(ScopeGuard&& other) noexcept
            : m_func(std::move(other.m_func)), m_active(other.m_active) {
            other.dismiss();
        }

        constexpr void dismiss() noexcept { m_active = false; }

        DAKT_NON_COPYABLE(ScopeGuard);

    private:
        F m_func;
        bool m_active;
    };

    template<std::invocable F>
    [[nodiscard]] constexpr ScopeGuard<F> makeScopeGuard(F&& func) noexcept {
        return ScopeGuard<F>(std::forward<F>(func));
    }

} // namespace dakt::core

#define DAKT_SCOPE_EXIT(code) \
    auto DAKT_UNIQUE_NAME(_scope_guard_) = ::dakt::core::makeScopeGuard([&]() { code; })

#define DAKT_DEFER(code) DAKT_SCOPE_EXIT(code)