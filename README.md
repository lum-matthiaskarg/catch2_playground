# Catch2

- playground for testing catch2 features
- offical documentation: https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md


# benchmark:
- check runtime of your implemenation
- https://github.com/catchorg/Catch2/blob/devel/docs/benchmarks.md

```
> bazel run benchmark

benchmark name                       samples       iterations    estimated
                                     mean          low mean      high mean
                                     std dev       low std dev   high std dev
-------------------------------------------------------------------------------
Fibonacci recursive                            100             1    321.041 ms 
                                         3.1371 ms    3.12537 ms    3.14741 ms 
                                        56.1837 us    46.4486 us    69.5518 us 
                                                                               
Fibonacci iterative                            100           588     2.7048 ms 
                                        49.2405 ns    48.4483 ns    49.8139 ns 
                                        3.41458 ns    2.67023 ns    4.13914 ns
```

- explanation: 
  - samples: nr of benchmark runs
    - can be configured ```bazel run benchmark  -- --benchmark-samples=10```
  - iterations: calculated dynamically in the catch2 framework to get rid of system noise in the runtime measuremnt. example: clock has not enough resolution to get accurate measures for a single run 
  - estimated: total time duration the test needs
  - mean: mean duration
  - std dev: standard deviation 
- report the result for example in a xml file:
  - ```bazel run benchmark  -- -r xml > benchmark.xml```