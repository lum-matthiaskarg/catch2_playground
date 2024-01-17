#define CATCH_CONFIG_MAIN  
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <thread>
#include <numeric>
#include <iostream>

namespace {

int fibonacci_recursive(int n) 
{
    if (n <= 1) 
    {
        return n;
    }
   
    return fibonacci_recursive(n-1) + fibonacci_recursive(n-2);
}

int fibonacci(int n) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < n; i++) {
        int temp = a;
        a = b;
        b = temp + b;
    }
    return a;
}

class Fibonacci
{
public:
    Fibonacci()  
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ~Fibonacci() 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int calc(int n) const
    {
        return fibonacci(n);
    }
};
}


TEST_CASE("Fibonacci - test output") 
{
    const auto [n, expected] = GENERATE( table<int, int>({
        std::make_pair(0, 0),
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 2),
        std::make_pair(4, 3),
        std::make_pair(5, 5),
        std::make_pair(6, 8),
        std::make_pair(7, 13),
        std::make_pair(8, 21),
        std::make_pair(9, 34) }));

    CHECK(fibonacci_recursive(n) == expected ); 
    CHECK(fibonacci(n) == expected );
}


TEST_CASE("Fibonacci - benchmark simple function") 
{
    const int n{30};
    
    BENCHMARK("Fibonacci recursive") 
    {
        return fibonacci_recursive(n); // the return is needed to prevent the compiler from optimizing the code away"
    };

    BENCHMARK("Fibonacci iterative") 
    {
        return fibonacci(n); 
    };
}


TEST_CASE("Fibonacci - benchmark a member function")
{
    const int n{30};

    BENCHMARK("Fibonacci with constructor")
    {
        Fibonacci fib{};
        return fib.calc(n);
    };


    Fibonacci fib{};

    BENCHMARK("Fibonacci without constructor")
    {
        return fib.calc(n);
    };
}


TEST_CASE("Fibonacci - benchmark with different input")
{
    const std::vector<int> input{0, 1, 2, 3, 4, 5};

    BENCHMARK("Fibonacci with constructor", run)
    {
        return fibonacci(input.at(run%input.size()));
    };
}


TEST_CASE("Fibonacci - benchmark constructor and destructor")
{
    BENCHMARK_ADVANCED("Fibonacci constructor - benchmark advanced")(Catch::Benchmark::Chronometer meter) 
    {
        std::vector<Catch::Benchmark::storage_for<Fibonacci>> storage(meter.runs());
        meter.measure([&](int i) { storage[i].construct(); });
    };


    BENCHMARK_ADVANCED("Fibonacci destructor - benchmark advanced")(Catch::Benchmark::Chronometer meter) 
    {
        std::vector<Catch::Benchmark::destructable_object<Fibonacci>> storage(meter.runs());
        for(auto&& object : storage)
        {
            object.construct();
        }
        meter.measure([&](int i) { storage[i].destruct(); });
    };
}
