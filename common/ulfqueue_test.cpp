#include "catch2/catch_message.hpp"
#include <cstdio>
#include <iostream>
#define CATCH_CONFIG_MAIN
#include "ulfqueue.h"
#include <atomic>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("ULFQueue Correctness Test", "[correctness]")
{
  const std::size_t bufferSize = 1024;
  Queue::ULFQueue<int> q(bufferSize);

  REQUIRE(Queue::empty(q, 0, 0));
  REQUIRE(Queue::enqueue(q, 42));
  REQUIRE_FALSE(Queue::empty(q, 0, 1));

  int value;
  REQUIRE(Queue::dequeue(q, value));
  REQUIRE(value == 42);
  REQUIRE(Queue::empty(q, 0, 0));
}

TEST_CASE("ULFQueue Benchmark - 100 Million Operations", "[benchmark]")
{
  const std::size_t bufferSize = 100 * 1024 * 1024 / sizeof(int);

  Queue::ULFQueue<int> q(bufferSize);

  std::atomic<std::size_t> pushCount{0};
  std::atomic<std::size_t> popCount{0};
  const int totalOperations = 100'000'000;

  std::atomic<bool> done{false};
  const int pushValue = 42;

  auto pushFunc = [&]()
  {
    while (pushCount < totalOperations)
    {
      if (Queue::enqueue(q, pushValue))
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
      if (Queue::dequeue(q, popValue))
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
