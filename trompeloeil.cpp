#define CATCH_CONFIG_MAIN  
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <optional>

using trompeloeil::_;

struct Trajectory
{
  int id{};
};

struct Collision
{
  float x{};
  float y{};
};

class ICollisionCalculator
{
public:
  ICollisionCalculator() = default;
  virtual ~ICollisionCalculator() = default;
  ICollisionCalculator(const ICollisionCalculator&) = delete;
  ICollisionCalculator& operator=(const ICollisionCalculator&) = delete;
  ICollisionCalculator(ICollisionCalculator&&) = delete;
  ICollisionCalculator& operator=(ICollisionCalculator&&) = delete;

  virtual std::optional<Collision> calc(const Trajectory& trajectory_0, const Trajectory& trajectory_1) = 0;
};

std::vector<Trajectory> readObjectTrajectories()
{
  return std::vector<Trajectory>{{1}, {3}, {5}};
} 

// -----------------------------------------------------------------------------------------------------------
// 1. two ways how to mock a class
// -----------------------------------------------------------------------------------------------------------
struct CollisionCalculator
{
  MAKE_MOCK2(calc, std::optional<Collision>(const Trajectory& trajectory_0, const Trajectory& trajectory_1));
};

class MockCollisionCalculator : public trompeloeil::mock_interface<ICollisionCalculator> 
{
public:
  IMPLEMENT_MOCK2(calc);
};

// don't mix it
// struct MockCollisionCalculator : public ICollisionCalculator
// {
//   MAKE_MOCK2(calc, std::optional<Collision>(const Trajectory& trajectory_0, const Trajectory& trajectory_1));
// };

TEST_CASE("Mock CollisionCalculator with/out interface")
{
  Trajectory ego_trajectory{};
  Trajectory object_trajectory{};

  SECTION("without interface - can be used for template parameters")
  {
    CollisionCalculator collision_calculator;
    REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
    auto collision = collision_calculator.calc(ego_trajectory, object_trajectory);
    REQUIRE(collision);
    CHECK( collision.value().x == 1.0F );
    CHECK( collision.value().y == 1.0F );
  }

  SECTION("with interface")
  {
    MockCollisionCalculator collision_calculator;
    REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{2.0F, 2.0F}});
    auto collision = collision_calculator.calc(ego_trajectory, object_trajectory);
    REQUIRE(collision);
    CHECK( collision.value().x == 2.0F );
    CHECK( collision.value().y == 2.0F );
  }
}

// -----------------------------------------------------------------------------------------------------------
// 2. how to use it by dependency injection
// -----------------------------------------------------------------------------------------------------------
class CollisionAlgoRef
{
  public:
  CollisionAlgoRef(ICollisionCalculator& collision_calculator) 
  : collision_calculator_{collision_calculator} 
  {}
  
  std::vector<Collision> run(const Trajectory& ego_trajectory)
  {
    std::vector<Collision> collisions;
    for (const auto& object_trajectory : readObjectTrajectories())
    {
      auto collision = collision_calculator_.calc(ego_trajectory, object_trajectory);
      if (collision)
      {
        collisions.push_back(collision.value());
      }
    } 
    return collisions;
  }

  private:
  ICollisionCalculator& collision_calculator_;
};

class CollisionAlgoPtr
{
  public:
  CollisionAlgoPtr(std::unique_ptr<ICollisionCalculator> collision_calculator) 
  : collision_calculator_{std::move(collision_calculator)} 
  {}
  
  std::vector<Collision> run(const Trajectory& ego_trajectory)
  {
    std::vector<Collision> collisions;
    for (const auto& object_trajectory : readObjectTrajectories())
    {
      auto collision = collision_calculator_->calc(ego_trajectory, object_trajectory);
      if (collision)
      {
        collisions.push_back(collision.value());
      }
    } 
    return collisions;
  }

  private:
  std::unique_ptr<ICollisionCalculator> collision_calculator_;
};


