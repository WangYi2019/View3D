
#include <cstring>
#include <cassert>

#include "image.hpp"

#include "utils/vec_mat.hpp"
#include "utils/utils.hpp"

// https://en.wikipedia.org/wiki/Grayscale
// Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
static constexpr vec3<float> rgb_to_luma_coeffs = { 0.2126f, 0.7152f, 0.0722f };

static inline constexpr float rgb_to_luma (vec3<float> rgb)
{
  return dot (rgb, rgb_to_luma_coeffs);
}

static inline constexpr uint8_t rgb_to_luma (vec3<uint8_t> rgb)
{
// it's funny but different code is produced for open-coded dot product and
// the dot function on x86.  both are correct though.
  return dot ((vec3<uint16_t>)rgb, (vec3<uint16_t>)(rgb_to_luma_coeffs * 256)) >> 8;

//  vec3<uint16_t> c = vec3<uint16_t> (rgb_to_luma_coeffs * 256);
//  return (rgb.r * c.r + rgb.g * c.g + rgb.b * c.b) >> 8;
}

static inline vec4<uint8_t> float_to_u8 (const vec4<float>& val)
{
  vec4<float> one (1);
  vec4<float> zero (0);
  vec4<float> u8max (255);

  return (vec4<uint8_t>) std::min (std::max (std::min (one, val), zero) * 256, u8max);
}

static inline constexpr vec4<float> u8_to_float (const vec4<uint8_t>& val)
{
  return vec4<float> (val) * 1.0f/255.0f;
}

// ---------------------------------------------------------------------------

template <pixel_format::fmt_t F> struct format_traits { };

