
#ifndef includeguard_utils_bits_includeguard
#define includeguard_utils_bits_includeguard

#include <cstdint>
#include <bitset>
#include <limits>

namespace utils
{

inline constexpr bool is_pow2 (unsigned int val) noexcept
{
  return (val & (val - 1)) == 0;
}

inline uint32_t ceil_pow2 (uint32_t val) noexcept
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

inline uint32_t floor_pow2 (uint32_t val) noexcept
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

template <typename T> inline constexpr T*
ceil_ptr (T* p, uintptr_t val) noexcept
{
  return (T*)(((uintptr_t)p + (val - 1)) & ~(val - 1));
}

template <typename T> inline constexpr bool
get_bit (T x, unsigned int n) noexcept
{
  return x & (1 << n);
}

template <std::size_t N> inline constexpr bool
get_bit (const std::bitset<N>& b, unsigned int n) noexcept
{
  return b[n];
}

template <typename T> inline constexpr T
set_bit (T x, unsigned int n, bool val = true) noexcept
{
  return (x & ~(1 << n)) | (val << n);
}

template <std::size_t N> inline constexpr
typename std::enable_if< (N <= std::numeric_limits<unsigned long>::digits),
			 std::bitset<N>>::type
set_bit (std::bitset<N> x, unsigned int n, bool val = true) noexcept
{
  auto xx = x.to_ulong ();
  return { (xx & ~((decltype (xx)(1)) << n)) | (decltype (xx)(val) << n) };
}

template <std::size_t N> inline constexpr
typename std::enable_if< (N > std::numeric_limits<unsigned long>::digits)
			 && (N <= std::numeric_limits<unsigned long long>::digits),
			 std::bitset<N>>::type
set_bit (std::bitset<N> x, unsigned int n, bool val = true) noexcept
{
  auto xx = x.to_ullong ();
  return { (xx & ~((decltype (xx)(1)) << n)) | (decltype (xx)(val) << n) };
}

template <std::size_t N> inline constexpr
typename std::enable_if< (N > std::numeric_limits<unsigned long long>::digits),
			std::bitset<N>>::type
set_bit (std::bitset<N> x, unsigned int n, bool val = true) noexcept
{
  x[n] = val;
  return x;
}


template <typename T> inline constexpr T
clear_bit (T x, unsigned int n) noexcept
{
  return set_bit (x, n, false);
}

// take 8 bits from the bitset at position 'i' and return the reversed bits.
template <std::size_t N>
inline uint8_t rev_8bits (const std::bitset<N>& bits, std::size_t i) noexcept
{
  return (bits[i + 0] << 7) | (bits[i + 1] << 6) | (bits[i + 2] << 5)
         | (bits[i + 3] << 4) | (bits[i + 4] << 3) | (bits[i + 5] << 2)
	 | (bits[i + 6] << 1) | (bits[i + 7] << 0);
}

inline constexpr uint8_t rev_8bits (uint8_t b) noexcept
{
  return (((b >> 0) & 1) << 7) | (((b >> 1) & 1) << 6) | (((b >> 2) & 1) << 5)
 	 | (((b >> 3) & 1) << 4) | (((b >> 4) & 1) << 3) | (((b >> 5) & 1) << 2)
	 | (((b >> 6) & 1) << 1) | (((b >> 7) & 1) << 0);
}

// return the reversed lower n bits of val.
template < unsigned int N, typename T >
inline constexpr T reverse_bits (T val) noexcept
{
  T out = 0;

  for (unsigned int i = 0; i < N; ++i)
    if (val & (T (1) << i))
      out |= T (1) << (N - i - 1);

  return out;
}

// merge bits A with B according the mask M.
// for each bit where M=0 the bit from A is taken.
// for each bit where M=1 the bit from B is taken.
// T can be some integer type or std::bitset
// see also https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
template < typename T > inline T
merge_bits (const T& a, const T& b, const T& m)
{
  return a ^ ((a ^ b) & m);
}

template <typename T> static inline void prefetch (const T* p)
{
  for (unsigned int i = 0; i < std::max ((size_t)1, sizeof (T) / 32); ++i)
    __builtin_prefetch ((char*)p + i*32);
}


} // namespace utils
#endif // includeguard_utils_bits_includeguard
