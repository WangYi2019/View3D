
#include <cstring>
#include <cassert>

#include "image.hpp"

#include "utils/vec_mat.hpp"
#include "utils/utils.hpp"

// https://en.wikipedia.org/wiki/Grayscale
// Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
static constexpr vec3<float> rgb_to_luma_coeffs = { 0.2126f, 0.7152f, 0.0722f };

static inline constexpr float rgb_to_luma (float r, float g, float b)
{
  return dot (vec3<float> (r, g, b), rgb_to_luma_coeffs);
}

static inline constexpr uint8_t rgb_to_luma (vec3<uint8_t> rgb)
{
// it's funny but different code is produced for open-coded dot product and
// the dot function on x86.  both are correct though.
  return dot ((vec3<uint16_t>)rgb, (vec3<uint16_t>)(rgb_to_luma_coeffs * 256)) >> 8;

//  vec3<uint16_t> c = vec3<uint16_t> (rgb_to_luma_coeffs * 256);
//  return (rgb.r * c.r + rgb.g * c.g + rgb.b * c.b) >> 8;
}

static inline constexpr vec4<uint8_t> float_to_u8 (const vec4<float>& val)
{
  vec4<float> one (1);
  vec4<float> zero (0);
  vec4<float> u8max (255);

  return (vec4<uint8_t>) std::min (std::max (std::min (one, val), zero) * 256, u8max);
}

static inline constexpr uint8_t float_to_u8 (float val)
{
  return (uint8_t) std::min (std::max (std::min (1.0f, val), 0.0f) * 256.0f, 255.0f);
}

static inline constexpr vec4<float> u8_to_float (const vec4<uint8_t>& val)
{
  return vec4<float> (val) * 1.0f/255.0f;
}

static inline constexpr float u8_to_float (uint8_t val)
{
  return val * 1.0f/255.0f;
}

// ---------------------------------------------------------------------------

template<pixel_format::fmt_t F> struct pixel_format_storage_type { };

