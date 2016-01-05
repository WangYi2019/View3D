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

  bool empty (void) const
  {
    return m_bytes_per_line == 0 || m_height == 0
	   || m_format == pixel_format::invalid;
  }

  vec2<unsigned int> size (void) const { return { m_width, m_height }; }

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
  unsigned int bytes_per_line (void) const { return m_bytes_per_line; }

  // the specified rect is clipped to the actual image size.
  // if the image pixel format is luma instead of rgb, the red value will be used.
  // the component float values are clamped and scaled to match the pixel format.
  // for unsigned pixel formats the normalized value range is [0,+1].
  // for signed pixel formats the normalized value range is [-1,+1].
  void fill (int x, int y,
	     unsigned int width, unsigned int height,
	     float r, float g, float b, float a);

  void fill (float r, float g, float b, float a)
  {
    fill (0, 0, width (), height (), r, g, b, a);
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

    vec2<unsigned int> src_top_left = { 0, 0 };
    vec2<unsigned int> dst_top_left = { 0, 0 };

    vec2<unsigned int> size = { 0, 0 };
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

  image pyr_down (down_sample_mode_t mode = down_sample_avg) const;

  image subimg (const vec2<int>& xy, const vec2<unsigned int>& sz) const;

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
