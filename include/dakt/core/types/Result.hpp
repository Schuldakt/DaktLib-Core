#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace dakt::core {

template <typename T, typename E> class Result {
public:
  using value_type = T;
  using error_type = E;

  static Result ok(T value) {
    return Result(std::in_place_index<0>, std::move(value));
  }

  static Result err(E error) {
    return Result(std::in_place_index<1>, std::move(error));
  }

  [[nodiscard]] bool isOk() const noexcept { return data_.index() == 0; }
  [[nodiscard]] bool isErr() const noexcept { return data_.index() == 1; }

  [[nodiscard]] T &value() & { return std::get<0>(data_); }
  [[nodiscard]] const T &value() const & { return std::get<0>(data_); }
  [[nodiscard]] T &&value() && { return std::move(std::get<0>(data_)); }

  [[nodiscard]] E &error() & { return std::get<1>(data_); }
  [[nodiscard]] const E &error() const & { return std::get<1>(data_); }
  [[nodiscard]] E &&error() && { return std::move(std::get<1>(data_)); }

  template <typename F>
  auto map(F &&f) -> Result<std::invoke_result_t<F, T>, E> {
    using U = std::invoke_result_t<F, T>;
    if (isOk()) {
      return Result<U, E>::ok(std::invoke(std::forward<F>(f), value()));
    }
    return Result<U, E>::err(error());
  }

  template <typename F> auto andThen(F &&f) -> std::invoke_result_t<F, T> {
    using Ret = std::invoke_result_t<F, T>;
    if (isOk()) {
      return std::invoke(std::forward<F>(f), value());
    }
    return Ret::err(error());
  }

  template <typename F>
  auto orElse(F &&f) -> Result<T, std::invoke_result_t<F, E>> {
    using Err = std::invoke_result_t<F, E>;
    if (isErr()) {
      return Result<T, Err>::err(std::invoke(std::forward<F>(f), error()));
    }
    return Result<T, Err>::ok(value());
  }

  [[nodiscard]] T valueOr(T defaultVal) const {
    if (isOk()) {
      return std::get<0>(data_);
    }
    return defaultVal;
  }

  explicit operator bool() const noexcept { return isOk(); }

private:
  template <std::size_t I, typename... Args>
  explicit Result(std::in_place_index_t<I> index, Args &&...args)
      : data_(index, std::forward<Args>(args)...) {}

  std::variant<T, E> data_;
};

template <typename E> class Result<void, E> {
public:
  using error_type = E;

  static Result ok() { return Result(true, std::nullopt); }
  static Result err(E error) { return Result(false, std::move(error)); }

  [[nodiscard]] bool isOk() const noexcept { return ok_; }
  [[nodiscard]] bool isErr() const noexcept { return !ok_; }

  [[nodiscard]] E &error() & { return *error_; }
  [[nodiscard]] const E &error() const & { return *error_; }
  [[nodiscard]] E &&error() && { return std::move(*error_); }

  template <typename F>
  auto orElse(F &&f) -> Result<void, std::invoke_result_t<F, E>> {
    using Err = std::invoke_result_t<F, E>;
    if (isErr()) {
      return Result<void, Err>::err(std::invoke(std::forward<F>(f), *error_));
    }
    return Result<void, Err>::ok();
  }

  explicit operator bool() const noexcept { return ok_; }

private:
  explicit Result(bool ok, std::optional<E> err)
      : ok_(ok), error_(std::move(err)) {}

  bool ok_{false};
  std::optional<E> error_{};
};

} // namespace dakt::core