template<> struct pixel_format_storage_type<pixel_format::rgba_8888> { typedef uint32_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgba_4444> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgba_5551> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgb_888> { typedef vec3<uint8_t> type; };
template<> struct pixel_format_storage_type<pixel_format::rgb_565> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgb_555> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgb_444> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::l_8> { typedef uint8_t type; };
template<> struct pixel_format_storage_type<pixel_format::a_8> { typedef uint8_t type; };
template<> struct pixel_format_storage_type<pixel_format::la_88> { typedef uint16_t type; };
template<> struct pixel_format_storage_type<pixel_format::rgba_f32> { typedef vec4<float> type; };

template <pixel_format::fmt_t PixelFormat>
vec4<uint8_t> unpack_pixel (typename pixel_format_storage_type<PixelFormat>::type);

template <pixel_format::fmt_t PixelFormat>
typename pixel_format_storage_type<PixelFormat>::type pack_pixel (vec4<uint8_t> p);

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgba_8888> (uint32_t p)
{
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return { (p >> 0) & 0xFF, (p >> 8) & 0xFF, (p >> 16) & 0xFF, (p >> 24) & 0xFF };
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return { (p >> 24) & 0xFF, (p >> 16) & 0xFF, (p >> 8) & 0xFF, (p >> 0) & 0xFF };
  #else
    #error unsupported endian
  #endif
}
template<> constexpr uint32_t
pack_pixel<pixel_format::rgba_8888> (vec4<uint8_t> p)
{
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (p.a << 24) | (p.b << 16) | (p.g << 8) | p.r;
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return (p.r << 24) | (p.g << 16) | (p.b << 8) | p.a;
  #else
    #error unsupported endian
  #endif
}


template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgba_4444> (uint16_t p)
{
  unsigned int r = (p >> 12) & 0xF;
  unsigned int g = (p >> 8) & 0xF;
  unsigned int b = (p >> 4) & 0xF;
  unsigned int a = (p >> 0) & 0xF;

  r = (r << 4) | r;
  g = (g << 4) | g;
  b = (b << 4) | b;
  a = (a << 4) | a;

// have to use vec4 ctor to allow folding of identity conversions.
//  uint32_t pp = (r << (0)) | (g << (8)) | (b << (16)) | (a << (24));
//  return unpack_pixel<pixel_format::rgba_8888, uint32_t> (pp | (pp << 4));
  return { r, g, b, a };

/*
  // unfortunately this generates bigger (assume slower) code than not using
  // vector extensions.
  vec4<uint8_t> u = vec4<uint8_t> { (p >> 12), (p >> 8), (p >> 4), (p >> 0) } & 15;
  return (u << 4) | u;
*/
}

template<> constexpr uint16_t
pack_pixel<pixel_format::rgba_4444> (vec4<uint8_t> p)
{
  return ((p.r >> 4) << 12) | ((p.g >> 4) << 8) | ((p.b >> 4) << 4) | ((p.a >> 4) << 0);
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgb_444> (uint16_t p)
{
  unsigned int r = (p >> 12) & 0xF;
  unsigned int g = (p >> 8) & 0xF;
  unsigned int b = (p >> 4) & 0xF;
  unsigned int a = 15;

  r = (r << 4) | r;
  g = (g << 4) | g;
  b = (b << 4) | b;
  a = (a << 4) | a;

  return { r, g, b, a };
}

template<> constexpr uint16_t
pack_pixel<pixel_format::rgb_444> (vec4<uint8_t> p)
{
  return ((p.r >> 4) << 12) | ((p.g >> 4) << 8) | ((p.b >> 4) << 4) | 0x0F;
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgba_5551> (uint16_t p)
{
  unsigned int r = (p >> (1+5+5)) & 31;
  unsigned int g = (p >> (1+5)) & 31;
  unsigned int b = (p >> (1)) & 31;
//  unsigned int a = (0 - (p & 1)) & 255;
  unsigned int a = p & 1 ? 255 : 0;  // this results in better identity conversion.

  r = (r << 3) | (r >> 2);
  g = (g << 3) | (g >> 2);
  b = (b << 3) | (b >> 2);

  return { r, g, b, a };
}

template<> constexpr uint16_t
pack_pixel<pixel_format::rgba_5551> (vec4<uint8_t> p)
{
  return ((p.r >> 3) << (1+5+5)) | ((p.g >> 3) << (1+5)) | ((p.b >> 3) << 1) | ((p.a >> 7) << 0);
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgb_555> (uint16_t p)
{
  unsigned int r = (p >> (1+5+5)) & 31;
  unsigned int g = (p >> (1+5)) & 31;
  unsigned int b = (p >> (1)) & 31;
  unsigned int a = 255;

  r = (r << 3) | (r >> 2);
  g = (g << 3) | (g >> 2);
  b = (b << 3) | (b >> 2);

  return { r, g, b, a };
}

template<> constexpr uint16_t
pack_pixel<pixel_format::rgb_555> (vec4<uint8_t> p)
{
  return ((p.r >> 3) << (1+5+5)) | ((p.g >> 3) << (1+5)) | ((p.b >> 3) << 1) | 1;
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgb_888> (vec3<uint8_t> p)
{
  return { p.r, p.g, p.b, 255 };
}

template<> constexpr vec3<uint8_t>
pack_pixel<pixel_format::rgb_888> (vec4<uint8_t> p)
{
  return { p.r, p.g, p.b };
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::rgb_565> (uint16_t p)
{
  unsigned int r = (p >> (5+6)) & 31;
  unsigned int g = (p >> 6) & 63;
  unsigned int b = p & 31;
  unsigned int a = 255;

  r = (r << 3) | (r >> 2);
  g = (g << 2) | (g >> 4);
  b = (b << 3) | (b >> 2);

  return { r, g, b, a };
}

template<> constexpr uint16_t
pack_pixel<pixel_format::rgb_565> (vec4<uint8_t> p)
{
  return ((p.r >> 3) << (5+6)) | ((p.g >> 2) << 6) | (p.b >> 3);
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::l_8> (uint8_t p)
{
  return { p, p, p, 255 };
}

template<> constexpr uint8_t
pack_pixel<pixel_format::l_8> (vec4<uint8_t> p)
{
  return rgb_to_luma (p.rgb ());
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::a_8> (uint8_t p)
{
  return { 255, 255, 255, p };
}

template<> constexpr uint8_t
pack_pixel<pixel_format::a_8> (vec4<uint8_t> p)
{
  return p.a;
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::la_88> (uint16_t p)
{
  unsigned int l = p & 255;
  unsigned int a = (p >> 8) & 255;
  return { l, l, l, a };
}

template<> constexpr uint16_t
pack_pixel<pixel_format::la_88> (vec4<uint8_t> p)
{
  return rgb_to_luma (p.rgb ()) | (p.a << 8);
}


// a generic function to convert a line of pixels from one format into another
// format using the generic vec4<uint8_t> functions.  if this is not good enough,
// a specialization for a particular combination can be implemented.
template <pixel_format::fmt_t src_format,
          pixel_format::fmt_t dst_format, typename Enable = void> struct convert_line;

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct convert_line<src_format, dst_format,
		    typename std::enable_if<
			  src_format != pixel_format::invalid
			  && src_format != pixel_format::rgba_f32
			  && dst_format != pixel_format::invalid
			  && dst_format != pixel_format::rgba_f32
			  && src_format != dst_format>::type>
{
  typedef typename pixel_format_storage_type<src_format>::type src_storage;
  typedef typename pixel_format_storage_type<dst_format>::type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
    {
      vec4<uint8_t> p = unpack_pixel<src_format> (*s++);
      *d++ = pack_pixel<dst_format> (p);
    }
  }
};

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct convert_line<src_format, dst_format,
 		    typename std::enable_if<src_format == pixel_format::invalid
					    || dst_format == pixel_format::invalid>::type>
{
  static void func (const char*, char*, unsigned int) { }
};

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct convert_line<src_format, dst_format,
		    typename std::enable_if<src_format == dst_format
					    && src_format != pixel_format::invalid
					    && dst_format != pixel_format::invalid>::type>
{
  typedef typename pixel_format_storage_type<src_format>::type src_storage;
  typedef typename pixel_format_storage_type<dst_format>::type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
      *d++ = *s++;
  }
};

template <pixel_format::fmt_t src_format>
struct convert_line<src_format, pixel_format::rgba_f32,
		    typename std::enable_if<src_format != pixel_format::invalid
					    && src_format != pixel_format::rgba_f32>::type>
{
  typedef typename pixel_format_storage_type<src_format>::type src_storage;
  typedef typename pixel_format_storage_type<pixel_format::rgba_f32>::type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
    {
      vec4<float> p = u8_to_float (unpack_pixel<src_format> (*s++));
      *d++ = p;
    }
  }
};

template <pixel_format::fmt_t dst_format>
struct convert_line<pixel_format::rgba_f32, dst_format,
		    typename std::enable_if<dst_format != pixel_format::invalid
					    && dst_format != pixel_format::rgba_f32>::type>
{
  typedef typename pixel_format_storage_type<pixel_format::rgba_f32>::type src_storage;
  typedef typename pixel_format_storage_type<dst_format>::type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
    {
      vec4<float> p = *s++;
      *d++ = pack_pixel<dst_format> (float_to_u8 (p));
    }
  }
};


// a function table for converting pixel formats.
// FIXME: this relies on the order of the pixel format enum values.
//        use compile-time sort.
static_assert (pixel_format::invalid == 0, "");
static_assert (pixel_format::rgba_8888 == 1, "");
static_assert (pixel_format::rgba_4444 == 2, "");
static_assert (pixel_format::rgba_5551 == 3, "");
static_assert (pixel_format::rgb_888 == 4, "");
static_assert (pixel_format::rgb_565 == 5, "");
static_assert (pixel_format::rgb_555 == 6, "");
static_assert (pixel_format::rgb_444 == 7, "");
static_assert (pixel_format::l_8 == 8, "");
static_assert (pixel_format::a_8 == 9, "");
static_assert (pixel_format::la_88 == 10, "");
static_assert (pixel_format::rgba_f32 == 11, "");
static_assert (pixel_format::max_count == 12, "");

typedef void (*conv_func_t)(const char* src, char* dst, unsigned int count);

static const conv_func_t conv_func_table[pixel_format::max_count][pixel_format::max_count] =
{
// 2d array initialization order: {1,2,3} = a[0][0] = 1, a[0][1] = 2, a[0][2] = 3

#define init_table_1(src_fmt, dst_fmt) convert_line<pixel_format::src_fmt, pixel_format::dst_fmt>::func

#define init_table(src_fmt) \
  init_table_1 (src_fmt, invalid), \
  init_table_1 (src_fmt, rgba_8888), \
  init_table_1 (src_fmt, rgba_4444), \
  init_table_1 (src_fmt, rgba_5551), \
  init_table_1 (src_fmt, rgb_888), \
  init_table_1 (src_fmt, rgb_565), \
  init_table_1 (src_fmt, rgb_555), \
  init_table_1 (src_fmt, rgb_444), \
  init_table_1 (src_fmt, l_8), \
  init_table_1 (src_fmt, a_8), \
  init_table_1 (src_fmt, la_88), \
  init_table_1 (src_fmt, rgba_f32),

  init_table (invalid)
  init_table (rgba_8888)
  init_table (rgba_4444)
  init_table (rgba_5551)
  init_table (rgb_888)
  init_table (rgb_565)
  init_table (rgb_555)
  init_table (rgb_444)
  init_table (l_8)
  init_table (a_8)
  init_table (la_88)
  init_table (rgba_f32)

#undef init_table_1
#undef init_table

};

// ---------------------------------------------------------------------------

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


template <typename T> static void
fill_2d (void* d, unsigned int x0, unsigned int y0,
	 unsigned int w, unsigned int h, unsigned int stride,
	 T val1)
{
  T* dst = (T*)((uintptr_t)d + y0 * stride + x0 * sizeof (T));

  for (unsigned int yy = 0; yy < h; ++yy)
  {
    for (unsigned int xx = 0; xx < w; ++xx)
      dst[xx] = val1;

    dst = (T*)((uintptr_t)dst + stride);
  }
}

void image::fill (int x, int y,
		  unsigned int width, unsigned int height,
		  float r, float g, float b, float a)
{
  if (empty ())
    return;

  const unsigned int x0 = std::max (0, x);
  const unsigned int y0 = std::max (0, y);
  const unsigned int x1 = std::min (x + (int)width, (int)m_width);
  const unsigned int y1 = std::min (y + (int)height, (int)m_height);

  const unsigned int w = x1 - x0;
  const unsigned int h = y1 - y0;

  if (w == 0 || h == 0)
    return;

  vec4<float> fill_val (r, g, b, a);

  if (m_format == pixel_format::rgba_8888)
  {
    vec4<uint8_t> val = float_to_u8 ({r, g, b, a });

    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      const uint32_t val1 = (val.a << 24) | (val.b << 16) | (val.g << 8) | val.r;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      const uint32_t val1 = (val.r << 24) | (val.g << 16) | (val.b << 8) | val.a;
    #endif

    fill_2d<uint32_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val1);
  }
  else if (m_format == pixel_format::rgba_4444)
  {
    vec4<uint8_t> val = float_to_u8 ({ r, g, b, a }) >> 4;

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val.r << 12) | (val.g << 8) | (val.b << 4) | val.a);
  }
  else if (m_format == pixel_format::rgb_444)
  {
    vec4<uint8_t> val = float_to_u8 ({ r, g, b, 1 }) >> 4;

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val.r << 12) | (val.g << 8) | (val.b << 4) | val.a);
  }
  else if (m_format == pixel_format::rgba_5551)
  {
    vec4<uint8_t> val = float_to_u8 ({ r, g, b, a }) >> 3;
    val.a = val.a >> (7-3);

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val.r << (5+5+1)) | (val.g << (5+1)) | (val.b << 1) | val.a);
  }
  else if (m_format == pixel_format::rgb_555)
  {
    vec4<uint8_t> val = float_to_u8 ({ r, g, b, 1 }) >> 3;
    val.a = val.a >> (7-3);

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val.r << (5+5+1)) | (val.g << (5+1)) | (val.b << 1) | val.a);
  }
  else if (m_format == pixel_format::rgb_565)
  {
    vec4<uint8_t> val = float_to_u8 ({ r, g, b, 1 });
    val.r = val.r >> 3;
    val.g = val.g >> 2;
    val.b = val.b >> 3;

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val.r << (5+6)) | (val.g << 5) | val.b);
  }
  else if (m_format == pixel_format::l_8)
  {
    uint8_t val = float_to_u8 (rgb_to_luma (r, g, b));

    fill_2d<uint8_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val);
  }
  else if (m_format == pixel_format::a_8)
  {
    uint8_t val = float_to_u8 (a);

    fill_2d<uint8_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val);
  }
  else if (m_format == pixel_format::la_88)
  {
    uint8_t val_l = float_to_u8 (rgb_to_luma (r, g, b));
    uint8_t val_a = float_to_u8 (a);

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val_a << 8) | val_l);
  }
  else if (m_format == pixel_format::rgb_888)
  {
    vec3<uint8_t> val = float_to_u8 ({ r, g, b, 1 }).rgb ();

    fill_2d<vec3<uint8_t>> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val);
  }
  else if (m_format == pixel_format::rgba_f32)
  {
    fill_2d<vec4<float>> (m_data.get (), x0, y0, w, h, m_bytes_per_line, { r, g, b, a });
  }
  else
    assert_unreachable ();
}

