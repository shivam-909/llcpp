#include "catch2/catch_message.hpp"
#include <cstdio>
#include <iostream>
#define CATCH_CONFIG_MAIN
#include "common.h"
#include <atomic>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("UQueue Correctness Test", "[correctness]")
{
  const std::size_t bufferSize = 1024;
  Common::Queue::UQueue<int> q(bufferSize);

  REQUIRE(Common::Queue::empty(q));
  REQUIRE(Common::Queue::enqueue(q, 42));
  REQUIRE_FALSE(Common::Queue::empty(q));

  int value;
  REQUIRE(Common::Queue::dequeue(q, value));
  REQUIRE(value == 42);
  REQUIRE(Common::Queue::empty(q));
}

TEST_CASE("UQueue Benchmark - 100 Million Operations", "[benchmark]")
{
  const std::size_t bufferSize = 100 * 1024 * 1024 / sizeof(int);
  Common::Queue::UQueue<int> q(bufferSize);

  std::atomic<std::size_t> pushCount{0};
  std::atomic<std::size_t> popCount{0};
  const int totalOperations = 100'000'000;

  std::atomic<bool> done{false};
  const int pushValue = 42;

  auto pushFunc = [&]()
  {
    while (pushCount < totalOperations)
    {
      if (Common::Queue::enqueue(q, pushValue))
      {
        ++pushCount;
      }
    }
    done = true;
  };

  auto popFunc = [&]()
  {
    int popValue;
    while (!done || popCount < pushCount)
    {
      if (Common::Queue::dequeue(q, popValue))
      {
        ++popCount;
      }
    }
  };

  BENCHMARK("100 Million Push and Pop Operations")
  {
    std::thread pushThread(pushFunc);
    std::thread popThread(popFunc);

    pushThread.join();
    popThread.join();

    return pushCount.load() + popCount.load();
  };
}
