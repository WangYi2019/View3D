#ifndef includeguard_gl_hpp_includeguard
#define includeguard_gl_hpp_includeguard

#define GL_INCLUDED

#define GL_GLEXT_PROTOTYPES

// ------------------------------------------------------------------
#ifdef USE_GL

#ifdef _WIN32

// on win32 we can't override function symbols with function pointer
// variables.  thus, don't pull-in the function declarations, but just
// the signatures.

#undef GL_GLEXT_PROTOTYPES

#endif

#include <GL/gl.h>
#include <GL/glext.h>

#ifndef GL_RGB565
  #define GL_RGB565 0x8D62
#endif

#ifndef GL_ARB_invalidate_subdata
inline void glInvalidateFramebuffer (GLenum,GLsizei,const GLenum*) { }
#endif

#ifdef _WIN32

#define concat1(x,y) x ## y
#define concat(x,y) concat1(x,y)

#define glextfunc(t,n) extern t concat(n, _);

#include "glext_funcs.x.hpp"

#undef glextfunc
#undef concat1
#undef concat


#define glDeleteBuffers(...) glDeleteBuffers_ (__VA_ARGS__)
#define glGenBuffers(...) glGenBuffers_ (__VA_ARGS__)
#define glBindBuffer(...) glBindBuffer_ (__VA_ARGS__)
#define glMapBuffer(...) glMapBuffer_ (__VA_ARGS__)
#define glUnmapBuffer(...) glUnmapBuffer_ (__VA_ARGS__)
#define glBufferData(...) glBufferData_ (__VA_ARGS__)
#define glBufferSubData(...) glBufferSubData_ (__VA_ARGS__)
#define glGenerateMipmap(...) glGenerateMipmap_ (__VA_ARGS__)
#define glActiveTexture(...) glActiveTexture_ (__VA_ARGS__)
#define glDeleteProgram(...) glDeleteProgram_ (__VA_ARGS__)
#define glDeleteShader(...) glDeleteShader_ (__VA_ARGS__)
#define glCompileShader(...) glCompileShader_ (__VA_ARGS__)
#define glUseProgram(...) glUseProgram_ (__VA_ARGS__)
#define glCreateShader(...) glCreateShader_ (__VA_ARGS__)
#define glShaderSource(...) glShaderSource_ (__VA_ARGS__)
#define glGetShaderiv(...) glGetShaderiv_ (__VA_ARGS__)
#define glGetShaderInfoLog(...) glGetShaderInfoLog_ (__VA_ARGS__)
#define glCreateProgram(...) glCreateProgram_ (__VA_ARGS__)
#define glAttachShader(...) glAttachShader_ (__VA_ARGS__)
#define glLinkProgram(...) glLinkProgram_ (__VA_ARGS__)
#define glValidateProgram(...) glValidateProgram_ (__VA_ARGS__)
#define glGetProgramiv(...) glGetProgramiv_ (__VA_ARGS__)
#define glGetUniformLocation(...) glGetUniformLocation_ (__VA_ARGS__)
#define glGetAttribLocation(...) glGetAttribLocation_ (__VA_ARGS__)
#define glVertexAttribPointer(...) glVertexAttribPointer_ (__VA_ARGS__)
#define glUniform1f(...) glUniform1f_ (__VA_ARGS__)
#define glUniform2f(...) glUniform2f_ (__VA_ARGS__)
#define glUniform3f(...) glUniform3f_ (__VA_ARGS__)
#define glUniform4f(...) glUniform4f_ (__VA_ARGS__)
#define glUniform1i(...) glUniform1i_ (__VA_ARGS__)
#define glUniform2i(...) glUniform2i_ (__VA_ARGS__)
#define glUniform3i(...) glUniform3i_ (__VA_ARGS__)
#define glUniform4i(...) glUniform4i_ (__VA_ARGS__)
#define glUniform1fv(...) glUniform1fv_ (__VA_ARGS__)
#define glUniform2fv(...) glUniform2fv_ (__VA_ARGS__)
#define glUniform3fv(...) glUniform3fv_ (__VA_ARGS__)
#define glUniform4fv(...) glUniform4fv_ (__VA_ARGS__)
#define glUniform1iv(...) glUniform1iv_ (__VA_ARGS__)
#define glUniform2iv(...) glUniform2iv_ (__VA_ARGS__)
#define glUniform3iv(...) glUniform3iv_ (__VA_ARGS__)
#define glUniform4iv(...) glUniform4iv_ (__VA_ARGS__)
#define glUniformMatrix2fv(...) glUniformMatrix2fv_ (__VA_ARGS__)
#define glUniformMatrix3fv(...) glUniformMatrix3fv_ (__VA_ARGS__)
#define glUniformMatrix4fv(...) glUniformMatrix4fv_ (__VA_ARGS__)
#define glDisableVertexAttribArray(...) glDisableVertexAttribArray_ (__VA_ARGS__)
#define glEnableVertexAttribArray(...) glEnableVertexAttribArray_ (__VA_ARGS__)
#define glVertexAttrib1f(...) glVertexAttrib1f_ (__VA_ARGS__)
#define glVertexAttrib1fv(...) glVertexAttrib1fv_ (__VA_ARGS__)
#define glVertexAttrib2f(...) glVertexAttrib2f_ (__VA_ARGS__)
#define glVertexAttrib2fv(...) glVertexAttrib2fv_ (__VA_ARGS__)
#define glVertexAttrib3f(...) glVertexAttrib3f_ (__VA_ARGS__)
#define glVertexAttrib3fv(...) glVertexAttrib3fv_ (__VA_ARGS__)
#define glVertexAttrib4f(...) glVertexAttrib4f_ (__VA_ARGS__)
#define glVertexAttrib4fv(...) glVertexAttrib4fv_ (__VA_ARGS__)

