#include "common.h"

namespace Common
{

namespace Math
{

size_t ceil_pow_two(size_t n)
{
  if (n == 0)
    return 1;

  n--;

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;

  if (sizeof(n) == 8)
  {
    n |= n >> 32;
  }

  return n + 1;
}

} // namespace Math

} // namespace Common
