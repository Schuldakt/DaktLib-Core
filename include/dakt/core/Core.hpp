// Aggregate header for DaktLib-Core public surface.
#pragma once

#include "types/Result.hpp"
#include "types/Span.hpp"
#include "types/StringView.hpp"

#include "concepts/CoreConcepts.hpp"

#include "interfaces/IAllocator.hpp"
#include "interfaces/IEventBus.hpp"
#include "interfaces/ILogger.hpp"
#include "interfaces/IRegionProvider.hpp"
#include "interfaces/ISerializable.hpp"

#include "logging/NullLogger.hpp"
#include "memory/SystemAllocator.hpp"

namespace dakt::core {
// Intentionally empty: this header simply aggregates the core surface.
}