#endif // _WIN32

// ------------------------------------------------------------------
#elif USE_GLES2

#ifndef GL_API_EXT
  #define GL_API_EXT
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef GL_WRITE_ONLY
  #define GL_WRITE_ONLY GL_WRITE_ONLY_OES
#endif

#ifndef GL_DEPTH_COMPONENT24
  #define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

#ifndef GL_DEPTH_COMPONENT32
  #define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#endif

#ifndef GL_RGBA8
  #define GL_RGBA8 GL_RGBA8_OES
#endif

#ifndef GL_RGB8
  #define GL_RGB8 GL_RGB8_OES
#endif

#define glClearDepth glClearDepthf

// function pointer variables are defined inside the gldev implementations.
extern PFNGLMAPBUFFEROESPROC glMapBuffer;
extern PFNGLUNMAPBUFFEROESPROC glUnmapBuffer;

// in GLES 2 it's glDiscardFramebufferEXT (extension)
// in GLES 3 it's glInvalidateFramebuffer (standard)
extern PFNGLDISCARDFRAMEBUFFEREXTPROC glInvalidateFramebuffer;

#endif

// ------------------------------------------------------------------

// from glDiscardFrameBufferEXT
// in GLES 3 and GL 4 those are already available.
#ifndef GL_COLOR
  #define GL_COLOR                      0x1800
#endif

#ifndef GL_DEPTH
  #define GL_DEPTH                      0x1801
#endif

#ifndef GL_STENCIL
  #define GL_STENCIL                    0x1802
#endif


/*

#define GL_PALETTE4_RGB8_OES         (0x8B90)
#define GL_PALETTE4_RGBA8_OES        (0x8B91)
#define GL_PALETTE4_R5_G6_B5_OES     (0x8B92)
#define GL_PALETTE4_RGBA4_OES        (0x8B93)
#define GL_PALETTE4_RGB5_A1_OES      (0x8B94)
#define GL_PALETTE8_RGB8_OES         (0x8B95)
#define GL_PALETTE8_RGBA8_OES        (0x8B96)
#define GL_PALETTE8_R5_G6_B5_OES     (0x8B97)
#define GL_PALETTE8_RGBA4_OES        (0x8B98)
#define GL_PALETTE8_RGB5_A1_OES      (0x8B99)

// some pseudo formats
#define GL_RGBA_4444 (GL_RGBA | (1 << 17))
#define GL_RGBA_5551 (GL_RGBA | (2 << 17))


*/

#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <cstdio>

#include <functional>
#include <array>

#include "utils/utils.hpp"
#include "utils/vec_mat.hpp"
#include "pixel_format.hpp"

namespace gl
{

extern void check_log_error_1 (const char* file, int lineno);
#define gl_check_log_error() do { gl::check_log_error_1 (__FILE__, __LINE__); } while (0)

// ==========================================================================
// per-frame statistics

struct frame_stats
{
  int64_t primitive_count;

  int64_t vertex_count;
  int64_t vertex_byte_count;

  int64_t index_count;
  int64_t index_byte_count;

  int64_t draw_call_count;
  int64_t buffer_select_count;
  int64_t shader_select_count;

  frame_stats (void) { reset (); }

  void reset (void)
  {
    primitive_count = 0;

    vertex_count = 0;
    vertex_byte_count = 0;

    index_count = 0;
    index_byte_count = 0;

    draw_call_count = 0;
    buffer_select_count = 0;
    shader_select_count = 0;
  }

  frame_stats& operator += (const frame_stats& rhs)
  {
    primitive_count += rhs.primitive_count;
    vertex_count += rhs.vertex_count;
    vertex_byte_count += rhs.vertex_byte_count;
    index_count += rhs.index_count;
    index_byte_count += rhs.index_byte_count;
    draw_call_count += rhs.draw_call_count;
    buffer_select_count += rhs.buffer_select_count;
    shader_select_count += rhs.shader_select_count;
    return *this;
  };
};

extern frame_stats cur_frame_stats;

// ==========================================================================
// GL vertex / index buffer wrapper

struct bind_vertex_buffer;
struct bind_index_buffer;

class buffer
{
public:
  enum buffer_type
  {
    vertex = GL_ARRAY_BUFFER,
    index = GL_ELEMENT_ARRAY_BUFFER
  };

  enum usage_type
  {
    static_draw = GL_STATIC_DRAW,
    dynamic_draw = GL_DYNAMIC_DRAW
  };

  enum access_type
  {
    write_only = GL_WRITE_ONLY,
    // read_only
    // read_write
  };


  buffer (void) : m_type ((buffer_type)0), m_obj (0) { }

  buffer (buffer_type t)
  : m_type (t), m_obj (0)
  {
    glGenBuffers (1, &m_obj);
  }

  buffer (buffer_type t, const void* data, size_t sz_bytes,
          usage_type ut = static_draw)
  : m_type (t), m_obj (0)
  {
    glGenBuffers (1, &m_obj);
    upload (data, sz_bytes, ut);
  }

  template <typename T, typename A>
  buffer (buffer_type t, const std::vector<T, A>& data,
          usage_type ut = static_draw)
  : m_type (t), m_obj (0)
  {
    glGenBuffers (1, &m_obj);
    upload (data, ut);
  }

  template <unsigned int N, typename T>
  buffer (buffer_type t, const T (&data)[N], usage_type ut = static_draw)
  : m_type (t), m_obj (0)
  {
    glGenBuffers (1, &m_obj);
    upload (data, ut);
  }

  ~buffer (void)
  {
    glDeleteBuffers (1, &m_obj);

    if (m_obj == 0 || m_type == (buffer_type)0)
      return;

    unsigned int* cur_bound = m_type == vertex ? &g_cur_vtx_bound : &g_cur_idx_bound;
    if (*cur_bound == m_obj)
    {
      glBindBuffer (m_type, m_obj);
      *cur_bound = 0;
      cur_frame_stats.buffer_select_count += 1;
    }
  }

