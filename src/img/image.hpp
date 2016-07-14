#ifndef includeguard_image_includeguard
#define includeguard_image_includeguard

#include <memory>
#include <algorithm>
#include "img/pixel_format.hpp"
#include "utils/vec_mat.hpp"

namespace img
{

class image
{
public:
  image (void)
  : m_size (0, 0), m_bytes_per_line (0), m_format (pixel_format::invalid)
  {
  }

  image (pixel_format pf, const utils::vec2<unsigned int>& size);

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
    m_data.swap (rhs.m_data);
  }

  bool empty (void) const
  {
    return m_bytes_per_line == 0 || m_size.y == 0
	   || m_format == pixel_format::invalid;
  }

  const utils::vec2<unsigned int>& size (void) const { return m_size; }

  unsigned int width (void) const { return m_size.x; }
  unsigned int height (void) const { return m_size.y; }
  pixel_format format (void) const { return m_format; }

  const void* data (void) const { return &m_data.ptr[0]; }
  void* data (void) { return &m_data.ptr[0]; }

  void* data_line (unsigned int y)
  {
    return &m_data.ptr[y * m_bytes_per_line];
  }

  const void* data_line (unsigned int y) const
  {
    return &m_data.ptr[y * m_bytes_per_line];
  }

  const void* data_at (const utils::vec2<unsigned int>& xy) const
  {
    return &m_data.ptr[xy.y * m_bytes_per_line + xy.x * m_format.bytes_per_pixel ()];
  }

  void* data_at (const utils::vec2<unsigned int>& xy)
  {
    return &m_data.ptr[xy.y * m_bytes_per_line + xy.x * m_format.bytes_per_pixel ()];
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
    fill_result (const utils::vec2<unsigned int>& tl, const utils::vec2<unsigned int>& sz)
    : top_left (tl), size (sz) { }

    utils::vec2<unsigned int> top_left = { 0 };
    utils::vec2<unsigned int> size = { 0 };
  };

  fill_result
  fill (const utils::vec2<int>& xy, const utils::vec2<unsigned int>& size,
	const utils::vec4<float>& rgba_value);

  fill_result fill (const utils::vec4<float>& rgba_value)
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
    copy_to_result (const utils::vec2<unsigned int>& src_tl,
		    const utils::vec2<unsigned int>& dst_tl,
		    const utils::vec2<unsigned int>& sz)
    : src_top_left (src_tl), dst_top_left (dst_tl), size (sz) { }

    utils::vec2<unsigned int> src_top_left = { 0 };
    utils::vec2<unsigned int> dst_top_left = { 0 };

    utils::vec2<unsigned int> size = { 0 };
  };

  copy_to_result
  copy_to (const utils::vec2<int>& src_xy, const utils::vec2<unsigned int>& src_size,
	   image& dst, const utils::vec2<int>& dst_xy = { 0, 0 }) const;

  copy_to_result
  copy_to (const utils::vec2<int>& src_xy,
	   image& dst, const utils::vec2<int>& dst_xy = { 0, 0 }) const
  {
    return copy_to (src_xy, size (), dst, dst_xy);
  }

  copy_to_result
  copy_to (image& dst, const utils::vec2<int>& dst_xy = { 0, 0 }) const
  {
    return copy_to ({0, 0}, size (), dst, dst_xy);
  }

  image pyr_down (void) const;
  void pyr_down_to (image& dst) const;
  void pyr_down_to (image&& dst) const;  // for rhs result of subimg

  // returns a shallow copy of a sub-region.  the data buffer ownership
  // is not transferred to the resulting image.  however, when the resulting
  // image is assigned (copy constructor, copy assignment operator), a new
  // buffer will be created and the data will be copied.
  image subimg (const utils::vec2<int>& xy, const utils::vec2<unsigned int>& sz) const;

protected:
  struct data_buffer
  {
    char* ptr;
    std::unique_ptr<char[]> allocated;

    data_buffer (void) : ptr (nullptr) { }
    data_buffer (const data_buffer&) = delete;

    data_buffer (data_buffer&& rhs)
    : ptr (std::move (rhs.ptr)), allocated (std::move (rhs.allocated))
    {
      rhs.ptr = nullptr;
    }

    data_buffer (size_t sz_bytes, uintptr_t align = 128)
    {
      allocated = std::make_unique<char[]> (sz_bytes + align);
      ptr = (char*)(((uintptr_t)allocated.get () + (align-1)) & ~(align-1));
    }

    data_buffer& operator = (const data_buffer&) = delete;
    data_buffer& operator = (data_buffer&& rhs)
    {
      ptr = std::move (rhs.ptr);
      allocated = std::move (rhs.allocated);
      rhs.ptr = nullptr;
      return *this;
    }

    data_buffer& operator = (std::nullptr_t)
    {
      ptr = nullptr;
      allocated = nullptr;
      return *this;
    }

    void swap (data_buffer& rhs)
    {
      std::swap (ptr, rhs.ptr);
      std::swap (allocated, rhs.allocated);
    }
  };

  static constexpr uintptr_t
  align_stride (uintptr_t length_bytes, uintptr_t align = 16)
  {
    return (length_bytes + (align-1)) & ~(align-1);
  }

  utils::vec2<unsigned int> m_size;
  unsigned int m_bytes_per_line;
  pixel_format m_format;
  data_buffer m_data;
};

} // namespace img

namespace std
{

template<> inline void swap (img::image& a, img::image& b)
{
  a.swap (b);
}

} // namespace std

#endif // includeguard_image_includeguard
