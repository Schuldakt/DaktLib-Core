#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include "../types/Span.hpp"

namespace dakt::core {

using EventId = std::uint64_t;
using SubscriptionToken = std::uint64_t;

struct IEventBus {
  virtual ~IEventBus() = default;

  virtual void publish(EventId id, Span<const std::byte> payload) = 0;
  virtual SubscriptionToken
  subscribe(EventId id, std::function<void(Span<const std::byte>)> handler) = 0;
  virtual void unsubscribe(SubscriptionToken token) = 0;
};

} // namespace dakt::core