  buffer (const buffer&) = delete;

  buffer (buffer&& rhs)
  {
    m_type = rhs.m_type;
    m_obj = rhs.m_obj;
    rhs.m_type = (buffer_type)0;
    rhs.m_obj = 0;
  }

  buffer& operator = (const buffer&) = delete;

  buffer& operator = (buffer&& rhs)
  {
    if (this == &rhs)
      return *this;

    glDeleteBuffers (1, &m_obj);
    m_type = rhs.m_type;
    m_obj = rhs.m_obj;

    rhs.m_type = (buffer_type)0;
    rhs.m_obj = 0;
    return *this;
  }

  void* map (access_type at) { return glMapBuffer (m_type, at); }
  void unmap (void) { glUnmapBuffer (m_type); }

  template <typename T> T* map_as (access_type at)
  {
    return (T*)glMapBuffer (m_type, at);
  }

  // glBufferData
  void allocate (size_t sz_bytes, usage_type ut = static_draw)
  {
    bind ();
    glBufferData (m_type, sz_bytes, NULL, ut);
  }

  void upload (const void* data, size_t sz_bytes, usage_type ut = static_draw)
  {
    bind ();
    glBufferData (m_type, sz_bytes, data, ut);
  }

  template <typename T, typename A>
  void upload (const std::vector<T, A>& data, usage_type ut = static_draw)
  {
    bind ();
    glBufferData (m_type, data.size () * sizeof (T), &data.front (), ut);
  }

  template <unsigned int N, typename T>
  void upload (const T (&data)[N], usage_type ut = static_draw)
  {
    bind ();
    glBufferData (m_type, N * sizeof (T), data, ut);
  }

  // glBufferSubData
  void upload (const void* data, size_t sz_bytes, size_t offset_bytes)
  {
    bind ();
    glBufferSubData (m_type, offset_bytes, sz_bytes, data);
  }

  template <typename T, typename A>
  void upload (const std::vector<T, A>& data, size_t offset_bytes)
  {
    bind ();
    glBufferSubData (m_type, offset_bytes, data.size () * sizeof (T), data);
  }

  template <unsigned int N, typename T>
  void upload (const T (&data)[N], size_t offset_bytes)
  {
    bind ();
    glBufferSubData (m_type, offset_bytes, N * sizeof (T), data);
  }

  void swap (buffer& rhs)
  {
    std::swap (m_type, rhs.m_type);
    std::swap (m_obj, rhs.m_obj);
  }

private:
  static unsigned int g_cur_vtx_bound;
  static unsigned int g_cur_idx_bound;

  void bind (void) const
  {
    if (m_type == (buffer_type)0)
      return;

    unsigned int* cur_bound = m_type == vertex ? &g_cur_vtx_bound : &g_cur_idx_bound;

    if (*cur_bound != m_obj)
    {
      glBindBuffer (m_type, m_obj);
      *cur_bound = m_obj;
      cur_frame_stats.buffer_select_count += 1;
    }
  }

  mutable buffer_type m_type;
  mutable unsigned int m_obj;

friend struct bind_vertex_buffer;
friend struct bind_index_buffer;
};

} // namespace gl

namespace std
{
template<> inline void swap (gl::buffer& a, gl::buffer& b)
{
  a.swap (b);
}
}

namespace gl
{

// a helper to invoke the private bind::buffer, in cases it's really needed.
// usually, there's no need for user code to invoke it directly.
struct bind_vertex_buffer
{
  static void bind (const buffer& b)
  {
    assert (b.m_type == buffer::vertex);
    b.bind ();
  }
};

struct bind_index_buffer
{
  static void bind (const buffer& b)
  {
    assert (b.m_type == buffer::index);
    b.bind ();
  }
};

// ==========================================================================
// texture

class texture
{
public:
  enum address_mode
  {
    clamp = GL_CLAMP_TO_EDGE,
    mirror = GL_MIRRORED_REPEAT,
    wrap = GL_REPEAT
  };

  enum filter_mode
  {
    nearest = GL_NEAREST,
    linear = GL_LINEAR,
    nearest_mipmap_nearest = GL_NEAREST_MIPMAP_NEAREST,
    linear_mipmap_nearest = GL_LINEAR_MIPMAP_NEAREST,
    nearest_mipmap_linear = GL_NEAREST_MIPMAP_LINEAR,
    linear_mipmap_linear = GL_LINEAR_MIPMAP_LINEAR
  };

  texture (void)
  : m_size (0), m_format (pixel_format::invalid),
    m_obj (0) { }

  texture (pixel_format pf, const vec2<unsigned int>& size,
	   const void* data = nullptr, unsigned int stride_bytes = 0)
  : m_size (size), m_format (pf), m_obj (0)
  {
    if (size.x > 0 && size.y > 0 && pf != pixel_format::invalid)
    {
      glGenTextures (1, &m_obj);

    // this is a trap.  by default mipmaps are enabled.
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      upload (data, stride_bytes);
    }
  }

  texture (const texture&) = delete;
  texture& operator = (const texture&) = delete;

  texture (texture&& rhs)
  : m_size (rhs.m_size), m_format (rhs.m_format),
    m_obj (rhs.m_obj)
  {
    rhs.m_size = { 0 };
    rhs.m_format = pixel_format::invalid;
    rhs.m_obj = 0;
  }

  texture& operator = (texture&& rhs)
  {
    if (this == &rhs)
      return *this;

    if (m_obj != 0)
      glDeleteTextures (1, &m_obj);

    m_size = rhs.m_size;
    m_format = rhs.m_format;
    m_obj = rhs.m_obj;

    rhs.m_size = { 0 };
    rhs.m_format = pixel_format::invalid;
    rhs.m_obj = 0;

    return *this;
  }

