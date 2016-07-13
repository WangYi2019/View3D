
#ifndef includeguard_utils_hpp_includeguard
#define includeguard_utils_hpp_includeguard

#include <cstdint>
#include <algorithm>

#define countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define assert_unreachable() do { assert ("unreachable" && 0); } while (1)

#define likely(expr) __builtin_expect (expr, 1)
#define unlikely(expr) __builtin_expect (expr, 0)


template <typename T> static inline T* ceil_ptr (T* p, uintptr_t val)
{
  return (T*)(((uintptr_t)p + (val - 1)) & ~(val - 1));
}

template <typename T> static inline void prefetch (const T* p)
{
  for (unsigned int i = 0; i < std::max ((size_t)1, sizeof (T) / 32); ++i)
    __builtin_prefetch ((char*)p + i*32);
}


inline uint32_t ceil_pow2 (uint32_t val) throw ()
{
   // for details see
   // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
   val--;
   val = (val >> 1) | val;
   val = (val >> 2) | val;
   val = (val >> 4) | val;
   val = (val >> 8) | val;
   val = (val >> 16) | val;
   val++;
   return val;
}

inline uint32_t floor_pow2 (uint32_t val) throw ()
{
   // from http://stackoverflow.com/questions/2679815/previous-power-of-2
   if (0 == val)
      return 0;

   val |= (val >> 1);
   val |= (val >> 2);
   val |= (val >> 4);
   val |= (val >> 8);
   val |= (val >> 16);
   return val - (val >> 1);
}

#endif // includeguard_utils_hpp_includeguard
