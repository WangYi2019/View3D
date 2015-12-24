#include <ostream>
#include <cstring>
#include <cctype>

#include "utils/utils.hpp"
#include "pixel_format.hpp"
#include "gl.hpp"

pixel_format::pixel_format (const char* str)
: m_fmt (invalid)
{
  if (str == nullptr)
    return;

  while (std::isspace (*str)) ++str;

  if (*str == '\0')
    return;

  if (std::strstr (str, "RGBA_8888") != nullptr)
    m_fmt = rgba_8888;
  else if (std::strstr (str, "RGBA_4444") != nullptr)
    m_fmt = rgba_4444;
  else if (std::strstr (str, "RGBA_5551") != nullptr)
    m_fmt = rgba_5551;
  else if (std::strstr (str, "RGB_888") != nullptr)
    m_fmt = rgb_888;
  else if (std::strstr (str, "RGB_565") != nullptr)
    m_fmt = rgb_565;
  else if (std::strstr (str, "RGB_555") != nullptr)
    m_fmt = rgb_555;
  else if (std::strstr (str, "RGB_444") != nullptr)
    m_fmt = rgb_444;
  else if (std::strstr (str, "L_8") != nullptr)
    m_fmt = l_8;
  else if (std::strstr (str, "A_8") != nullptr)
    m_fmt = a_8;
  else if (std::strstr (str, "LA_88") != nullptr)
    m_fmt = la_88;
}

const char* pixel_format::str (void) const
{
  switch (m_fmt)
  {
  default:
    assert_unreachable ();

  case invalid: return "INVALID";
  case rgba_8888: return "RGBA_8888";
  case rgba_4444: return "RGBA_4444";
  case rgba_5551: return "RGBA_5551";
  case rgb_888: return "RGB_888";
  case rgb_565: return "RGB_565";
  case rgb_555: return "RGB_555";
  case rgb_444: return "RGB_444";
  case l_8: return "L_8";
  case a_8: return "A_8";
  case la_88: return "LA_88";
  }
}

unsigned int pixel_format::gl_fmt (void) const
{
  switch (m_fmt)
  {
  default:
    assert_unreachable ();

  case invalid: return 0;
  case rgba_8888: return GL_RGBA;
  case rgba_4444: return GL_RGBA;
  case rgba_5551: return GL_RGBA;
  case rgb_888: return GL_RGB;
  case rgb_565: return GL_RGB;
  case rgb_555: return GL_RGB;
  case rgb_444: return GL_RGB;
  case l_8: return GL_LUMINANCE;
  case a_8: return GL_ALPHA;
  case la_88: return GL_LUMINANCE_ALPHA;
  }
}

unsigned int pixel_format::gl_type (void) const
{
  switch (m_fmt)
  {
  default:
  case invalid: return 0;
  case rgba_8888: return GL_UNSIGNED_BYTE;
  case rgba_4444: return GL_UNSIGNED_SHORT_4_4_4_4;
  case rgba_5551: return GL_UNSIGNED_SHORT_5_5_5_1;
  case rgb_888: return GL_UNSIGNED_BYTE;
  case rgb_565: return GL_UNSIGNED_SHORT_5_6_5;
  case rgb_555: return GL_UNSIGNED_SHORT_5_5_5_1;  // not sure if that works.
  case rgb_444: return GL_UNSIGNED_SHORT_4_4_4_4;  // not sure if that works.
  case l_8: return GL_UNSIGNED_BYTE;
  case a_8: return GL_UNSIGNED_BYTE;
  case la_88: return GL_UNSIGNED_BYTE;
  }
}

unsigned int pixel_format::bits_per_pixel (void) const
{
  switch (m_fmt)
  {
  default:
  case invalid: return 0;
  case rgba_8888: return 32;
  case rgba_4444: return 16;
  case rgba_5551: return 16;
  case rgb_888: return 24;
  case rgb_565: return 16;
  case rgb_555: return 16;
  case rgb_444: return 16;
  case l_8: return 8;
  case a_8: return 8;
  case la_88: return 16;
  }
}



ds_format::ds_format (const char* str)
: m_fmt (invalid)
{
  if (str == nullptr)
    return;

  while (std::isspace (*str)) ++str;

  if (*str == '\0')
    return;

  if (std::strstr (str, "DS_24_8") != nullptr)
    m_fmt = ds_24_8;
  if (std::strstr (str, "DS_24_0") != nullptr)
    m_fmt = ds_24_0;
  if (std::strstr (str, "DS_16_8") != nullptr)
    m_fmt = ds_16_8;
  if (std::strstr (str, "DS_16_0") != nullptr)
    m_fmt = ds_16_0;
  if (std::strstr (str, "DS_0_8") != nullptr)
    m_fmt = ds_0_8;
  if (std::strstr (str, "DS_0_0") != nullptr)
    m_fmt = ds_0_0;
}

const char* ds_format::str (void) const
{
  switch (m_fmt)
  {
  default:
  case invalid: return "INVALID";
  case ds_24_8: return "DS_24_8";
  case ds_24_0: return "DS_24_0";
  case ds_16_8: return "DS_16_8";
  case ds_16_0: return "DS_16_0";
  case ds_0_8: return "DS_0_8";
  case ds_0_0: return "DS_0_0";
  }
}

std::ostream& operator << (std::ostream& out, const pixel_format& rhs)
{
  return out << rhs.str ();
}

std::ostream& operator << (std::ostream& out, const ds_format& rhs)
{
  return out << rhs.str ();
}