  ~texture (void)
  {
    if (m_obj != 0)
      glDeleteTextures (1, &m_obj);
  }

  bool empty (void) const { return m_size.x == 0 || m_size.y == 0 || m_obj == 0; }
  const vec2<unsigned int>& size (void) const { return m_size; }
  pixel_format format (void) const { return m_format; }

  void upload (const void* data, unsigned int stride_bytes = 0)
  {
    if (empty ())
      return;

    bind ();
    set_upload_stride (stride_bytes);

    glTexImage2D (GL_TEXTURE_2D, 0, m_format.gl_internal_fmt (), m_size.x, m_size.y, 0,
		  m_format.gl_fmt (), m_format.gl_type (), data);
  }

  void upload (const void* data,
	       const vec2<int>& dst_xy, const vec2<unsigned int>& size,
	       unsigned int stride_bytes = 0)
  {
    if (empty ())
      return;

    bind ();
    set_upload_stride (stride_bytes);

    if (dst_xy.x == 0 && dst_xy.y == 0
	&& size.x == m_size.x && size.y == m_size.y)
      glTexImage2D (GL_TEXTURE_2D, 0, m_format.gl_internal_fmt (), m_size.x, m_size.y, 0,
		    m_format.gl_fmt (), m_format.gl_type (), data);
    else
    {
      // GL textures have y = 0 = bottom ... but we want y = 0 = top,
      // so have to mirror the y offset coordinate.
      glTexSubImage2D (GL_TEXTURE_2D, 0, dst_xy.x, m_size.y - size.y - dst_xy.y,
		       size.x, size.y, m_format.gl_fmt (), m_format.gl_type (), data);
    }
  }

  void set_address_mode_u (address_mode u)
  {
    if (empty ())
      return;
    bind ();
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, u);
  }

  void set_address_mode_v (address_mode v)
  {
    if (empty ())
      return;
    bind ();
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, v);
  }

  void set_min_filter (filter_mode f)
  {
    if (empty ())
      return;
    bind ();
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, f);
  }

  void set_mag_filter (filter_mode f)
  {
    if (empty ())
      return;
    bind ();
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, f);
  }

  void generate_mipmaps (void)
  {
    if (empty ())
      return;

    bind ();
    glGenerateMipmap (GL_TEXTURE_2D);
  }

  void bind (int texunit = 0) const
  {
    glActiveTexture (GL_TEXTURE0 + texunit);
    glBindTexture (GL_TEXTURE_2D, m_obj);
  }

  void swap (texture& rhs)
  {
    std::swap (m_size, rhs.m_size);
    std::swap (m_format, rhs.m_format);
    std::swap (m_obj, rhs.m_obj);
  }

private:
  // static unsigned int g_cur_bound[MAX_TEXUNITS];

  // should remember the texture unit this texture is bound to
  // this allows for quick unbinding and also restoring the current binding
  // after invoking functions on the texture, like upload.
  // -1 = invalid / not bound to a texunit.
  // int m_cur_texunit;
  vec2<unsigned int> m_size;
  pixel_format m_format;
  unsigned int m_obj;

  void set_upload_stride (unsigned int stride_bytes)
  {
    if (stride_bytes == 0)
    {
      glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
    }
    else
    {
      // GL_UNPACK_ALIGNMENT is the byte alignment (1,2,4,8) of each image line.
      // GL_UNPACK_ROW_LENGTH is the image stride in pixels.

      // 24 bpp = 3 bytes / pixel
      // image width = 61 pixels = 183 bytes
      // image stride = 1024
      // unpack alignment = 8
      // unpack row length = 1024 / 3 = 341

      unsigned int stride_bytes_1 = 1;
      if ((stride_bytes & 1) == 0)
	stride_bytes_1 = 2;
      if ((stride_bytes & 3) == 0)
	stride_bytes_1 = 4;
      if ((stride_bytes & 7) == 0)
	stride_bytes_1 = 8;

      // try to avoid the division...
      uint8_t bpp = m_format.bytes_per_pixel ();
      unsigned int stride_pixels;
      if (bpp == 1)
	stride_pixels = stride_bytes;
      else if (bpp == 2)
	stride_pixels = stride_bytes / 2;
      else if (bpp == 4)
	stride_pixels = stride_bytes / 4;
      else if (bpp == 8)
	stride_pixels = stride_bytes / 8;
      else
	stride_pixels = stride_bytes / bpp;

      glPixelStorei (GL_UNPACK_ALIGNMENT, stride_bytes_1);
      glPixelStorei (GL_UNPACK_ROW_LENGTH, stride_pixels);
    }
  }
};

} // namespace gl

namespace std
{

template<> inline void swap (gl::texture& a, gl::texture& b)
{
  a.swap (b);
}

} // namespace std

