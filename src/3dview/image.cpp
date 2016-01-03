
#include <cstring>
#include <cassert>

#include "image.hpp"

#include "utils/vec_mat.hpp"
#include "utils/utils.hpp"

static inline float rgb_to_luma (const vec3<float>& rgb)
{
  // https://en.wikipedia.org/wiki/Grayscale
  // Y = 0.2126 * R + 0.7152 * G + 0.0722 * B

  return dot (rgb, { 0.2126f, 0.7152f, 0.0722f });
}

static inline vec4<uint8_t> float_to_u8 (const vec4<float>& val)
{
  vec4<float> one (1);
  vec4<float> zero (0);
  vec4<float> u8max (255);

  return (vec4<uint8_t>) std::min (std::max (std::min (one, val), zero) * 256, u8max);
}

static inline uint8_t float_to_u8 (float val)
{
  return (uint8_t) std::min (std::max (std::min (1.0f, val), 0.0f) * 256.0f, 255.0f);
}

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
    uint8_t val = float_to_u8 (rgb_to_luma ({ r, g, b }));

    fill_2d<uint8_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val);
  }
  else if (m_format == pixel_format::a_8)
  {
    uint8_t val = float_to_u8 (a);

    fill_2d<uint8_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line, val);
  }
  else if (m_format == pixel_format::la_88)
  {
    uint8_t val_l = float_to_u8 (rgb_to_luma ({ r, g, b }));
    uint8_t val_a = float_to_u8 (a);

    fill_2d<uint16_t> (m_data.get (), x0, y0, w, h, m_bytes_per_line,
		       (val_a << 8) | val_l);
  }
  else if (m_format == pixel_format::rgb_888)
  {
    vec3<uint8_t> val = float_to_u8 ({ r, g, b, 1 }).rgb ();

    const unsigned int stride = m_bytes_per_line;
    uint8_t* dst = (uint8_t*)((uintptr_t)m_data.get () + y0 * stride + x0 * 3);

    for (unsigned int yy = 0; yy < h; ++yy)
    {
      for (unsigned int xx = 0; xx < w; ++xx)
      {
	dst[xx*3 + 0] = val.r;
	dst[xx*3 + 1] = val.g;
	dst[xx*3 + 2] = val.b;
      }
      dst = (uint8_t*)((uintptr_t)dst + stride);
    }
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

  uint8_t* src_ptr = (uint8_t*)((uintptr_t)m_data.get () + src_y0 * src_stride
				+ src_x0 * m_format.bytes_per_pixel ());
  uint8_t* dst_ptr = (uint8_t*)((uintptr_t)m_data.get () + dst_y0 * dst_stride
				+ dst_x0 * m_format.bytes_per_pixel ());

  if (m_format == dst.format ())
  {
    // if src and dst pixel format is the same, can do a straight copy.
    const unsigned int copy_line_bytes = copy_w * m_format.bytes_per_pixel ();
 
    for (unsigned int yy = 0; yy < copy_h; ++yy)
    {
      for (unsigned int xx = 0; xx < copy_w; ++xx)
	std::memcpy (dst_ptr, src_ptr, copy_line_bytes);

      src_ptr += src_stride;
      dst_ptr += dst_stride;
    }
  }
  else
  {
    // otherwise convert line-wise to vec4<uint8_t> and then to the dst format.

  }
}

image image::pyr_down (down_sample_mode_t mode) const
{
  image i (m_format,
	   std::max ((unsigned int)1, m_width / 2),
	   std::max ((unsigned int)1, m_height / 2));

  

  return std::move (i);
}

template <pixel_format::fmt_t PixelFormat, typename StorageType>
vec4<uint8_t> unpack_pixel (StorageType p);

template <pixel_format::fmt_t PixelFormat, typename StorageType>
StorageType pack_pixel (vec4<uint8_t> p);

template<> vec4<uint8_t>
unpack_pixel<pixel_format::rgba_8888, uint32_t> (uint32_t p)
{
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return { (p >> 0) & 0xFF, (p >> 8) & 0xFF, (p >> 16) & 0xFF, (p >> 24) & 0xFF };
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return { (p >> 24) & 0xFF, (p >> 16) & 0xFF, (p >> 8) & 0xFF, (p >> 0) & 0xFF };
  #else
    #error unsupported endian
  #endif
}

template<> uint32_t
pack_pixel<pixel_format::rgba_8888, uint32_t> (vec4<uint8_t> p)
{
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (p.a << 24) | (p.b << 16) | (p.g << 8) | p.r;
  #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return (p.r << 24) | (p.g << 16) | (p.b << 8) | p.a;
  #else
    #error unsupported endian
  #endif
}

template <pixel_format::fmt_t SrcPixelFormat, typename SrcStorageType,
	  pixel_format::fmt_t DstPixelFormat, typename DstStorageType>
static inline void
convert_line (const char* src, char* dst, unsigned int w)
{
  for (unsigned int xx = 0; xx < w; ++xx)
  {
    vec4<uint8_t> p = unpack_pixel<SrcPixelFormat, SrcStorageType> (*(const SrcStorageType*)src);
    src += sizeof (SrcStorageType);
    *(DstStorageType*)dst = pack_pixel<DstPixelFormat, DstStorageType> (p);
    dst += sizeof (DstStorageType);
  }
}

void convert_format (const char* src, unsigned int src_stride,
		     char* dst, unsigned int dst_stride,
		     unsigned int w, unsigned int h)
{
  for (unsigned int yy = 0; yy < h; ++yy)
  {
    convert_line<pixel_format::rgba_8888, uint32_t,
		 pixel_format::rgba_8888, uint32_t> (src, dst, w);
    src += src_stride;
    dst += dst_stride;
  }
}
