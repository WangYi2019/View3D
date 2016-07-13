
#include <cstring>
#include <cassert>

#include "image.hpp"

#include "utils/vec_mat.hpp"
#include "utils/utils.hpp"

#include "utils/pp_for_each.hpp"

using utils::vec2;
using utils::vec3;
using utils::vec4;

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

static inline vec4<float> u8_to_float (const vec4<uint8_t>& val)
{
  return vec4<float> (val) * 1.0f/255.0f;
}

// ---------------------------------------------------------------------------

template <pixel_format::fmt_t F> struct format_traits { };

template<> struct format_traits<pixel_format::rgba_8888>
{
  typedef uint32_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgba_4444>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgba_5551>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgb_888>
{
  typedef vec3<uint8_t> storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgb_565>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgb_555>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgb_444>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::l_8>
{
  typedef uint8_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::a_8>
{
  typedef uint8_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::la_88>
{
  typedef uint16_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgba_f32>
{
  typedef vec4<float> storage_type;
  typedef vec4<float> unpacked_type;
  typedef vec4<float> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::rgb_332>
{
  typedef uint8_t storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
};

template<> struct format_traits<pixel_format::bgr_888>
{
  typedef vec3<uint8_t> storage_type;
  typedef vec4<uint8_t> unpacked_type;
  typedef vec4<uint16_t> unpacked_widened_type;
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

template<> vec4<uint8_t>
unpack_pixel<pixel_format::rgb_332> (uint8_t p)
{
  unsigned int r = (p >> (3+2)) & 7;
  unsigned int g = (p >> 2) & 7;
  unsigned int b = p & 3;
  unsigned int a = 255;

  r = (r << 5) | (r << 2) | (r >> 1);
  g = (g << 5) | (g << 2) | (g >> 1);
  b = (b << 2) | (b << 4) | (b << 6) | b;

  return { r, g, b, a };
}

template<> uint8_t
pack_pixel<pixel_format::rgb_332> (vec4<uint8_t> p)
{
  return ((p.r >> 5) << (3+2)) | ((p.g >> 5) << 2) | (p.b >> 6);
}

template<> constexpr vec4<uint8_t>
unpack_pixel<pixel_format::bgr_888> (vec3<uint8_t> p)
{
  return { p.b, p.g, p.r, 255 };
}

template<> constexpr vec3<uint8_t>
pack_pixel<pixel_format::bgr_888> (vec4<uint8_t> p)
{
  return { p.b, p.g, p.r };
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

// ---------------------------------------------------------------------------

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

#ifdef __GNUC__
  static void __attribute__((flatten))
#else
  static void
#endif
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

#ifdef __GNUC__
  static void __attribute__((flatten))
#else
  static void
#endif
  func (const char* src, char* dst, unsigned int count)
  {
    auto&& s = (const src_storage*)src;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < count; ++xx)
      *d++ = *s++;
  }
};

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

// generic function to pyr-down a line of pixels (one destination line,
// two source lines).

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format,
	  typename Enable = void> struct pyr_down_line;

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct pyr_down_line<src_format, dst_format,
		     typename std::enable_if<
			src_format != pixel_format::invalid
			&& dst_format != pixel_format::invalid>::type>
{
  typedef typename format_traits<src_format>::storage_type src_storage;
  typedef typename format_traits<dst_format>::storage_type dst_storage;
  typedef typename format_traits<src_format>::unpacked_widened_type src_widened;
  typedef typename format_traits<dst_format>::unpacked_type dst_unpacked;

#ifdef __GNUC__
  static void __attribute__((flatten))
#else
  static void
#endif
  func (const char* src0, const char* src1, char* dst, unsigned int dst_count)
  {
    auto&& s0 = (const src_storage*)src0;
    auto&& s1 = (const src_storage*)src1;
    auto&& d = (dst_storage*)dst;

    for (unsigned int xx = 0; xx < dst_count; ++xx)
    {
      auto p0 = unpack_pixel<src_format> (*s0++);
      auto p1 = unpack_pixel<src_format> (*s0++);
      auto p2 = unpack_pixel<src_format> (*s1++);
      auto p3 = unpack_pixel<src_format> (*s1++);

      // determine the promoted type of the calculation, which is needed
      // to avoid overflow.  for integer types could also do the calculation
      // with the narrow type, by first shifting right, summing up and adding
      // a rounding term of the lower bits that have been shifted out.  but
      // that's more complicated than simply using the wider type.
      //
      // could do this like that:
      //   typedef typename decltype (p0)::element_type narrow_type;
      //   typedef decltype (narrow_type (1) + narrow_type (1)) widened_type;
      // but then uint8_t gets promoted to int and that is too wide.
      // we want uint8_t -> uint16_t widening in this case.

      decltype (p0) p_avg ((src_widened (p0) + src_widened (p1) + src_widened (p2)
			   + src_widened (p3)) / 4);

      *d++ = pack_pixel<dst_format> (
	convert_unpacked<decltype (p_avg), dst_unpacked> (p_avg));
    }
  }
};

template <pixel_format::fmt_t src_format, pixel_format::fmt_t dst_format>
struct pyr_down_line<src_format, dst_format,
		     typename std::enable_if<
			src_format == pixel_format::invalid
			|| dst_format == pixel_format::invalid>::type>
{
  static void func (const char*, const char*, char*, unsigned int) { }
};


// ---------------------------------------------------------------------------

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
static_assert (pixel_format::rgb_332 == 12, "");
static_assert (pixel_format::bgr_888 == 13, "");
static_assert (pixel_format::max_count == 14, "");

#define all_pixel_formats \
  invalid, \
  rgba_8888, \
  rgba_4444, \
  rgba_5551, \
  rgb_888, \
  rgb_565, \
  rgb_555, \
  rgb_444, \
  l_8, \
  a_8, \
  la_88, \
  rgba_f32, \
  rgb_332, \
  bgr_888

typedef void (*conv_func_t)(const char* src, char* dst, unsigned int count);

static const conv_func_t conv_func_table[pixel_format::max_count][pixel_format::max_count] =
{
// 2d array initialization order: {1,2,3} = a[0][0] = 1, a[0][1] = 2, a[0][2] = 3

// generate a cartesian product for all pixel formats with the order { src fmt, dst fmt }
#define init_table_1(src_fmt, dst_fmt) convert_line<pixel_format::src_fmt, pixel_format::dst_fmt>::func,
#define init_table(src_fmt) pp_for_each_i (init_table_1, src_fmt, all_pixel_formats)

  pp_for_each (init_table, all_pixel_formats)

#undef init_table_1
#undef init_table

};

// fill 2D function table
typedef void (*fill_func_t)(void* d, unsigned int x0, unsigned int y0,
			    unsigned int w, unsigned int h, unsigned int stride,
			    const vec4<float>& fill_val);

static const fill_func_t fill_func_table[pixel_format::max_count] =
{
#define init_table(fmt) fill_2d<pixel_format :: fmt>,

  pp_for_each (init_table, all_pixel_formats)

#undef init_table
};


// pyr down function table
typedef void (*pyr_down_func_t)(const char*, const char*, char*, unsigned int);

static const pyr_down_func_t pyr_down_func_table[pixel_format::max_count][pixel_format::max_count] =
{
#define init_table_1(src_fmt, dst_fmt) pyr_down_line<pixel_format::src_fmt, pixel_format::dst_fmt>::func,
#define init_table(src_fmt) pp_for_each_i (init_table_1, src_fmt, all_pixel_formats)

  pp_for_each (init_table, all_pixel_formats)

#undef init_table_1
#undef init_table

};


// ---------------------------------------------------------------------------

image::image (pixel_format pf, const vec2<unsigned int>& size)
{
  m_size = size;
  m_bytes_per_line = (unsigned int)align_stride (size.x * pf.bytes_per_pixel ());
  m_format = pf;
  m_data = data_buffer (m_bytes_per_line * size.y);
}

image::image (const image& rhs)
{
  m_size = rhs.m_size;
  m_bytes_per_line = rhs.m_bytes_per_line;
  m_format = rhs.m_format;

  if (!rhs.empty ())
  {
    m_data = data_buffer (m_bytes_per_line * m_size.y);
    std::memcpy (m_data.ptr, rhs.data (), m_bytes_per_line * m_size.y);
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
    m_data = data_buffer (m_bytes_per_line * m_size.y);
    std::memcpy (m_data.ptr, rhs.data (), m_bytes_per_line * m_size.y);
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

image::fill_result
image::fill (const vec2<int>& xy, const vec2<unsigned int>& size,
	     const vec4<float>& rgba_value)
{
  if (empty ())
    return { };

  const vec2<unsigned int> tl (std::min (std::max (vec2<int> (0), xy), (vec2<int>)m_size));
  const vec2<unsigned int> br (std::min (std::max (vec2<int> (0), xy + (vec2<int>)size), (vec2<int>)m_size));
  const vec2<unsigned int> fill_size = br - tl;

  if (fill_size.x == 0 || fill_size.y == 0)
    return { };

  fill_func_table[m_format.value ()] (m_data.ptr, tl.x, tl.y,
				      fill_size.x, fill_size.y,
				      m_bytes_per_line, rgba_value);

  return { tl, fill_size };
}

image::copy_to_result
image::copy_to (const vec2<int>& src_xy, const vec2<unsigned int>& src_size,
		image& dst, const vec2<int>& dst_xy) const
{
  if (empty () || dst.empty ())
    return { };

  // tl = top-left coordinate, br = bottom-right coordinate
  // if the size is numeric_limits::max, the calculations will wrap around and
  // go bad.  catch this by limiting the size early.
  const auto use_src_size = std::min (src_size, size ());

  const vec2<unsigned int> src_tl (std::min (std::max (vec2<int> (0), src_xy), (vec2<int>)size ()));
  const vec2<unsigned int> src_br (std::min (std::max (vec2<int> (0), src_xy + (vec2<int>)use_src_size), (vec2<int>)size ()));

  const vec2<unsigned int> dst_tl (std::min (std::max (vec2<int> (0), dst_xy), (vec2<int>)dst.size ()));
  const vec2<unsigned int> dst_br (std::min (std::max (vec2<int> (0), dst_xy + (vec2<int>)use_src_size), (vec2<int>)dst.size ()));

  const auto src_copy_sz = src_br - src_tl;
  const auto dst_copy_sz = dst_br - dst_tl;

  const auto copy_sz = std::min (src_copy_sz, dst_copy_sz);

  if (copy_sz.x == 0 || copy_sz.y == 0)
    return { };

  const unsigned int src_stride = m_bytes_per_line;
  const unsigned int dst_stride = dst.bytes_per_line ();

  auto&& src_ptr = (const char*)this->data_at (src_tl);
  auto&& dst_ptr = (char*)dst.data_at (dst_tl);

  auto&& copy_line = conv_func_table[m_format.value ()][dst.format ().value ()];

  for (unsigned int yy = 0; yy < copy_sz.y; ++yy)
  {
    copy_line (src_ptr, dst_ptr, copy_sz.x);

    src_ptr += src_stride;
    dst_ptr += dst_stride;
  }

  return { src_tl, dst_tl, copy_sz };
}

image image::pyr_down (void) const
{
  image dst (m_format, std::max (vec2<unsigned int> (1), m_size / 2));

  pyr_down_to (dst);

  return std::move (dst);
}

void image::pyr_down_to (image&& dst) const
{
  pyr_down_to ((image&)dst);
}

void image::pyr_down_to (image& dst) const
{
  const auto required_dst_size = std::max (vec2<unsigned int> (1), m_size / 2);

  if (dst.size ().x < required_dst_size.x || dst.size ().y < required_dst_size.y)
    return;

  const unsigned int src_stride = m_bytes_per_line;
  const unsigned int dst_stride = dst.bytes_per_line ();

  auto&& src_ptr = (const char*)this->data ();
  auto&& dst_ptr = (char*)dst.data ();

  auto&& pyr_down_line = pyr_down_func_table[m_format.value ()][dst.format ().value ()];

  for (unsigned int yy = 0; yy < dst.height (); ++yy)
  {
    pyr_down_line (src_ptr, src_ptr + src_stride, dst_ptr, dst.width ());

    src_ptr += src_stride * 2;
    dst_ptr += dst_stride;
  }
}

image image::subimg (const vec2<int>& xy, const vec2<unsigned int>& sz) const
{
  const vec2<unsigned int> src_tl (std::min (std::max (vec2<int> (0), xy), (vec2<int>)size ()));
  const vec2<unsigned int> src_br (std::min (std::max (vec2<int> (0), xy + (vec2<int>)sz), (vec2<int>)size ()));

  const auto copy_sz = src_br - src_tl;

  if (copy_sz.x == 0 || copy_sz.y == 0)
    return { };

  image res;
  res.m_size = copy_sz;
  res.m_bytes_per_line = m_bytes_per_line;
  res.m_format = m_format;
  res.m_data.ptr = (char*)data_at (src_tl);

  return std::move (res);
}