namespace gl
{

// ==========================================================================
// vertex buffer attribute binding/description

// extract vertex component offset
template <typename VertexType, typename ComponentType>
inline unsigned int vertex_component_offset (ComponentType (VertexType::*comp))
{
  return (const char*)(&(((VertexType*)NULL)->*comp)) - (const char*)(NULL);
}

// extract vertex component part count
template <typename T>
struct vertex_component_part_count { static const unsigned int value = 1; };

template <typename T, unsigned int N>
struct vertex_component_part_count<T[N]> { static const unsigned int value = N; };

template <typename T>
struct vertex_component_part_count<vec2<T> > { static const unsigned int value = 2; };

template <typename T>
struct vertex_component_part_count<vec3<T> > { static const unsigned int value = 3; };

template <typename T>
struct vertex_component_part_count<vec4<T> > { static const unsigned int value = 4; };


// extract vertex component base type
template <typename T> struct vertex_component_base_type_1
{
  static GLenum value (void)
  {
    static_assert ((std::is_same<T, T>::value & false),
                   "unsupported vertex component base type");
    return 0;
  }
};

template <> struct vertex_component_base_type_1<int8_t> {  static GLenum value (void) { return GL_BYTE; } };
template <> struct vertex_component_base_type_1<uint8_t> {  static GLenum value (void) { return GL_UNSIGNED_BYTE; } };
template <> struct vertex_component_base_type_1<int16_t> {  static GLenum value (void) { return GL_SHORT; } };
template <> struct vertex_component_base_type_1<uint16_t> {  static GLenum value (void) { return GL_UNSIGNED_SHORT; } };
template <> struct vertex_component_base_type_1<float> {  static GLenum value (void) { return GL_FLOAT; } };

template <typename VecType> struct vertex_component_base_type_1<vec2<VecType> >
{
  static GLenum value (void) { return vertex_component_base_type_1<VecType>::value (); }
};

template <typename VecType> struct vertex_component_base_type_1<vec3<VecType> >
{
  static GLenum value (void) { return vertex_component_base_type_1<VecType>::value (); }
};

template <typename VecType> struct vertex_component_base_type_1<vec4<VecType> >
{
  static GLenum value (void) { return vertex_component_base_type_1<VecType>::value (); }
};

template <typename T, unsigned int N> struct vertex_component_base_type_1<T[N]>
{
  static GLenum value (void) { return vertex_component_base_type_1<T>::value (); }
};

template <typename VertexType, typename ComponentType>
inline GLenum vertex_component_base_type (ComponentType (VertexType::*))
{
  return vertex_component_base_type_1<ComponentType>::value ();
}

enum vertex_attrib_normalize_t
{
  vertex_attrib_direct = GL_FALSE,
  vertex_attrib_normalize = GL_TRUE
};

template <typename VertexType, typename ComponentType>
struct vertex_attrib_desc
{
  const buffer* m_buffer;
  const GLenum m_base_type;
  const unsigned int m_offset_bytes;
  const vertex_attrib_normalize_t m_normalize;

  vertex_attrib_desc (const buffer* b, GLenum t, unsigned int off,
                      vertex_attrib_normalize_t n)
  : m_buffer (b), m_base_type (t), m_offset_bytes (off), m_normalize (n) { }

  const buffer& vertex_buffer (void) const { return *m_buffer; }

  unsigned int part_count (void) const
  {
    return vertex_component_part_count<ComponentType>::value;
  }
  unsigned int stride_bytes (void) const { return sizeof (VertexType); }
  unsigned int offset_bytes (void) const { return m_offset_bytes; }
  GLenum base_type (void) const { return m_base_type; }
  vertex_attrib_normalize_t normalize (void) const { return m_normalize; }
};


template <typename VertexType, typename ComponentType>
inline vertex_attrib_desc<VertexType, ComponentType>
vertex_attrib (ComponentType (VertexType::*comp),
               vertex_attrib_normalize_t nrm = vertex_attrib_direct,
               size_t offset_bytes = 0)
{
  return vertex_attrib_desc<VertexType, ComponentType> (NULL,
    vertex_component_base_type (comp),
    vertex_component_offset (comp) + offset_bytes, nrm);
}

template <typename VertexType, typename ComponentType>
inline vertex_attrib_desc<VertexType, ComponentType>
vertex_attrib (const buffer& b, ComponentType (VertexType::*comp),
               vertex_attrib_normalize_t nrm = vertex_attrib_direct,
               size_t offset_bytes = 0)
{
  return vertex_attrib_desc<VertexType, ComponentType> (&b,
    vertex_component_base_type (comp),
    vertex_component_offset (comp) + offset_bytes, nrm);
}

// ==========================================================================
// GL shader / shader parameter (uniform, attribute) wrappers

class shader
{
public:

  // -----------------------------
  struct sampler2D { };

  struct highp { };
  struct mediump { };
  struct lowp { };
  struct undefined_precision { };


  template<typename T> struct precision_string_spec
  {
    static const char* value (void)
    {
      static_assert ((std::is_same<T, T>::value & false), "");
      return "";
    }
  };

  template<typename T, typename Precision> struct precision_string
  {
    static const char* value (void)
    {
      // at least on some MESA GL versions (8.0.4-ubuntu0.7), there will be an error if a
      // sampler2D (array) has a precision specifier.  thus, for all sampler2D
      // variables, do not emit a precision specifier.
      return std::is_same< typename std::remove_extent<T>::type,
                                sampler2D >::value
             ? "" : precision_string_spec<Precision>::value ();
    }
  };

  template<typename T> struct gltype_string_spec
  {
    static const char* value (void)
    {
      static_assert ((std::is_same<T, T>::value & false), "");
      return "";
    }
  };

  template<typename T> struct gltype_string
  {
    static const char* value (void)
    {
      return gltype_string_spec<typename std::remove_extent<T>::type>::value ();
    }
  };

  template<typename T> struct gltype_extent_string
  {
    static const char* value (void)
    {
      typedef std::numeric_limits< typename std::extent <T>::value_type > t_limits;

      // take sign bit, zero terminating character and '[' ']' characters
      // into account.
      static char s[t_limits::digits10 + 1 + t_limits::is_signed + 2 + 1];

      // dummy variable to get static initialization of the formatted string.
      static int x = std::extent<T>::value == 0
                     ? 0
                     : t_limits::is_signed
                       ? std::sprintf (s, "[%i]", (int)std::extent<T>::value)
                       : std::sprintf (s, "[%u]", (unsigned int)std::extent<T>::value);

      // "x - x" avoids unused variable warning in this case.
      return std::extent<T>::value > 0 ? (s + (x - x)) : "";
    }
  };