template<> struct format_traits<pixel_format::rgba_8888>
{
  typedef uint32_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgba_4444>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgba_5551>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgb_888>
{
  typedef vec3<uint8_t> storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgb_565>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgb_555>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgb_444>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::l_8>
{
  typedef uint8_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::a_8>
{
  typedef uint8_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::la_88>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
};

template<> struct format_traits<pixel_format::rgba_f32>
{
  typedef vec4<float> storage_type;
  typedef vec4<float> unpacked_type;
};

template <typename S, typename D> D convert_unpacked (S);

template<> vec4<uint8_t> convert_unpacked<vec4<uint8_t>, vec4<uint8_t>> (vec4<uint8_t> s) { return s; }
template<> vec4<float>   convert_unpacked<vec4<uint8_t>, vec4<float>>   (vec4<uint8_t> s) { return u8_to_float (s); }
template<> vec4<uint8_t> convert_unpacked<vec4<float>,   vec4<uint8_t>> (vec4<float> s) { return float_to_u8 (s); }
template<> vec4<float>   convert_unpacked<vec4<float>,   vec4<float>>   (vec4<float> s) { return s; }


template <pixel_format::fmt_t F>
typename format_traits<F>::unpacked_type unpack_pixel (typename format_traits<F>::storage_type);

template <pixel_format::fmt_t F>
typename format_traits<F>::storage_type pack_pixel (typename format_traits<F>::unpacked_type);

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


template<> vec4<uint8_t>
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

template<> vec4<uint8_t>
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

template<> vec4<uint8_t>
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

template<> vec4<uint8_t>
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

template<> vec4<uint8_t>
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
  return ((p.r >> 3) << (5+6)) | ((p.g >> 2) << 5) | (p.b >> 3);
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

template<> vec4<uint8_t>
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

template<> constexpr vec4<float>
unpack_pixel<pixel_format::rgba_f32> (vec4<float> p)
{
  return p;
}

template<> constexpr vec4<float>
pack_pixel<pixel_format::rgba_f32> (vec4<float> p)
{
  return p;
}


template<pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
typename format_traits<dst_format>::storage_type
repack_pixel (typename format_traits<src_format>::storage_type p)
{
  return pack_pixel<dst_format> (
	convert_unpacked<typename format_traits<src_format>::unpacked_type,
			 typename format_traits<dst_format>::unpacked_type> (
		unpack_pixel<src_format> (p)));
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
			  && dst_format != pixel_format::invalid
			  && src_format != dst_format>::type>
{
  typedef typename format_traits<src_format>::storage_type src_storage;
  typedef typename format_traits<dst_format>::storage_type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
      *d++ = repack_pixel<src_format, dst_format> (*s++);
  }
};

// invalid formats do nothing.
template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct convert_line<src_format, dst_format,
 		    typename std::enable_if<src_format == pixel_format::invalid
					    || dst_format == pixel_format::invalid>::type>
{
  static void func (const char*, char*, unsigned int) { }
};

// converting between the same formats is a simple memcpy.
template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct convert_line<src_format, dst_format,
		    typename std::enable_if<src_format == dst_format
					    && src_format != pixel_format::invalid
					    && dst_format != pixel_format::invalid>::type>
{
  typedef typename format_traits<src_format>::storage_type src_storage;
  typedef typename format_traits<dst_format>::storage_type dst_storage;

  static void __attribute__((flatten))
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
      *d++ = *s++;
  }
};

// generic 2D fill function
template <pixel_format::fmt_t PixelFormat> void
fill_2d (void* d, unsigned int x0, unsigned int y0,
	 unsigned int w, unsigned int h, unsigned int stride,
	 const vec4<float>& fill_val)
{
  typedef typename format_traits<PixelFormat>::storage_type packed_pixel;

  auto&& dst = (packed_pixel*)((uintptr_t)d + y0 * stride + x0 * sizeof (packed_pixel));
  auto val = repack_pixel<pixel_format::rgba_f32, PixelFormat> (fill_val);

  for (unsigned int yy = 0; yy < h; ++yy)
  {
    for (unsigned int xx = 0; xx < w; ++xx)
      dst[xx] = val;

    dst = (packed_pixel*)((uintptr_t)dst + stride);
  }
}

template<> void
fill_2d<pixel_format::invalid> (void*, unsigned int, unsigned int,
				unsigned int, unsigned int, unsigned int,
				const vec4<float>&)
{
}



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

// a function table for 2D filling a constant color value.
typedef void (*fill_func_t)(void* d, unsigned int x0, unsigned int y0,
			    unsigned int w, unsigned int h, unsigned int stride,
			    const vec4<float>& fill_val);

static const fill_func_t fill_func_table[pixel_format::max_count] =
{
#define init_table(fmt) fill_2d<pixel_format :: fmt>,

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

#undef init_table
};

// ---------------------------------------------------------------------------

image::image (pixel_format pf, const vec2<unsigned int>& size)
{
  m_size = size;
  m_bytes_per_line = size.x * pf.bytes_per_pixel ();
  m_format = pf;
  m_data = std::make_unique<char[]> (m_bytes_per_line * size.y);
}

image::image (const image& rhs)
{
  m_size = rhs.m_size;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;

  if (!rhs.empty ())
  {
    m_data = std::make_unique<char[]> (m_bytes_per_line * m_size.y);
    std::memcpy (m_data.get (), rhs.data (), m_bytes_per_line * m_size.y);
  }
}

image::image (image&& rhs)
{
  m_size = rhs.m_size;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = std::move (rhs.m_data);

  rhs.m_size = { 0, 0 };
  rhs.m_bytes_per_line = 0;
  rhs.m_format = 0;
}

image& image::operator = (const image& rhs)
{
  if (this == &rhs)
    return *this;

  m_size = rhs.m_size;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = nullptr;

  if (!rhs.empty ())
  {
    m_data = std::make_unique<char[]> (m_bytes_per_line * m_size.y);
    std::memcpy (m_data.get (), rhs.data (), m_bytes_per_line * m_size.y);
  }

  return *this;
}

image& image::operator = (image&& rhs)
{
  if (this == &rhs)
    return *this;

  m_size = rhs.m_size;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;
  m_data = std::move (rhs.m_data);

  rhs.m_size = { 0, 0 };
  rhs.m_bytes_per_line = 0;
  rhs.m_format = 0;

  return *this;
}

image::~image (void)
{
}

void image::fill (const vec2<int>& xy, const vec2<unsigned int>& size,
		  const vec4<float>& rgba_value)
{
  if (empty ())
    return;

  const vec2<unsigned int> tl (std::max (vec2<int> (0), xy));
  const vec2<unsigned int> br (std::min (xy + (vec2<int>)size, (vec2<int>)m_size));
  const vec2<unsigned int> copy_size = br - tl;

  if (copy_size.x == 0 || copy_size.y == 0)
    return;

  fill_func_table[m_format.value ()] (m_data.get (), tl.x, tl.y,
				      copy_size.x, copy_size.y,
				      m_bytes_per_line, rgba_value);
}

image::copy_to_result
image::copy_to (const vec2<int>& src_xy, const vec2<unsigned int>& src_size,
		image& dst, const vec2<int>& dst_xy) const
{
  if (empty () || dst.empty ())
    return { };

  // tl = top-left coordinate, br = bottom-right coordinate

  const vec2<unsigned int> src_tl (std::max (vec2<int> (0), src_xy));
  const vec2<unsigned int> src_br (std::min (src_xy + (vec2<int>)src_size, (vec2<int>)size ()));

  const vec2<unsigned int> dst_tl (std::max (vec2<int> (0), dst_xy));
  const vec2<unsigned int> dst_br (std::min (dst_xy + (vec2<int>)src_size, (vec2<int>)dst.size ()));

  const auto src_copy_sz = src_br - src_tl;
  const auto dst_copy_sz = dst_br - dst_tl;

  const auto copy_sz = std::min (src_copy_sz, dst_copy_sz);

  if (copy_sz.x == 0 || copy_sz.y == 0)
    return { };

  const unsigned int src_stride = m_bytes_per_line;
  const unsigned int dst_stride = dst.bytes_per_line ();

  auto&& src_ptr = (const char*)((uintptr_t)m_data.get () + src_tl.y * src_stride
				 + src_tl.x * m_format.bytes_per_pixel ());
  auto&& dst_ptr = (char*)((uintptr_t)dst.data () + dst_tl.y * dst_stride
			   + dst_tl.x * m_format.bytes_per_pixel ());

  auto&& copy_line = conv_func_table[m_format.value ()][dst.format ().value ()];

  for (unsigned int yy = 0; yy < copy_sz.y; ++yy)
  {
    copy_line (src_ptr, dst_ptr, copy_sz.x);

    src_ptr += src_stride;
    dst_ptr += dst_stride;
  }

  return { src_tl, dst_tl, copy_sz };
}

image image::pyr_down (down_sample_mode_t mode) const
{
  image i (m_format, std::max (vec2<unsigned int> (1), m_size / 2));

  

  return std::move (i);
}

