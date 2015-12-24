
#include <cstring>
#include <cassert>

#include "image.hpp"

image::image (pixel_format pf, unsigned int width, unsigned int height)
{
  m_width = width;
  m_height = height;
  m_bytes_per_line = width * pf.bytes_per_pixel ();
  m_format = pf;
  m_data = std::make_unique<char[]> (m_bytes_per_line * height);
}

image::image (const image& rhs)
{
  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;

  if (!rhs.empty ())
  {
    m_data = std::make_unique<char[]> (m_bytes_per_line * m_height);
    std::memcpy (m_data.get (), rhs.data (), m_bytes_per_line * m_height);
  }
}

image::image (image&& rhs)
{
  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = std::move (rhs.m_data);

  rhs.m_width = 0;
  rhs.m_height = 0;
  rhs.m_bytes_per_line = 0;
  rhs.m_format = 0;
}

image& image::operator = (const image& rhs)
{
  if (this == &rhs)
    return *this;

  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = nullptr;

  if (!rhs.empty ())
  {
    m_data = std::make_unique<char[]> (m_bytes_per_line * m_height);
    std::memcpy (m_data.get (), rhs.data (), m_bytes_per_line * m_height);
  }

  return *this;
}

image& image::operator = (image&& rhs)
{
  if (this == &rhs)
    return *this;

  m_width = rhs.m_width;
  m_height = rhs.m_height;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = std::move (rhs.m_data);

  rhs.m_width = 0;
  rhs.m_height = 0;
  rhs.m_bytes_per_line = 0;
  rhs.m_format = 0;

  return *this;
}

image::~image (void)
{
}

void image::fill (uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  if (empty ())
    return;

  assert (m_format == pixel_format::rgba_8888);

  uint32_t* out = (uint32_t*)m_data.get ();
  const unsigned int count = m_width * m_height;

  // little endian only
  const uint32_t val = (a << 24) | (b << 16) | (g << 8) | r;

  for (unsigned int i = 0; i < count; ++i)
    *out++ = val;
}

void image::fill (uint8_t l)
{
  if (empty ())
    return;

  assert (m_format == pixel_format::l_8 || m_format == pixel_format::a_8);

  std::memset (m_data.get (), l, m_bytes_per_line * m_height);
}
