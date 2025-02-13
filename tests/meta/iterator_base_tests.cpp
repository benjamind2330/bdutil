
#include <gtest/gtest.h>

#include <iterator>
#include <ranges>
#include <utility>

#include "bdutil/meta/iterator_base.hpp"

using namespace bd;

enum class city : int {
  brisbane,
  sydney,
  melbourne,
  hobart,
  perth,
  adelaide,
  darwin,
  canberra
};

std::string to_string(city c) {
  switch (c) {
    case city::brisbane:
      return "brisbane";
    case city::sydney:
      return "sydney";
    case city::melbourne:
      return "melbourne";
    case city::hobart:
      return "hobart";
    case city::perth:
      return "perth";
    case city::adelaide:
      return "adelaide";
    case city::darwin:
      return "darwin";
    case city::canberra:
      return "canberra";
  }
}

class citirator : public iterator_base<citirator> {
 public:
  citirator() = default;
  citirator(city c) : m_pos{std::to_underlying(c)} {}
  city dereference() const { return static_cast<city>(m_pos); }
  void advance(std::ptrdiff_t off) { m_pos += off; }
  std::ptrdiff_t distance_to(const citirator& other) const {
    return other.m_pos - m_pos;
  }

 private:
  int m_pos = 0;
};

class city_range {
 public:
  citirator begin() const { return citirator{}; }
  citirator end() const {
    return citirator{static_cast<city>(std::to_underlying(city::canberra) + 1)};
  }
};

static_assert(std::input_iterator<citirator>);
static_assert(std::forward_iterator<citirator>);
static_assert(std::bidirectional_iterator<citirator>);
static_assert(std::random_access_iterator<citirator>);
static_assert(std::ranges::range<city_range>);

// Demonstrate some basic assertions.
TEST(CiteratorTest, Check) {
  for (auto c : city_range{}) {
    std::cout << to_string(c) << "\n";
  }

  for (auto c : city_range{} | std::views::take(1)) {
    std::cout << "Best city is " << to_string(c) << "\n";
  }
}