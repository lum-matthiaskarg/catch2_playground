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
  //WARN("value: " << std::get<0>(test_table)
  //               << " string: " << std::get<1>(test_table));
}

TEST_CASE("filter") {
  // filter(predicate, GeneratorWrapper<T>&&) for FilterGenerator<T, Predicate>
  const auto i =
      GENERATE(filter([](const int i) { return i == 42; }, values({42, 43})));
  //WARN("i: " << i);
}



TEST_CASE("take") {
  // take is used to limit the number of values produced by a generator
  const auto i =
      GENERATE(take(2, values({1,2,3})));
  //WARN("i: " << i);
}

TEST_CASE("repeat") {
  // repeat is used to repeat the sequence of values produced by a generator
  const auto i =
      GENERATE(repeat(2, values({1,2,3})));
  //WARN("i: " << i);
}

TEST_CASE("map") {
  // map is used to apply a transformation function to each value produced by a generator
  const double i = GENERATE(map([](int i) { return i * i + 0.1; }, values({1, 2, 3})));   // maps from int to double
  WARN("i: " << i);
}



} // namespace