TEST_CASE("mocking with depencency injection")
{
  SECTION("dependency injection by reference")
  {
    MockCollisionCalculator collision_calculator;
    CollisionAlgoRef collision_algo{collision_calculator};    
    ALLOW_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
    const auto collisions = collision_algo.run(Trajectory{});
    REQUIRE(collisions.size() == 3);
  }
  SECTION("dependency injection by pointer")
  {
    auto collision_calculator = std::make_unique<MockCollisionCalculator>();
    auto& collision_calculator_mock = *collision_calculator;
    CollisionAlgoPtr collision_algo{std::move(collision_calculator)};    
    ALLOW_CALL(collision_calculator_mock, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
    const auto collisions = collision_algo.run(Trajectory{});
    REQUIRE(collisions.size() == 3);
  }
}

// -----------------------------------------------------------------------------------------------------------
// 3. what about free functions
// free functions can not be mocked directly, so they need to be wrapped
// usecase: strategy pattern
// check if it is really worth to wrap it or is a lambda function enough
// -----------------------------------------------------------------------------------------------------------
using CollisionStrategy = std::function<std::optional<Collision>(const Trajectory&, const Trajectory&)>;

bool hasCollision(Trajectory trajectory_0, Trajectory trajectory_1, CollisionStrategy& strategy)
{
  auto collision = strategy(trajectory_0, trajectory_1);
  return collision.has_value();
}

class CollisionStrategyMock
{
  MAKE_MOCK2(calc, std::optional<Collision>(const Trajectory& trajectory_0, const Trajectory& trajectory_1));
};


TEST_CASE("free function")
{
  CollisionStrategyMock collision_strategy_mock;
  CollisionStrategy strategy = [&](const Trajectory& trajectory_0, const Trajectory& trajectory_1)
  {
    return collision_strategy_mock.calc(trajectory_0, trajectory_1);
  };

  REQUIRE_CALL(collision_strategy_mock, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
  CHECK(hasCollision(Trajectory{}, Trajectory{}, strategy));
}


// -----------------------------------------------------------------------------------------------------------
// 4. what can I do with the mock
// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-setting-expectations
// you can add expectations to the mock
// - how often a function is called
// - with which parameters
// - what should be returned
// -----------------------------------------------------------------------------------------------------------
TEST_CASE("Expectations")
{
  MockCollisionCalculator collision_calculator;
  std::optional<Collision> collision{{1.0F, 1.0F}};

  SECTION("ALLOW_CALL")
  {
    ALLOW_CALL(collision_calculator, calc(_, _)).RETURN(collision);
    collision_calculator.calc(Trajectory{}, Trajectory{});
  }

  SECTION("FORBID_CALL")
  {
    FORBID_CALL(collision_calculator, calc(_, _));
  }

  SECTION("REQUIRE_CALL")
  {
    REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(collision);
    collision_calculator.calc(Trajectory{}, Trajectory{});
  }
}

TEST_CASE("REQUIRE CALL - number of calls")
{
  MockCollisionCalculator collision_calculator;
  Trajectory expectedTrajectory{1};
  std::optional<Collision> collision{{1.0F, 1.0F}};

  SECTION("different number of calls")
  {
    {
      REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(collision);
      collision_calculator.calc(expectedTrajectory, Trajectory{});
    }
    {
      REQUIRE_CALL(collision_calculator, calc(_, _)).TIMES(2).RETURN(collision);
      collision_calculator.calc(expectedTrajectory, Trajectory{});
      collision_calculator.calc(expectedTrajectory, Trajectory{});

    }
    {
      REQUIRE_CALL(collision_calculator, calc(_, _)).TIMES(AT_LEAST(2)).RETURN(collision);
      collision_calculator.calc(expectedTrajectory, Trajectory{});
      collision_calculator.calc(expectedTrajectory, Trajectory{});
      collision_calculator.calc(expectedTrajectory, Trajectory{});
    }
    {
      REQUIRE_CALL(collision_calculator, calc(_, _)).TIMES(AT_MOST(2)).RETURN(collision);
      collision_calculator.calc(expectedTrajectory, Trajectory{});
      collision_calculator.calc(expectedTrajectory, Trajectory{});
    }
  }
}

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-matching-values-with-conditions
bool operator==(const Trajectory& lhs, const Trajectory& rhs)
{
  return lhs.id == rhs.id;
}

bool operator!=(const Trajectory& lhs, const Trajectory& rhs)
{
  return !(lhs == rhs);
}

TEST_CASE("REQUIRE CALL - matching-values-with-conditions")
{
  MockCollisionCalculator collision_calculator;
  Trajectory expectedTrajectory{1};
  std::optional<Collision> collision{{1.0F, 1.0F}};

  SECTION("parameters checks - eq, ne, gt, ge, lt, le")
  {
    {
      REQUIRE_CALL(collision_calculator, calc(expectedTrajectory, _)).RETURN(collision);
      REQUIRE_CALL(collision_calculator, calc(trompeloeil::eq(expectedTrajectory), _)).RETURN(collision);
      collision_calculator.calc(expectedTrajectory, Trajectory{});
      collision_calculator.calc(expectedTrajectory, Trajectory{});
    }
    {
      REQUIRE_CALL(collision_calculator, calc(trompeloeil::ne(expectedTrajectory), _)).RETURN(collision);
      collision_calculator.calc(Trajectory{2}, Trajectory{});
    }
  }


  SECTION("more complex parameter checks")
  {
    REQUIRE_CALL(collision_calculator, calc(_,_)).WITH(_1.id == expectedTrajectory.id).RETURN(collision);
    collision_calculator.calc(Trajectory{1}, Trajectory{});
  }

  SECTION("stack requirements")
  {
    trompeloeil::stream_tracer tracer{std::cout};
    ALLOW_CALL(collision_calculator, calc(_,_)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
    REQUIRE_CALL(collision_calculator, calc(Trajectory{2},_)).TIMES(2).RETURN(std::optional<Collision>{{2.0F, 2.0F}});
    REQUIRE_CALL(collision_calculator, calc(Trajectory{2},_)).RETURN(std::optional<Collision>{{3.0F, 3.0F}});

    const auto collision_0 = collision_calculator.calc(Trajectory{2}, Trajectory{});
    CHECK(collision_0->x == 3.0F);
    CHECK(collision_0->y == 3.0F);
    const auto collision_1 = collision_calculator.calc(Trajectory{2}, Trajectory{});
    CHECK(collision_1->x == 2.0F);
    CHECK(collision_1->y == 2.0F);
    const auto collision_2 = collision_calculator.calc(Trajectory{2}, Trajectory{});
    CHECK(collision_2->x == 2.0F);
    CHECK(collision_2->y == 2.0F);
    const auto collision_3 = collision_calculator.calc(Trajectory{2}, Trajectory{});
    CHECK(collision_3->x == 1.0F);
    CHECK(collision_3->y == 1.0F);
  }
}



// -----------------------------------------------------------------------------------------------------------
// 5. lifecycle of mock
// -----------------------------------------------------------------------------------------------------------

void set_default_requirements(MockCollisionCalculator& collision_calculator)
{
  REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
}

std::unique_ptr<trompeloeil::expectation> create_default_requirements(MockCollisionCalculator& collision_calculator)
{
  return NAMED_REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
}

TEST_CASE("Lifecycle")
{
  MockCollisionCalculator collision_calculator;

  SECTION("will this work?")
  {
    // {
    //   REQUIRE_CALL(collision_calculator, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});
    // }
    // collision_calculator.calc(Trajectory{}, Trajectory{});
  }

  SECTION("will this work?")
  {
    // set_default_requirements(collision_calculator);
    // collision_calculator.calc(Trajectory{}, Trajectory{});
  }

  SECTION("named requirement")
  {
    std::unique_ptr<trompeloeil::expectation> expectation = create_default_requirements(collision_calculator);
    collision_calculator.calc(Trajectory{}, Trajectory{});
  }
}

class TestFixture
{
public:
  TestFixture()
  {
    default_expectation_ = NAMED_ALLOW_CALL(collision_calculator_, calc(_, _)).RETURN(std::optional<Collision>{{1.0F, 1.0F}});    
  }
  MockCollisionCalculator collision_calculator_;
  std::unique_ptr<trompeloeil::expectation> default_expectation_{}; 
};


TEST_CASE_METHOD(TestFixture, "default expectation")
{
  const auto collision_0 = collision_calculator_.calc(Trajectory{}, Trajectory{});
  const auto collision_1 = collision_calculator_.calc(Trajectory{}, Trajectory{});
  CHECK( collision_0->x == 1.0F );
  CHECK( collision_1->x == 1.0F );
}

TEST_CASE_METHOD(TestFixture, "change default expectation")
{
  default_expectation_ = NAMED_ALLOW_CALL(collision_calculator_, calc(_, _)).RETURN(std::optional<Collision>{{2.0F, 2.0F}});
  const auto collision = collision_calculator_.calc(Trajectory{}, Trajectory{});
  CHECK( collision->x == 2.0F );
}


// follow up...

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-define-side-effects-for-matching-calls

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-return-references-from-matching-calls

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-throwing-exceptions-from-matching-calls

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-expecting-several-matching-calls-in-some-sequences

// https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#-writing-custom-matchers