
#ifndef includeguard_utils_math_includeguard
#define includeguard_utils_math_includeguard

#include <cstdint>

namespace utils
{

template <typename T> inline T deg_to_rad (T val)
{
  const T pi (3.14159265358979323846);

  return val * T (pi / 180);
}

} // namespace utils
#endif // includeguard_utils_math_includeguard
