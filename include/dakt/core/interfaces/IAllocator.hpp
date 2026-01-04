#pragma once

#include <cstddef>
#include <new>

namespace dakt::core {

struct IAllocator {
  virtual ~IAllocator() = default;

  virtual void *allocate(std::size_t size,
                         std::size_t alignment = alignof(std::max_align_t)) = 0;
  virtual void deallocate(void *ptr, std::size_t size) = 0;
  virtual void *reallocate(void *ptr, std::size_t oldSize,
                           std::size_t newSize) = 0;
};

} // namespace dakt::core
