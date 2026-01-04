#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <vector>

#include "../interfaces/IAllocator.hpp"
#include "../interfaces/IRegionProvider.hpp"
#include "../interfaces/ISerializable.hpp"
#include "../types/Result.hpp"
#include "../types/StringView.hpp"

namespace dakt::core {

template <typename T>
concept Loggable = requires(T t) {
  { t.toString() } -> std::convertible_to<std::string>;
} || std::convertible_to<T, std::string_view>;

template <typename T>
concept Serializable = std::derived_from<T, ISerializable>;

template <typename T>
concept Allocatable = requires(T alloc, void *ptr, std::size_t size) {
  { alloc.allocate(size) } -> std::same_as<void *>;
  { alloc.deallocate(ptr, size) } -> std::same_as<void>;
};

template <typename T>
concept RegionProvider = requires(T provider, StringView name) {
  { provider.getRegion(name) } -> std::same_as<Result<Rect, std::string>>;
  { provider.listRegions() } -> std::convertible_to<std::vector<StringView>>;
};

} // namespace dakt::core