void image::copy_to (int src_x, int src_y,
		     unsigned int src_width, unsigned int src_height,
		     image& dst, int dst_x, int dst_y)
{
  if (empty () || dst.empty ())
    return;

  const unsigned int src_x0 = std::max (0, src_x);
  const unsigned int src_y0 = std::max (0, src_y);
  const unsigned int src_x1 = std::min (src_x + (int)src_width, (int)m_width);
  const unsigned int src_y1 = std::min (src_y + (int)src_height, (int)m_height);

  const unsigned int src_w = src_x1 - src_x0;
  const unsigned int src_h = src_y1 - src_y0;

  const unsigned int dst_x0 = std::max (0, dst_x);
  const unsigned int dst_y0 = std::max (0, dst_y);
  const unsigned int dst_x1 = std::min (dst_x + (int)src_width, (int)dst.width ());
  const unsigned int dst_y1 = std::min (dst_y + (int)src_height, (int)dst.height ());

  const unsigned int dst_w = dst_x1 - dst_x0;
  const unsigned int dst_h = dst_y1 - dst_y0;

  const unsigned int copy_w = std::min (src_w, dst_w);
  const unsigned int copy_h = std::min (src_h, dst_h);

  if (copy_w == 0 || copy_h == 0)
    return;

  const unsigned int src_stride = m_bytes_per_line;
  const unsigned int dst_stride = dst.bytes_per_line ();

  auto&& src_ptr = (const char*)((uintptr_t)m_data.get () + src_y0 * src_stride
				 + src_x0 * m_format.bytes_per_pixel ());
  auto&& dst_ptr = (char*)((uintptr_t)dst.data () + dst_y0 * dst_stride
			   + dst_x0 * m_format.bytes_per_pixel ());

  auto&& copy_line = conv_func_table[m_format.value ()][dst.format ().value ()];

  for (unsigned int yy = 0; yy < copy_h; ++yy)
  {
    copy_line (src_ptr, dst_ptr, copy_w);

    src_ptr += src_stride;
    dst_ptr += dst_stride;
  }
}

image image::pyr_down (down_sample_mode_t mode) const
{
  image i (m_format,
	   std::max ((unsigned int)1, m_width / 2),
	   std::max ((unsigned int)1, m_height / 2));

  

  return std::move (i);
}

