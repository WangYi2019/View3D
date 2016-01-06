#ifndef includeguard_image_includeguard
#define includeguard_image_includeguard

#include <memory>
#include <algorithm>
#include "gl/pixel_format.hpp"
#include "utils/vec_mat.hpp"

class image
{
public:
  image (void)
  : m_size (0, 0), m_bytes_per_line (0), m_format (pixel_format::invalid)
  {
  }

  image (pixel_format pf, const vec2<unsigned int>& size);

  image (const image& rhs);
  image (image&& rhs);

  image& operator = (const image& rhs);
  image& operator = (image&& rhs);

  ~image (void);

  void swap (image& rhs)
  {
    std::swap (m_size, rhs.m_size);
    std::swap (m_bytes_per_line, rhs.m_bytes_per_line);
    std::swap (m_format, rhs.m_format);
    std::swap (m_data, rhs.m_data);
  }

  bool empty (void) const
  {
    return m_bytes_per_line == 0 || m_size.y == 0
	   || m_format == pixel_format::invalid;
  }

  const vec2<unsigned int>& size (void) const { return m_size; }

  unsigned int width (void) const { return m_size.x; }
  unsigned int height (void) const { return m_size.y; }
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

  const void* data_at (const vec2<unsigned int>& xy) const
  {
    return &m_data[xy.y * m_bytes_per_line + xy.x * m_format.bytes_per_pixel ()];
  }

  void* data_at (const vec2<unsigned int>& xy)
  {
    return &m_data[xy.y * m_bytes_per_line + xy.x * m_format.bytes_per_pixel ()];
  }

  unsigned int data_line_size_bytes (void) const { return m_bytes_per_line; }
  unsigned int bytes_per_line (void) const { return m_bytes_per_line; }

  // the specified rect is clipped to the actual image size.
  // if the image pixel format is luma instead of rgb, the red value will be used.
  // the component float values are clamped and scaled to match the pixel format.
  // for unsigned pixel formats the normalized value range is [0,+1].
  // for signed pixel formats the normalized value range is [-1,+1].
  struct fill_result
  {
    fill_result (void) { }
    fill_result (const vec2<unsigned int>& tl, const vec2<unsigned int>& sz)
    : top_left (tl), size (sz) { }

    vec2<unsigned int> top_left = { 0 };
    vec2<unsigned int> size = { 0 };
  };

  fill_result
  fill (const vec2<int>& xy, const vec2<unsigned int>& size,
	const vec4<float>& rgba_value);

  fill_result fill (const vec4<float>& rgba_value)
  {
    return fill ({ 0, 0 }, size (), rgba_value);
  }

  // copy the source area from this image to an area in another image.
  // the source area is clipped to be inside of this image.
  // the resulting area is then clipped to be inside of the other image.
  // if the pixel formats of the images differ, they are converted.
  // converting rgb to luma is done by calculating a grayscale value.
  // converting luma to rgb is done by replicating the luma values.
  // returns the clipped areas in the source and destination images that have
  // been used for copying.
  struct copy_to_result
  {
    copy_to_result (void) { }
    copy_to_result (const vec2<unsigned int>& src_tl, const vec2<unsigned int>& dst_tl,
		    const vec2<unsigned int>& sz)
    : src_top_left (src_tl), dst_top_left (dst_tl), size (sz) { }

    vec2<unsigned int> src_top_left = { 0 };
    vec2<unsigned int> dst_top_left = { 0 };

    vec2<unsigned int> size = { 0 };
  };

  copy_to_result
  copy_to (const vec2<int>& src_xy, const vec2<unsigned int>& src_size,
	   image& dst, const vec2<int>& dst_xy = { 0, 0 }) const;

  copy_to_result
  copy_to (const vec2<int>& src_xy,
	   image& dst, const vec2<int>& dst_xy = { 0, 0 }) const
  {
    return copy_to (src_xy, size (), dst, dst_xy);
  }

  copy_to_result
  copy_to (image& dst, const vec2<int>& dst_xy = { 0, 0 }) const
  {
    return copy_to ({0, 0}, size (), dst, dst_xy);
  }

  enum down_sample_mode_t
  {
    down_sample_avg,
  };

  // FIXME: add overload to specifiy the destination image
  //        (which could be a shared-buffer subimg).
  image pyr_down (down_sample_mode_t mode = down_sample_avg) const;

  // FIXME: return a shared image buffer instead of a copy.
  //        add parameter to subimg to make a real copy.
  image subimg (const vec2<int>& xy, const vec2<unsigned int>& sz) const;

private:
  vec2<unsigned int> m_size;
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
