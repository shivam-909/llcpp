#ifndef ULF_QUEUE_H
#define ULF_QUEUE_H

#include "common.h"
#include "queue.h"
#include <__atomic/atomic.h>
#include <atomic>
#include <cstddef>
#include <memory>

namespace Queue
{

// Unidirectional lock-free FIFO queue
template <typename T, typename Alloc = std::allocator<T>> struct ULFQueue
{
  using t_alloc = Alloc;
  using t_value = T;
  using t_ptr = typename std::allocator_traits<Alloc>::pointer;

  t_alloc allocator;

  std::size_t capacity;

  t_ptr ring;

  alignas(128) std::atomic<std::size_t> h_cursor;
  alignas(128) std::atomic<std::size_t> t_cursor;
  static_assert(std::atomic<std::size_t>::is_always_lock_free);

  char padding[128 - sizeof(std::size_t)];

  ULFQueue(std::size_t cap, const Alloc &alloc = Alloc())
      : allocator(alloc), capacity(Common::Math::ceil_pow_two(cap)),
        ring(std::allocator_traits<Alloc>::allocate(
            allocator, capacity)), // can't use original cap since capacity has
                                   // been bounded upwards
        h_cursor(0), t_cursor(0)
  {
  }

  ~ULFQueue()
  {
    if (ring)
    {
      std::allocator_traits<Alloc>::deallocate(allocator, ring, capacity);
    }
  }

  ULFQueue(const ULFQueue &) = delete;
  ULFQueue &operator=(const ULFQueue &) = delete;
};

template <typename T, typename Alloc = std::allocator<T>>
bool enqueue(ULFQueue<T, Alloc> &q, const T &v)
{
  size_t t_cursor = q.t_cursor.load(std::memory_order_relaxed);
  size_t h_cursor = q.h_cursor.load(std::memory_order_acquire);

  if (full(q, h_cursor, t_cursor))
  {
    return false;
  }

  new (elem(q, t_cursor)) T(v);
  q.t_cursor.store(t_cursor + 1, std::memory_order_release);

  return true;
}

template <typename T, typename Alloc = std::allocator<T>>
bool dequeue(ULFQueue<T, Alloc> &q, T &v)
{

  size_t t_cursor = q.t_cursor.load(std::memory_order_relaxed);
  size_t h_cursor = q.h_cursor.load(std::memory_order_acquire);

  if (empty(q, h_cursor, t_cursor))
  {
    return false;
  }

  v = *elem(q, h_cursor);
  elem(q, h_cursor)->~T();
  q.h_cursor.store(h_cursor + 1, std::memory_order_release);

  return true;
}

template <typename T, typename Alloc = std::allocator<T>>
bool empty(const ULFQueue<T, Alloc> &q, std::size_t h_cursor,
           std::size_t t_cursor)
{
  return size(q, h_cursor, t_cursor) == 0;
}

template <typename T, typename Alloc = std::allocator<T>>
bool full(const ULFQueue<T, Alloc> &q, std::size_t h_cursor,
          std::size_t t_cursor)
{
  return (t_cursor - h_cursor) == q.capacity;
}

template <typename T, typename Alloc = std::allocator<T>>
size_t size(const ULFQueue<T, Alloc> &q, std::size_t h_cursor,
            std::size_t t_cursor)
{
  return t_cursor - h_cursor;
}

template <typename T, typename Alloc = std::allocator<T>>
T *elem(const ULFQueue<T, Alloc> &q, std::size_t cursor)
{
  return &q.ring[NEXT_CUR(cursor, q.capacity)];
}

} // namespace Queue
#endif
