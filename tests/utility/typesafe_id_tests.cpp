

#include <gtest/gtest.h>

#include "bdutil/utility/typesafe_id.hpp"

using test_id = bd::typesafe_id<struct test_id_tag, int>;

TEST(TypesafeId, CreateInvalid) {
  test_id id{bd::nullid};
  EXPECT_FALSE(id.is_valid());
  EXPECT_EQ(id.value(), std::numeric_limits<int>::max());
  EXPECT_FALSE(id);
}

TEST(TypesafeId, CreateValid) {
  test_id id{1};
  EXPECT_TRUE(id.is_valid());
  EXPECT_EQ(id.value(), 1);
  EXPECT_TRUE(id);
  EXPECT_EQ(static_cast<int>(id), 1);
}

TEST(TypesafeId, ValidityComparisons) {
  test_id id1{1};
  test_id id2{1};
  test_id id3{2};
  test_id id4{};

  EXPECT_NE(id1, id3);
  EXPECT_EQ(id1, id2);
  EXPECT_EQ(id1, id1);
  EXPECT_NE(id1, id4);

  EXPECT_EQ(id4, bd::nullid);
  EXPECT_NE(id2, bd::nullid);
}

TEST(TypesafeId, MonadicTransform) {
  test_id id{1};
  auto result = id.transform([](test_id id) { return id.value() + 1; });
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 2);

  test_id id2{};
  auto result2 = id2.transform([](test_id id) { return id.value() + 1; });
  EXPECT_FALSE(result2.has_value());
}

TEST(TypesafeId, MonadicOrElse) {
  test_id id{1};
  auto result = id.or_else([] { return test_id{2}; });
  EXPECT_EQ(result, id);

  test_id id2{};
  auto result2 = id2.or_else([] { return test_id{2}; });
  EXPECT_EQ(result2.value(), 2);
}