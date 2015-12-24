#ifndef includeguard_image_includeguard
#define includeguard_image_includeguard

#include <memory>
#include <algorithm>
#include "gl/pixel_format.hpp"

class image
{
public:
  image (void)
  : m_width (0), m_height (0), m_bytes_per_line (0), m_format (pixel_format::invalid)
  {
  }

  image (pixel_format pf, unsigned int width, unsigned int height);

  image (const image& rhs);
  image (image&& rhs);

  image& operator = (const image& rhs);
  image& operator = (image&& rhs);

  ~image (void);

  void swap (image& rhs)
  {
    std::swap (m_width, rhs.m_width);
    std::swap (m_height, rhs.m_height);
    std::swap (m_bytes_per_line, rhs.m_bytes_per_line);
    std::swap (m_format, rhs.m_format);
    std::swap (m_data, rhs.m_data);
  }

  bool empty (void) const { return m_bytes_per_line == 0 || m_height == 0; }
  unsigned int width (void) const { return m_width; }
  unsigned int height (void) const { return m_height; }
  pixel_format format (void) const { return m_format; }

  const void* data (void) const { return m_data.get (); }
  void* data (void) { return m_data.get (); }

  void* data_line (unsigned int y)
  {
    return &m_data[y * m_bytes_per_line];
  }

  const void* data_line (unsigned int y) const
  {
    return &m_data[y * m_bytes_per_line];
  }

  unsigned int data_line_size_bytes (void) const { return m_bytes_per_line; }

  void fill (uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  void fill (uint8_t l);

private:
  unsigned int m_width;
  unsigned int m_height;
  unsigned int m_bytes_per_line;
  pixel_format m_format;
  std::unique_ptr<char[]> m_data;

};

namespace std
{

template<> inline void swap (image& a, image& b)
{
  a.swap (b);
}

} // namespace std

#endif // includeguard_image_includeguard
