#pragma once

#include <string>
#include <vector>

#include "../types/Result.hpp"
#include "../types/Span.hpp"

namespace dakt::core {

struct ISerializable {
  virtual ~ISerializable() = default;

  virtual Result<std::vector<std::byte>, std::string> serialize() const = 0;
  virtual Result<void, std::string> deserialize(Span<const std::byte> data) = 0;
};

} // namespace dakt::core
