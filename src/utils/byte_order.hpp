#ifndef includeguard_utils_byte_order_includeguard
#define includeguard_utils_byte_order_includeguard

#include <cstdint>
#include <type_traits>

namespace utils
{

enum byte_order_t
{
  little_endian = 0,
  big_endian = 1
};

inline constexpr byte_order_t native_byte_order (void)
{
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return little_endian;
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return big_endian;
  #else
    #error unsupported byte order
  #endif
}


// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67625
// can't use __builtin_bswap{16|32} directly in constexpr returns.  have to
// ferry it through a constexpr struct.
namespace bswap_impl
{

template <std::size_t SZ, typename T> struct bswapped_sz;

struct enabled { };

template <typename T> struct bswapped_sz<1, T >
  : private std::enable_if<std::is_integral<T>::value, enabled>::type
{
  const T val;
  constexpr bswapped_sz (T v) : val (v) { }
};

template <typename T> struct bswapped_sz<2, T >
  : private std::enable_if<std::is_integral<T>::value, enabled>::type
{
  const T val;
  constexpr bswapped_sz (T v) : val (__builtin_bswap16 (v)) { }
};

template <typename T> struct bswapped_sz<4, T >
  : private std::enable_if<std::is_integral<T>::value, enabled>::type
{
  const T val;
  constexpr bswapped_sz (T v) : val (__builtin_bswap32 (v)) { }
};

template <typename T> struct bswapped_sz<8, T >
  : private std::enable_if<std::is_integral<T>::value, enabled>::type
{
  const T val;
  constexpr bswapped_sz (T v) : val (__builtin_bswap64 (v)) { }
};


} // namespace bswap_impl


template <typename T>
inline constexpr T bswap (T val) { return bswap_impl::bswapped_sz<sizeof (T), T> (val).val; }

template <typename T> inline constexpr T native_to (byte_order_t bo, const T& val)
{
  return bo == native_byte_order () ? val : bswap (val);
}

template <typename T> inline constexpr T to_native (byte_order_t bo, const T& val)
{
  return bo == native_byte_order () ? val : bswap (val);
}

} // namespace utils
#endif // includeguard_utils_byte_order_includeguard
