#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <new>

#include "../interfaces/IAllocator.hpp"

namespace dakt::core {

struct SystemAllocator : IAllocator {
  void *allocate(std::size_t size,
                 std::size_t alignment = alignof(std::max_align_t)) override {
    return ::operator new(size, std::align_val_t{alignment});
  }

  void deallocate(void *ptr, std::size_t size) override {
    ::operator delete(ptr, size);
  }

  void *reallocate(void *ptr, std::size_t oldSize,
                   std::size_t newSize) override {
    void *newPtr = allocate(newSize, alignof(std::max_align_t));
    const std::size_t copySize = std::min(oldSize, newSize);
    std::memcpy(newPtr, ptr, copySize);
    deallocate(ptr, oldSize);
    return newPtr;
  }
};

} // namespace dakt::core
