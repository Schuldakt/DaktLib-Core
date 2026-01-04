#pragma once

#include "../interfaces/ILogger.hpp"

namespace dakt::core {

struct NullLogger : ILogger {
  void log(Severity, StringView) override {}
  void flush() override {}
  void setMinSeverity(Severity) override {}
};

} // namespace dakt::core