  // -----------------------------
  class parameter
  {
    static std::string empty_decl_func (const char*) { return ""; }

  public:
    enum { INVALID_LOCATION = -1 };

    parameter (void)
    : m_name (""), m_location (INVALID_LOCATION), m_makedecl (empty_decl_func) { }

    const char* name (void) const { return m_name; }
    int location (void) const { return m_location; }
    std::string make_decl (void) const { return m_makedecl (m_name); }

  protected:
    const char* m_name;
    int m_location;
    std::string (*m_makedecl)(const char* shader_variable_name);

  friend class shader;
  };

  // -----------------------------
  template <typename T, typename Precision>
  class uniform_base : public parameter
  {
  protected:
     static_assert (std::rank<T>::value <= 1, "unsupported array dimension > 1");

     static std::string make_decl_func (const char* name)
     {
       return std::string ("uniform ")
              + precision_string<T, Precision>::value () + ' '
              + gltype_string<T>::value () + ' '
              + name + gltype_extent_string<T>::value () + ';';
     }

     uniform_base (void) { m_makedecl = &make_decl_func; }
  };

  template <typename T, typename Precision>
  struct uniform : public uniform_base<T, Precision>
  {
     // trigger compile time error
     template <typename Value> void operator = (const Value&)
     {
       static_assert ((std::is_same<Value, Value>::value & false), "");
     }
  };

  // -----------------------------
  template <typename T, typename Precision>
  class attribute_base : public parameter
  {
  protected:
    static std::string make_decl_func (const char* name)
    {
      return std::string ("attribute ")
             + precision_string<T, Precision>::value () + ' '
             + gltype_string<T>::value () + ' '
             + name + gltype_extent_string<T>::value () + ';';
    }

    attribute_base (void) { m_makedecl = &make_decl_func; }
  };

  template <typename T, typename Precision>
  struct attribute : public attribute_base<T, Precision>
  {
     // trigger compile time error
     template <typename Value> void operator = (const Value&)
     {
       static_assert ((std::is_same<Value, Value>::value & false), "");
     }
  };

  // -----------------------------

  enum { INVALID_PROGRAM_ID = 0 };

  virtual ~shader (void);

  void activate (void);

protected:
  shader (void)
  : m_program (INVALID_PROGRAM_ID),
    m_vertex_shader (INVALID_PROGRAM_ID),
    m_fragment_shader (INVALID_PROGRAM_ID),
    m_tried_compile (false) { }

  virtual const char* vertex_shader_text_str (void) = 0;
  virtual const char* fragment_shader_text_str (void) = 0;
  virtual void register_parameters (void) { }

