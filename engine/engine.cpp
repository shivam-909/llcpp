#include "../common/common.h"
#include <__chrono/duration.h>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <thread>

Common::Queue::UQueue<size_t> q(10 * 1024 * 1024);

void pushOneHundredMillion()
{
  for (size_t i = 0; i < 3 * 1e9; i++)
  {
    Common::Queue::enqueue(q, i);
  }
}

void readOneHundredMillion()
{
  while (Common::Queue::empty(q))
  {
    ;
  }

  for (size_t i = 0; i < 1e9; i++)
  {
    size_t x;
    Common::Queue::dequeue(q, x);
  }
}

int main()
{
  std::chrono::high_resolution_clock::time_point c =
      std::chrono::high_resolution_clock::now();

  std::thread t1(pushOneHundredMillion);

  std::thread t2(readOneHundredMillion);
  std::thread t3(readOneHundredMillion);
  std::thread t4(readOneHundredMillion);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  std::chrono::high_resolution_clock::time_point e =
      std::chrono::high_resolution_clock::now();

  auto d = std::chrono::duration_cast<std::chrono::seconds>(e - c).count();

  std::cout << d << " seconds" << std::endl;

  return 0;
}
