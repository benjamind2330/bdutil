#pragma once

#include <concepts>
#include <iterator>

namespace bd {

namespace traits {

template <typename T>
struct iter_value_type {
  static const T& it;
  using type = std::remove_cvref_t<decltype(*it)>;
};

template <typename T>
  requires requires(T it) { typename T::value_type; }
struct iter_value_type<T> {
  using type = T::value_type;
};

template <typename T>
using iter_value_type_t = iter_value_type<T>::type;

template <typename>
struct iter_difference_type {
  using type = std::ptrdiff_t;
};

template <typename T>
  requires requires(T it) {
    { it.distance_to(it) };
  }
struct iter_difference_type<T> {
  static const T& it;
  using type = decltype(it.distance_to(it));
};

template <typename T>
using iter_difference_type_t = iter_difference_type<T>::type;

}  // namespace traits

namespace concepts {

template <typename T>
concept has_distance_to = requires(const T it) {
  { it.distance_to(it) };
};

template <typename T>
concept has_increment = requires(T t) {
  { t.increment() };
};

template <typename T>
concept has_decrement = requires(T t) {
  { t.decrement() };
};

template <typename T>
concept has_advance =
    requires(T it, const traits::iter_difference_type_t<T> offset) {
      { it.advance(offset) };
    };

template <typename T>
concept has_equal_to = requires(const T it) {
  { it.equal_to(it) } -> std::convertible_to<bool>;
};

template <typename Arg, typename T>
concept difference_type_arg =
    std::convertible_to<Arg, traits::iter_difference_type_t<T>>;

}  // namespace concepts

template <typename t_child>
struct iterator_base {
  using self_type = t_child;

  // Input iterator
  decltype(auto) operator*() const { return self().dereference(); }

  auto operator->() const {
    decltype(auto) ref = **this;
    if constexpr (std::is_reference_v<decltype(ref)>) {
      return std::addressof(ref);
    } else {
      return arrow_proxy<decltype(ref)>{std::move(ref)};
    }
  }

  self_type& operator++() {
    if constexpr (concepts::has_increment<self_type>) {
      self().increment();
    } else {
      static_assert(concepts::has_advance<self_type>,
                    "Iterator subclass must provide either "
                    ".advance() or .increment()");
      self() += 1;
    }
    return self();
  }

  self_type operator++(int) {
    auto copy = self();
    ++*this;
    return copy;
  }

  // Forward iterator

  friend bool operator==(const self_type& left, const self_type& right) {
    if constexpr (concepts::has_equal_to<self_type>) {
      return left.equal_to(right);
    } else {
      static_assert(concepts::has_distance_to<self_type>,
                    "Iterator must provide `.equal_to()` "
                    "or `.distance_to()`");
      return left.distance_to(right) == 0;
    }
  }

  // Bidirectional Iterator

  self_type& operator--() {
    if constexpr (concepts::has_decrement<self_type>) {
      self().decrement();
    } else {
      static_assert(concepts::has_advance<self_type>,
                    "Iterator subclass must provide either "
                    ".advance() or .decrement()");
      self() -= 1;
    }
    return self();
  }

  self_type operator--(int) {
    auto copy = *this;
    --*this;
    return copy;
  }

  // Random access Iterator
  friend self_type& operator+=(
      self_type& self, concepts::difference_type_arg<self_type> auto offset)
    requires concepts::has_advance<self_type>
  {
    self.advance(offset);
    return self;
  }

  friend self_type operator+(self_type left,
                             concepts::difference_type_arg<self_type> auto off)
    requires concepts::has_advance<self_type>
  {
    return left += off;
  }

  friend self_type operator+(concepts::difference_type_arg<self_type> auto off,
                             self_type right)
    requires concepts::has_advance<self_type>
  {
    return right += off;
  }

  friend self_type operator-(self_type left,
                             concepts::difference_type_arg<self_type> auto off)
    requires concepts::has_advance<self_type>
  {
    return left + -off;
  }

  friend self_type& operator-=(
      self_type& left, concepts::difference_type_arg<self_type> auto off)
    requires concepts::has_advance<self_type>
  {
    return left = left - off;
  }

  decltype(auto) operator[](
      concepts::difference_type_arg<self_type> auto off) const
    requires concepts::has_advance<self_type>
  {
    return *(self() + off);
  }

  friend auto operator-(const self_type& left, const self_type& right)
    requires concepts::has_distance_to<self_type>
  {
    return right.distance_to(left);
  }

  friend auto operator<=>(const self_type& left, const self_type& right)
    requires concepts::has_distance_to<self_type>
  {
    return (left - right) <=> 0;
  }

 private:
  template <class t_ref>
  struct arrow_proxy {
    t_ref r;
    t_ref* operator->() { return &r; }
  };

  self_type& self() { return static_cast<self_type&>(*this); }
  const self_type& self() const { return static_cast<const self_type&>(*this); }
};

namespace categories {

template <typename T>
concept random_access =
    concepts::has_advance<T> && concepts::has_distance_to<T>;

template <typename T>
concept bidirectional = random_access<T> || concepts::has_decrement<T>;

}  // namespace categories

}  // namespace bd

template <typename t_iter>
  requires std::is_base_of_v<bd::iterator_base<t_iter>, t_iter>
struct std::iterator_traits<t_iter> {
  static const t_iter& it;
  using reference = decltype(*it);
  using pointer = decltype(it.operator->());
  using value_type = bd::traits::iter_value_type_t<t_iter>;
  using difference_type = bd::traits::iter_difference_type_t<t_iter>;

  // clang-format off
  using iterator_category = 
    std::conditional_t<
      bd::categories::random_access<t_iter>, 
      std::random_access_iterator_tag,
      std::conditional_t<
        bd::categories::bidirectional<t_iter>,
        std::bidirectional_iterator_tag,
        std::conditional_t<
          bd::concepts::has_equal_to<t_iter>,
          std::forward_iterator_tag,
          std::input_iterator_tag
        >
      >
    >;
  // clang-format on

  // Just set this to the iterator_category, for now
  using iterator_concept = iterator_category;
};