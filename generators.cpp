#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <tuple>
#include <vector>

namespace {

TEST_CASE("Generators") {
  auto i = GENERATE(1, 2);
  auto j = GENERATE(1, 3, 5);
  // WARN("i: " << i << " j: " << j);
}

TEST_CASE("table") {
  const auto test_table =
      GENERATE(table<int, std::string>({{1, "two"}, {3, "four"}, {5, "five"}}));
  // WARN("value: " << std::get<0>(test_table)
  //                << " string: " << std::get<1>(test_table));
}

TEST_CASE("filter") {
  // filter(predicate, GeneratorWrapper<T>&&) for FilterGenerator<T, Predicate>
  const auto i =
      GENERATE(filter([](const int i) { return i == 42; }, values({42, 43})));
  // WARN("i: " << i);
}

TEST_CASE("take") {
  // take is used to limit the number of values produced by a generator
  const auto i = GENERATE(take(2, values({1, 2, 3})));
  // WARN("i: " << i);
}

TEST_CASE("repeat") {
  // repeat is used to repeat the sequence of values produced by a generator
  const auto i = GENERATE(repeat(2, values({1, 2, 3})));
  // WARN("i: " << i);
}

TEST_CASE("map") {
  // map is used to apply a transformation function to each value produced by a
  // generator
  const double i = GENERATE(map([](int i) { return i * i + 0.1; },
                                values({1, 2, 3}))); // maps from int to double
  // WARN("i: " << i);
}

TEST_CASE("chunk") {
  // chunk is used to group values produced by a generator into chunks of a
  // specified size
  auto test_chunk =
      GENERATE(chunk(2, values({1, 2, 3, 4, 5}))); // 5 will be ignored
  REQUIRE(test_chunk.size() == 2);
  // WARN("0: " << test_chunk.at(0) << " 1: " << test_chunk.at(1));
}

TEST_CASE("range") {
  // range is used to create a generator that generates a range of values from a
  // start value to an end value, with a custom step size [a, b[ with step size
  // s
  const auto i = GENERATE(range(0, 10, 2));
  // WARN("i: " << i);
}

const std::vector<int> values = {1, 2, 3};
std::vector<int> create_values() { return {1, 2, 3}; }
TEST_CASE("from_range") {
  // from_range is used to create a generator from a range specified by a
  // container
  const auto i = GENERATE(from_range(values));
  const auto j = GENERATE(from_range(values.cbegin(), values.cend()));
  const auto k = GENERATE(from_range(create_values()));
  // WARN("i: " << i << " j: " << j << " k: " << k);
}

} // namespace