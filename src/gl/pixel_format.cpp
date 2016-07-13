#include <ostream>
#include <cstring>
#include <cctype>

#include "pixel_format.hpp"
#include "gl.hpp"

struct pixel_format_desc
{
  pixel_format fmt;
  int8_t r_bits;
  int8_t g_bits;
  int8_t b_bits;
  int8_t a_bits;
  uint8_t rgb_bits;
  uint8_t bpp;
  uint16_t gl_type;
  uint16_t gl_fmt;
  uint16_t gl_internal_fmt;
  const char* str;
};

// FIXME: use compile time sort to ensure that this table can be indexed by
// the enum value.
static const pixel_format_desc pf_table[] =
{
  { pixel_format::invalid,    0,  0,  0,  0,    0,   0,  0, 0, 0, "INVALID" },
  { pixel_format::rgba_8888,  8,  8,  8,  8,   24,   32, GL_UNSIGNED_BYTE,          GL_RGBA,            GL_RGBA,            "RGBA_8888" },
  { pixel_format::rgba_4444,  4,  4,  4,  4,   12,   16, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA,            GL_RGBA,            "RGBA_4444" },
  { pixel_format::rgba_5551,  5,  5,  5,  1,   15,   16, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA,            GL_RGBA,            "RGBA_5551" },
  { pixel_format::rgb_888,    8,  8,  8,  0,   24,   24, GL_UNSIGNED_BYTE,          GL_RGB,             GL_RGB,             "RGB_888" },
  { pixel_format::rgb_565,    5,  6,  5,  0,   16,   16, GL_UNSIGNED_SHORT_5_6_5,   GL_RGB,             GL_RGB,             "RGB_565" },
  { pixel_format::rgb_555,    5,  5,  5,  0,   15,   16, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA,            GL_RGBA,            "RGB_555" },
  { pixel_format::rgb_444,    4,  4,  4,  0,   12,   16, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA,            GL_RGBA,            "RGB_444" },
  { pixel_format::l_8,        0,  0,  0,  0,    0,    8, GL_UNSIGNED_BYTE,          GL_LUMINANCE,       GL_LUMINANCE,       "L_8" },
  { pixel_format::a_8,        0,  0,  0,  8,    0,    8, GL_UNSIGNED_BYTE,          GL_ALPHA,           GL_ALPHA,           "A_8" },
  { pixel_format::la_88,      0,  0,  0,  8,    0,   16, GL_UNSIGNED_BYTE,          GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, "LA_88" },
  { pixel_format::rgba_f32,  32, 32, 32, 32, 32*3, 32*4, GL_FLOAT,                  GL_RGBA,            GL_RGBA,            "RGBA_32F" },
  { pixel_format::rgb_332,    3,  3,  2,  0,    8,    8, GL_UNSIGNED_BYTE_3_3_2,    GL_RGB,             GL_RGB,             "RGB_332" },
  { pixel_format::bgr_888,    8,  8,  8,  0,   24,   24, GL_UNSIGNED_BYTE,          GL_BGR,             GL_RGB,             "BGR_888" },
};

pixel_format::pixel_format (const char* str)
: m_fmt (invalid)
{
  if (str == nullptr)
    return;

  while (std::isspace (*str)) ++str;

  if (*str == '\0')
    return;

  for (auto&& t : pf_table)
    if (std::strstr (str, t.str) == str)
    {
      m_fmt = t.fmt.value ();
      break;
    }
}

const char* pixel_format::str (void) const { return pf_table[value ()].str; }
unsigned int pixel_format::bits_per_pixel (void) const { return pf_table[value ()].bpp; }
unsigned int pixel_format::r_bits (void) const { return pf_table[value ()].r_bits; }
unsigned int pixel_format::g_bits (void) const { return pf_table[value ()].g_bits; }
unsigned int pixel_format::b_bits (void) const { return pf_table[value ()].b_bits; }
unsigned int pixel_format::rgb_bits (void) const { return pf_table[value ()].rgb_bits; }
unsigned int pixel_format::a_bits (void) const { return pf_table[value ()].a_bits; }
unsigned int pixel_format::gl_fmt (void) const { return pf_table[value ()].gl_fmt; }
unsigned int pixel_format::gl_internal_fmt (void) const { return pf_table[value ()].gl_internal_fmt; }
unsigned int pixel_format::gl_type (void) const { return pf_table[value ()].gl_type; }

struct ds_format_desc
{
  ds_format fmt;
  int8_t d_bits;
  int8_t s_bits;
  const char* str;
};

// FIXME: use compile time sort to ensure that this table can be indexed by
// the enum value.
static const ds_format_desc ds_table[] =
{
  { ds_format::invalid,  0, 0, "INVALID" },
  { ds_format::ds_24_8, 24, 8, "DS_24_8" },
  { ds_format::ds_24_0, 24, 0, "DS_24_0" },
  { ds_format::ds_16_8, 16, 8, "DS_16_8" },
  { ds_format::ds_0_8,   0, 8, "DS_0_8" },
  { ds_format::ds_0_0,   0, 0, "DS_0_0" }
};

ds_format::ds_format (const char* str)
: m_fmt (invalid)
{
  if (str == nullptr)
    return;

  while (std::isspace (*str)) ++str;

  if (*str == '\0')
    return;

  for (auto&& t : ds_table)
    if (std::strstr (str, t.str) == str)
    {
      m_fmt = t.fmt;
      break;
    }
}

const char* ds_format::str (void) const { return ds_table[value ()].str; }
unsigned int ds_format::d_bits (void) const { return ds_table[value ()].d_bits; }
unsigned int ds_format::s_bits (void) const { return ds_table[value ()].s_bits; }

std::ostream& operator << (std::ostream& out, const pixel_format& rhs)
{
  return out << rhs.str ();
}

std::ostream& operator << (std::ostream& out, const ds_format& rhs)
{
  return out << rhs.str ();
}
