
#ifndef includeguard_pixelformat_hpp_includeguard
#define includeguard_pixelformat_hpp_includeguard

#include <iosfwd>

class pixel_format
{
public:
  enum fmt_t
  {
    invalid = 0,
    rgba_8888,
    rgba_4444,
    rgba_5551,
    rgb_888,
    rgb_565,
    rgb_555,
    rgb_444,
    l_8,
    a_8,
    la_88,
  };

  static constexpr unsigned int max_count = la_88 + 1;

  constexpr pixel_format (void) : m_fmt (invalid) { }
  constexpr pixel_format (fmt_t f) : m_fmt (f) { }

  // parse from string.
  pixel_format (const char* str);

  constexpr bool operator == (const pixel_format& rhs) const { return m_fmt == rhs.m_fmt; }
  constexpr bool operator == (fmt_t rhs) const { return m_fmt == rhs; }
  constexpr bool operator != (const pixel_format& rhs) const { return m_fmt != rhs.m_fmt; }
  constexpr bool operator != (fmt_t rhs) const { return m_fmt != rhs; }

  constexpr bool valid (void) const { return m_fmt != invalid; }
  constexpr fmt_t value (void) const { return m_fmt; }

  constexpr operator fmt_t () const { return m_fmt; }

  unsigned int r_bits (void) const;
  unsigned int g_bits (void) const;
  unsigned int b_bits (void) const;
  unsigned int a_bits (void) const;

  unsigned int rgb_bits (void) const;

  const char* str (void) const;

  unsigned int gl_fmt (void) const;
  unsigned int gl_type (void) const;

  unsigned int bits_per_pixel (void) const;
  unsigned int bytes_per_pixel (void) const { return bits_per_pixel () / 8; }

private:
  fmt_t m_fmt;
};

class ds_format
{
public:
  enum fmt_t
  {
    invalid = 0,

    ds_24_8,
    ds_24_0,
    ds_16_8,
    ds_16_0,
    ds_0_8,
    ds_0_0
  };

  constexpr ds_format (void) : m_fmt (invalid) { }
  constexpr ds_format (fmt_t f) : m_fmt (f) { }

  // parse from string.
  ds_format (const char* str);

  constexpr bool operator == (const ds_format& rhs) const { return m_fmt == rhs.m_fmt; }
  constexpr bool operator == (fmt_t rhs) const { return m_fmt == rhs; }
  constexpr bool operator != (const ds_format& rhs) const { return m_fmt != rhs.m_fmt; }
  constexpr bool operator != (fmt_t rhs) const { return m_fmt != rhs; }

  constexpr bool valid (void) const { return m_fmt != invalid; }
  constexpr fmt_t value (void) const { return m_fmt; }

  constexpr operator fmt_t () const { return m_fmt; }

  unsigned int d_bits (void) const;
  unsigned int s_bits (void) const;

  const char* str (void) const;

private:
  fmt_t m_fmt;
};


std::ostream& operator << (std::ostream& out, const pixel_format&);
std::ostream& operator << (std::ostream& out, const ds_format&);


#endif // includeguard_pixelformat_hpp_includeguard
