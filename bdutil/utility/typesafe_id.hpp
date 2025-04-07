///@file
#pragma once

#include <concepts>
#include <limits>
#include <optional>

namespace bd {

struct nullid_t {};
inline constexpr nullid_t nullid{};

template <class IdName, typename T, T Invalid = std::numeric_limits<T>::max()>
  requires std::integral<T>
class typesafe_id {
 public:
  using value_type = T;

  constexpr typesafe_id() = default;
  constexpr explicit typesafe_id(value_type id) : _id{id} {}

  constexpr typesafe_id(nullid_t) {}
  constexpr typesafe_id& operator=(nullid_t) {
    _id = Invalid;
    return *this;
  }

  [[nodiscard]] constexpr T value() const { return _id; }
  [[nodiscard]] constexpr explicit operator value_type() const { return _id; }

  [[nodiscard]] constexpr bool is_valid() const { return _id != Invalid; }
  [[nodiscard]] constexpr explicit operator bool() const { return is_valid(); }

  template <class F>
  constexpr typesafe_id or_else(F&& f) const {
    if (!is_valid()) {
      return std::invoke(std::forward<F>(f));
    }
    return *this;
  }

  template <class F>
  constexpr auto transform(F&& f) const -> std::optional<
      std::remove_cvref_t<std::invoke_result_t<F, decltype(*this)>>> {
    if (is_valid()) return std::invoke(std::forward<F>(f), *this);
    return std::nullopt;
  }

  friend bool operator==(const typesafe_id&, const typesafe_id&) = default;

 private:
  value_type _id{Invalid};
};

}  // namespace bd