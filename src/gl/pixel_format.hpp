
#ifndef includeguard_pixelformat_hpp_includeguard
#define includeguard_pixelformat_hpp_includeguard

#include <iosfwd>

class pixel_format
{
public:
  enum fmt_t
  {
    invalid = 0,

    rgba_8888 = 0x0000'8888,
    rgba_4444 = 0x0000'4444,
    rgba_5551 = 0x0000'5551,
    rgb_888   = 0x0000'8880,
    rgb_565   = 0x0000'5650,
    rgb_555   = 0x0000'5550,
    rgb_444   = 0x0000'4440,
    l_8       = 0x0001'0000,
    a_8       = 0x0002'0000,
    la_88     = 0x0003'0000,
  };

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

  constexpr unsigned int r_bits (void) const { return ((unsigned int)m_fmt >> 12) & 0x0F; }
  constexpr unsigned int g_bits (void) const { return ((unsigned int)m_fmt >>  8) & 0x0F; }
  constexpr unsigned int b_bits (void) const { return ((unsigned int)m_fmt >>  4) & 0x0F; }
  constexpr unsigned int a_bits (void) const { return ((unsigned int)m_fmt >>  0) & 0x0F; }

  constexpr unsigned int rgb_bits (void) const { return r_bits () + g_bits () + b_bits (); }

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

    ds_24_8 = 0xFFFF1808,
    ds_24_0 = 0xFFFF1800,
    ds_16_8 = 0xFFFF1008,
    ds_16_0 = 0xFFFF1000,
    ds_0_8  = 0xFFFF0008,
    ds_0_0  = 0xFFFF0000
  };

  constexpr ds_format (void) : m_fmt (invalid) { }
  constexpr ds_format (fmt_t f) : m_fmt (f) { }

  // parse from string.
  ds_format (const char* str);

  constexpr bool operator == (const ds_format& rhs) const { return m_fmt == rhs.m_fmt; }
  constexpr bool operator == (fmt_t rhs) const { return m_fmt == rhs; }
  constexpr bool operator != (const ds_format& rhs) const { return m_fmt != rhs.m_fmt; }
  constexpr bool operator != (fmt_t rhs) const { return m_fmt != rhs; }

  constexpr bool valid (void) const { return (unsigned int)m_fmt >> 16; }
  constexpr fmt_t value (void) const { return m_fmt; }

  constexpr operator fmt_t () const { return m_fmt; }

  constexpr unsigned int d_bits (void) const { return ((unsigned int)m_fmt >> 8) & 0xFF & ((unsigned int)m_fmt >> 16); }
  constexpr unsigned int s_bits (void) const { return ((unsigned int)m_fmt >> 0) & 0xFF & ((unsigned int)m_fmt >> 16); }

  const char* str (void) const;

private:
  fmt_t m_fmt;
};


std::ostream& operator << (std::ostream& out, const pixel_format&);
std::ostream& operator << (std::ostream& out, const ds_format&);


#endif // includeguard_pixelformat_hpp_includeguard
