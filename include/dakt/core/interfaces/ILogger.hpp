#pragma once

#include <cstdint>
#include <format>
#include <utility>

#include "../types/StringView.hpp"

namespace dakt::core {

enum class Severity : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

struct ILogger {
  virtual ~ILogger() = default;

  virtual void log(Severity level, StringView msg) = 0;

  template <typename... Args>
  void log(Severity level, std::format_string<Args...> fmt, Args &&...args) {
    log(level, std::format(fmt, std::forward<Args>(args)...));
  }

  virtual void flush() = 0;
  virtual void setMinSeverity(Severity level) = 0;
};

} // namespace dakt::core
