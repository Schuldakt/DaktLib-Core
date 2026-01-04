#pragma once

#include <compare>
#include <cstddef>
#include <string>
#include <string_view>

namespace dakt::core {

class StringView {
public:
  static constexpr std::size_t npos = static_cast<std::size_t>(-1);

  constexpr StringView() noexcept = default;
  constexpr explicit StringView(const char *str) noexcept
      : data_(str), size_(str ? std::char_traits<char>::length(str) : 0) {}
  constexpr StringView(const char *str, std::size_t len) noexcept
      : data_(str), size_(len) {}
  constexpr StringView(const std::string &str) noexcept
      : data_(str.data()), size_(str.size()) {}

  [[nodiscard]] constexpr const char *data() const noexcept { return data_; }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
  [[nodiscard]] constexpr std::size_t length() const noexcept { return size_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

  [[nodiscard]] constexpr char operator[](std::size_t idx) const {
    return data_[idx];
  }

  [[nodiscard]] constexpr StringView substr(std::size_t pos,
                                            std::size_t count = npos) const {
    if (pos > size_) {
      return {};
    }
    const std::size_t remaining = size_ - pos;
    const std::size_t clamped =
        count == npos ? remaining : (count > remaining ? remaining : count);
    return StringView(data_ + pos, clamped);
  }

  [[nodiscard]] constexpr std::size_t find(char c,
                                           std::size_t pos = 0) const noexcept {
    for (std::size_t i = pos; i < size_; ++i) {
      if (data_[i] == c) {
        return i;
      }
    }
    return npos;
  }

  [[nodiscard]] constexpr std::size_t find(StringView sv,
                                           std::size_t pos = 0) const noexcept {
    if (sv.size_ == 0) {
      return pos <= size_ ? pos : npos;
    }
    if (sv.size_ > size_) {
      return npos;
    }
    for (std::size_t i = pos; i + sv.size_ <= size_; ++i) {
      if (std::string_view(data_ + i, sv.size_) ==
          std::string_view(sv.data_, sv.size_)) {
        return i;
      }
    }
    return npos;
  }

  [[nodiscard]] constexpr bool operator==(StringView other) const noexcept {
    return std::string_view(data_, size_) ==
           std::string_view(other.data_, other.size_);
  }

  [[nodiscard]] constexpr auto operator<=>(StringView other) const noexcept {
    return std::string_view(data_, size_) <=>
           std::string_view(other.data_, other.size_);
  }

  [[nodiscard]] std::string toString() const {
    return std::string(data_, size_);
  }
  [[nodiscard]] constexpr operator std::string_view() const noexcept {
    return std::string_view(data_, size_);
  }

private:
  const char *data_{nullptr};
  std::size_t size_{0};
};

} // namespace dakt::core
