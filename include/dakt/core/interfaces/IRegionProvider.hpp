#pragma once

#include <string>
#include <vector>

#include "../types/Result.hpp"
#include "../types/StringView.hpp"

namespace dakt::core {

struct Rect {
  int x{};
  int y{};
  int width{};
  int height{};
};

struct IRegionProvider {
  virtual ~IRegionProvider() = default;

  virtual Result<Rect, std::string> getRegion(StringView name) const = 0;
  virtual std::vector<StringView> listRegions() const = 0;
};

} // namespace dakt::core