  #define vertex_shader_text(str)\
  virtual const char* vertex_shader_text_str (void) override { return #str ; }

  #define vertex_shader_text_decl\
  virtual const char* vertex_shader_text_str (void) override

  #define vertex_shader_text_def(cls,str)\
  const char* cls :: vertex_shader_text_str (void) { return #str ; }

  #define fragment_shader_text(str)\
  virtual const char* fragment_shader_text_str (void) override { return #str ; }

  #define fragment_shader_text_decl\
  virtual const char* fragment_shader_text_str (void) override

  #define fragment_shader_text_def(cls,str)\
  const char* cls :: fragment_shader_text_str (void) { return #str ; }

  template <typename T, typename P> void
  add (uniform_base<T, P>& p, const char* name)
  {
    parameter* pp = &p;
    pp->m_name  = name;
    m_uniforms.push_back (pp);
  }

  template <typename T, typename P> void
  add (attribute_base<T, P>& p, const char* name)
  {
    parameter* pp = &p;
    pp->m_name = name;
    m_attributes.push_back (pp);
  }

  #define named_parameter(x) add (x, #x)

  std::vector<parameter*> m_uniforms;
  std::vector<parameter*> m_attributes;

private:
  static GLuint g_cur_program;

  void check_try_compile (void);

  GLuint m_program;
  GLuint m_vertex_shader;
  GLuint m_fragment_shader;
  bool m_tried_compile;
};


// specializations

#ifdef USE_GL
template<> struct shader::precision_string_spec<shader::highp> { static const char* value (void) { return ""; } };
template<> struct shader::precision_string_spec<shader::mediump> { static const char* value (void) { return ""; } };
template<> struct shader::precision_string_spec<shader::lowp> { static const char* value (void) { return ""; } };
#endif

#ifdef USE_GLES2
template<> struct shader::precision_string_spec<shader::highp> { static const char* value (void) { return "highp"; } };
template<> struct shader::precision_string_spec<shader::mediump> { static const char* value (void) { return "mediump"; } };
template<> struct shader::precision_string_spec<shader::lowp> { static const char* value (void) { return "lowp"; } };
#endif

template<> struct shader::precision_string_spec<shader::undefined_precision> { static const char* value (void) { return ""; } };

template<> struct shader::gltype_string_spec<float> { static const char* value (void) { return "float"; } };
template<> struct shader::gltype_string_spec<int> { static const char* value (void) { return "int"; } };
template<> struct shader::gltype_string_spec<bool> { static const char* value (void) { return "bool"; } };
template<> struct shader::gltype_string_spec<shader::sampler2D> { static const char* value (void) { return "sampler2D"; } };
template<> struct shader::gltype_string_spec<vec2<float> > { static const char* value (void) { return "vec2"; } };
template<> struct shader::gltype_string_spec<vec3<float> > { static const char* value (void) { return "vec3"; } };
template<> struct shader::gltype_string_spec<vec4<float> > { static const char* value (void) { return "vec4"; } };
template<> struct shader::gltype_string_spec<mat4<float> > { static const char* value (void) { return "mat4"; } };


// -----------------------------
// uniform

// float
template <typename Precision>
struct shader::uniform<float, Precision>
  : public shader::uniform_base<float, Precision>
{
  void operator = (float val)
  {
    int i = uniform_base<float, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1f (i, val);
  }
};

template <unsigned int N, typename Precision>
struct shader::uniform<float[N], Precision>
  : public shader::uniform_base<float[N], Precision>
{
  void operator = (const float* val)
  {
    int i = uniform_base<float[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1fv (i, N, val);
  }
  void operator = (const std::array<float, N>& val)
  {
    int i = uniform_base<float[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1fv (i, N, val.data ());
  }
  void operator = (const std::vector<float>& val)
  {
    int i = uniform_base<float[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1fv (i, std::min ((unsigned int)val.size (), N), &val.front ());
  }
};

// vec2<float>
template <typename Precision>
struct shader::uniform<vec2<float>, Precision>
  : public shader::uniform_base<vec2<float>, Precision>
{
  void operator = (const vec2<float>& val)
  {
    int i = uniform_base<vec2<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform2f (i, val.x, val.y);
  }
};

template <unsigned int N, typename Precision>
struct shader::uniform<vec2<float>[N], Precision>
  : public shader::uniform_base<vec2<float>[N], Precision>
{
  void operator = (const vec2<float>* val)
  {
    int i = uniform_base<vec2<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform2fv (i, N, (const float*)val);
  }
  void operator = (const std::array<vec2<float>, N>& val)
  {
    int i = uniform_base<vec2<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform2fv (i, N, (const float*)val.data ());
  }
  void operator = (const std::vector<vec2<float> >& val)
  {
    int i = uniform_base<vec2<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform2fv (i, std::min ((unsigned int)val.size (), N), (const float*)&val.front ());
  }
};

// vec3<float>
template <typename Precision>
struct shader::uniform<vec3<float>, Precision>
  : public shader::uniform_base<vec3<float>, Precision>
{
  void operator = (const vec3<float>& val)
  {
    int i = uniform_base<vec3<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform3f (i, val.x, val.y, val.z);
  }
};

template <unsigned int N, typename Precision>
struct shader::uniform<vec3<float>[N], Precision>
  : public shader::uniform_base<vec3<float>[N], Precision>
{
  void operator = (const vec3<float>* val)
  {
    int i = uniform_base<vec3<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform3fv (i, N, (const float*)val);
  }
  void operator = (const std::array<vec3<float>, N>& val)
  {
    int i = uniform_base<vec3<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform3fv (i, N, (const float*)val.data ());
  }
  void operator = (const std::vector<vec3<float> >& val)
  {
    int i = uniform_base<vec3<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform3fv (i, std::min ((unsigned int)val.size (), N), (const float*)&val.front ());
  }
};

// vec4<float>
template <typename Precision>
struct shader::uniform<vec4<float>, Precision>
  : public shader::uniform_base<vec4<float>, Precision>
{
  void operator = (const vec4<float>& val)
  {
    int i = uniform_base<vec4<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform4f (i, val.x, val.y, val.z, val.w);
  }
};

template <unsigned int N, typename Precision>
struct shader::uniform<vec4<float>[N], Precision>
  : public shader::uniform_base<vec4<float>[N], Precision>
{
  void operator = (const vec4<float>* val)
  {
    int i = uniform_base<vec4<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform4fv (i, N, (const float*)val);
  }
  void operator = (const std::array<vec4<float>, N>& val)
  {
    int i = uniform_base<vec4<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform4fv (i, N, (const float*)val.data ());
  }
  void operator = (const std::vector<vec4<float> >& val)
  {
    int i = uniform_base<vec4<float>[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform4fv (i, std::min ((unsigned int)val.size (), N), (const float*)&val.front ());
  }
};

// mat4<float>
template <typename Precision>
struct shader::uniform<mat4<float>, Precision>
  : public shader::uniform_base<mat4<float>, Precision>
{
  void operator = (const mat4<float>& m)
  {
    int i = uniform_base<mat4<float>,Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniformMatrix4fv (i, 1, GL_FALSE, m.m);
  }
};

// sampler2D
template <typename Precision>
struct shader::uniform<shader::sampler2D, Precision>
  : public shader::uniform_base<shader::sampler2D, Precision>
{
  void operator = (int val)
  {
    int i = uniform_base<sampler2D, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1i (i, val);
  }
};

template <unsigned int N, typename Precision>
struct shader::uniform<shader::sampler2D[N], Precision>
  : public shader::uniform_base<shader::sampler2D[N], Precision>
{
  void operator = (const int* val)
  {
    int i = uniform_base<sampler2D[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1iv (i, N, val);
  }
  void operator = (const std::array<int, N>& val)
  {
    int i = uniform_base<sampler2D[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
      glUniform1iv (i, N, val.data ());
  }
  void operator = (const std::vector<int>& val)
  {
    int i = uniform_base<sampler2D[N], Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    glUniform1iv (i, std::min ((unsigned int)val.size (), N), &val.front ());
  }
};

// -----------------------------
// attribute

// float
template <typename Precision>
struct shader::attribute<float, Precision>
  : public shader::attribute_base<float, Precision>
{
  void operator = (float val)
  {
    int i = attribute_base<float, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      glDisableVertexAttribArray (i);
      glVertexAttrib1f (i, val);
    }
  }

  template <typename VertexType, typename ComponentType>
  void operator = (const vertex_attrib_desc<VertexType, ComponentType>& desc)
  {
    static_assert (vertex_component_part_count<ComponentType>::value == 1,
                   "vertex component count mismatch");

    int i = attribute_base<float, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      bind_vertex_buffer::bind (desc.vertex_buffer ());
      glVertexAttribPointer (i, desc.part_count (), desc.base_type (),
                             desc.normalize (), desc.stride_bytes (),
                             (const void*)desc.offset_bytes ());
      glEnableVertexAttribArray (i);
    }
  }
};

// vec2<float>
template <typename Precision>
struct shader::attribute<vec2<float>, Precision>
  : public shader::attribute_base<vec2<float>, Precision>
{
  void operator = (const vec2<float>& val)
  {
    int i = attribute_base<vec2<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      glDisableVertexAttribArray (i);
      glVertexAttrib2f (i, val.x, val.y);
    }
  }

  template <typename VertexType, typename ComponentType>
  void operator = (const vertex_attrib_desc<VertexType, ComponentType>& desc)
  {
    static_assert (vertex_component_part_count<ComponentType>::value == 2,
                   "vertex component count mismatch");

    int i = attribute_base<vec2<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      bind_vertex_buffer::bind (desc.vertex_buffer ());
      glEnableVertexAttribArray (i);
      glVertexAttribPointer (i, desc.part_count (), desc.base_type (),
                             desc.normalize (), desc.stride_bytes (),
                             (const void*)(size_t)desc.offset_bytes ());
    }
  }
};

// vec3<float>
template <typename Precision>
struct shader::attribute<vec3<float>, Precision>
  : public shader::attribute_base<vec3<float>, Precision>
{
  void operator = (const vec3<float>& val)
  {
    int i = attribute_base<vec3<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      glDisableVertexAttribArray (i);
      glVertexAttrib3f (i, val.x, val.y, val.z);
    }
  }

  template <typename VertexType, typename ComponentType>
  void operator = (const vertex_attrib_desc<VertexType, ComponentType>& desc)
  {
    static_assert (vertex_component_part_count<ComponentType>::value == 3,
                   "vertex component count mismatch");

    int i = attribute_base<vec3<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      bind_vertex_buffer::bind (desc.vertex_buffer ());
      glEnableVertexAttribArray (i);
      glVertexAttribPointer (i, desc.part_count (), desc.base_type (),
                             desc.normalize (), desc.stride_bytes (),
                             (const void*)(size_t)desc.offset_bytes ());
    }
  }
};

// vec4<float>
template <typename Precision>
struct shader::attribute<vec4<float>, Precision>
  : public shader::attribute_base<vec4<float>, Precision>
{
  void operator = (const vec4<float>& val)
  {
    int i = attribute_base<vec4<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      glDisableVertexAttribArray (i);
      glVertexAttrib4f (i, val.x, val.y, val.z, val.w);
    }
  }

  template <typename VertexType, typename ComponentType>
  void operator = (const vertex_attrib_desc<VertexType, ComponentType>& desc)
  {
    static_assert (vertex_component_part_count<ComponentType>::value == 4,
                   "vertex component count mismatch");

    int i = attribute_base<vec4<float>, Precision>::m_location;
    if (i != shader::parameter::INVALID_LOCATION)
    {
      bind_vertex_buffer::bind (desc.vertex_buffer ());
      glEnableVertexAttribArray (i);
      glVertexAttribPointer (i, desc.part_count (), desc.base_type (),
                             desc.normalize (), desc.stride_bytes (),
                             (const void*)(size_t)desc.offset_bytes ());
    }
  }
};

// ==========================================================================
// wrappers for drawing primitives

enum primitive_type_t
{
  points = GL_POINTS,
  line_strip = GL_LINE_STRIP,
  line_loop = GL_LINE_LOOP,
  lines = GL_LINES,
  triangle_strip = GL_TRIANGLE_STRIP,
  triangle_fan = GL_TRIANGLE_FAN,
  triangles = GL_TRIANGLES
};

inline unsigned int calc_primitive_count (primitive_type_t t, unsigned int count)
{
  switch (t)
  {
    case points: return count;
    case line_strip: return count - 1;
    case line_loop: return count;
    case lines: return count / 2;
    case triangle_strip: return count - 2;
    case triangle_fan: return count - 2;
    case triangles: return count / 3;
    default: return 0;
  }
}

struct index_type
{
  int gl_type;
  int size;

  index_type (void) : gl_type (0), size (0) { }
  index_type (int t, int s) : gl_type (t), size (s) { }
};

template <typename T> struct make_index_type;

template <> struct make_index_type<uint8_t> : index_type
{
  make_index_type<uint8_t> (void) : index_type (GL_UNSIGNED_BYTE, 1) { }
};

template <> struct make_index_type<uint16_t> : index_type
{
  make_index_type<uint16_t> (void) : index_type (GL_UNSIGNED_SHORT, 2) { }
};

template <> struct make_index_type<uint32_t> : index_type
{
  make_index_type<uint32_t> (void) : index_type (GL_UNSIGNED_INT, 4) { }
};

inline void draw (primitive_type_t t, unsigned int vtxsize,
                  unsigned int count, unsigned int offset = 0)
{
  glDrawArrays (t, offset, count);
  cur_frame_stats.vertex_count += count;
  cur_frame_stats.vertex_byte_count += count * vtxsize;
  cur_frame_stats.primitive_count += calc_primitive_count (t, count);
  cur_frame_stats.draw_call_count += 1;
}

inline void draw_indexed (primitive_type_t t, unsigned int vtxsize,
                          const buffer& idxbuffer,
                          const index_type& it,
                          unsigned int count, unsigned int offset = 0)
{
  bind_index_buffer::bind (idxbuffer);
  glDrawElements (t, count, it.gl_type, (const void*)(size_t)offset);
  cur_frame_stats.primitive_count += calc_primitive_count (t, count);
  cur_frame_stats.vertex_count += count;
  cur_frame_stats.vertex_byte_count += count * vtxsize;
  cur_frame_stats.index_count += count;
  cur_frame_stats.index_byte_count += count * it.size;
  cur_frame_stats.draw_call_count += 1;
}

} // namespace gl
#endif // includeguard_gl_hpp_includeguard
