#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"
#include "queue.h"
#include <__atomic/aliases.h>
#include <cstddef>
#include <memory>
#include <mutex>

namespace Queue
{

// Unidirectional lock-based FIFO queue
template <typename T, typename Alloc = std::allocator<T>> struct UQueue
{
  using t_alloc = Alloc;
  using t_value = T;
  using t_ptr = typename std::allocator_traits<Alloc>::pointer;

  t_alloc allocator;

  std::size_t capacity;

  t_ptr ring;

  std::mutex h_lock;
  std::mutex t_lock;

  alignas(128) std::size_t h_cursor;
  alignas(128) std::size_t t_cursor;

  char padding[128 - sizeof(std::size_t)];

  UQueue(std::size_t cap, const Alloc &alloc = Alloc())
      : allocator(alloc), capacity(Common::Math::ceil_pow_two(cap)),
        ring(std::allocator_traits<Alloc>::allocate(
            allocator, capacity)), // can't use original cap since capacity has
                                   // been bounded upwards
        h_cursor(0), t_cursor(0)
  {
  }

  ~UQueue()
  {
    if (ring)
    {
      std::allocator_traits<Alloc>::deallocate(allocator, ring, capacity);
    }
  }

  UQueue(const UQueue &) = delete;
  UQueue &operator=(const UQueue &) = delete;
};

template <typename T, typename Alloc = std::allocator<T>>
bool enqueue(UQueue<T, Alloc> &q, const T &v)
{
  if (full(q))
  {
    return false;
  }

  std::lock_guard<std::mutex> lock(q.t_lock);
  new (&q.ring[NEXT_CUR(q.t_cursor, q.capacity)]) T(v);
  q.t_cursor++;

  return true;
}

template <typename T, typename Alloc = std::allocator<T>>
bool dequeue(UQueue<T, Alloc> &q, T &v)
{
  if (empty(q))
  {
    return false;
  }
  std::lock_guard<std::mutex> lock(q.h_lock);
  v = q.ring[NEXT_CUR(q.h_cursor, q.capacity)];
  std::allocator_traits<Alloc>::destroy(q.allocator, &q.ring[q.h_cursor]);
  q.h_cursor++;
  return true;
}

template <typename T, typename Alloc = std::allocator<T>>
bool empty(const UQueue<T, Alloc> &q)
{
  return size(q) == 0;
}

template <typename T, typename Alloc = std::allocator<T>>
bool full(const UQueue<T, Alloc> &q)
{
  return size(q) == q.capacity;
}

template <typename T, typename Alloc = std::allocator<T>>
size_t size(const UQueue<T, Alloc> &q)
{
  return q.h_cursor - q.t_cursor;
}

// Bidirectional lock-free FIFO queue
template <typename T> struct BLFQueue
{
};
} // namespace Queue

#endif
