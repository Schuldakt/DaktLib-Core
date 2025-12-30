#pragma once

// ============================================================================
// DaktLib Core
// Main header - includes all core headers (C++23)
// ============================================================================

#include "dakt/core/Buffer.hpp"
#include "dakt/core/FileSystem.hpp"
#include "dakt/core/Hash.hpp"
#include "dakt/core/Macros.hpp"
#include "dakt/core/Memory.hpp"
#include "dakt/core/Platform.hpp"
#include "dakt/core/String.hpp"
#include "dakt/core/Time.hpp"
#include "dakt/core/Types.hpp"

// ============================================================================
// Version Info
// ============================================================================

namespace dakt::core
{

struct Version
{
    int major;
    int minor;
    int patch;

    [[nodiscard]] constexpr auto operator<=>(const Version&) const = default;

    [[nodiscard]] String toString() const { return string::format("{}.{}.{}", major, minor, patch); }
};

inline constexpr Version LibraryVersion = {DAKT_VERSION_MAJOR, DAKT_VERSION_MINOR, DAKT_VERSION_PATCH};

// C++ Standard Version
inline constexpr int CppStandard = __cplusplus;

// Compile-time check for C++23
static_assert(__cplusplus >= 202302L, "DaktLib requires C++23 or later");

}  // namespace dakt::core