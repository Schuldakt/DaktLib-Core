#pragma once

#include <cstddef>
#include <type_traits>

namespace dakt::core {

template <typename T> class Span {
public:
  using element_type = T;
  using pointer = T *;
  using reference = T &;

  static constexpr std::size_t npos = static_cast<std::size_t>(-1);

  constexpr Span() noexcept = default;
  constexpr Span(T *data, std::size_t size) noexcept
      : data_(data), size_(size) {}

  template <std::size_t N>
  constexpr Span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}

  template <typename Container>
    requires requires(Container &c) {
      c.data();
      c.size();
    }
  constexpr explicit Span(Container &c) noexcept
      : data_(c.data()), size_(c.size()) {}

  [[nodiscard]] constexpr T *data() const noexcept { return data_; }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

  [[nodiscard]] constexpr T &operator[](std::size_t idx) const {
    return data_[idx];
  }

  [[nodiscard]] constexpr Span subspan(std::size_t offset,
                                       std::size_t count = npos) const {
    if (offset > size_) {
      return {};
    }
    const std::size_t remaining = size_ - offset;
    const std::size_t clamped =
        count == npos ? remaining : (count > remaining ? remaining : count);
    return Span(data_ + offset, clamped);
  }

  [[nodiscard]] constexpr T *begin() const noexcept { return data_; }
  [[nodiscard]] constexpr T *end() const noexcept { return data_ + size_; }

private:
  T *data_{nullptr};
  std::size_t size_{0};
};

} // namespace dakt::core
