#include <EGL/egl.h>

// #include <EGL/eglplatform.h>

#include "gl.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <cassert>

#include "gldev.hpp"
#include "display.hpp"
#include "img/pixel_format.hpp"

using img::pixel_format;
using img::ds_format;

#ifdef USE_GLES2

PFNGLMAPBUFFEROESPROC glMapBuffer = 0;
PFNGLUNMAPBUFFEROESPROC glUnmapBuffer = 0;
PFNGLDISCARDFRAMEBUFFEREXTPROC glInvalidateFramebuffer = 0;

static void dummy_glInvalidateFramebuffer (GLenum,GLsizei,const GLenum*)
{
  // the glDiscardFramebufferEXT PVR extension is an optional hint.  if the
  // function is not there, just do nothing.  this way we avoid checks for
  // extension function presence in the user code.
  // in GLES3 the extension became standardized under the name
  // glInvalidateFramebuffer.
}

#endif

struct gldev_egl : gldev
{
  bool m_init_ok;
  EGLNativeDisplayType m_native_display;
  EGLDisplay m_display;
  EGLConfig m_config;
  EGLContext m_context;
  EGLSurface m_surface;
  EGLint m_native_visual_id;

  gldev_egl (display& d, pixel_format pf, ds_format ds, int multisample)
  {
    m_init_ok = false;
    m_native_display = (EGLNativeDisplayType)d.handle ();
    m_display = 0;
    m_config = 0;
    m_context = 0;
    m_surface = 0;
    m_native_visual_id = 0;

    m_display = eglGetDisplay (m_native_display);

    if (m_display == EGL_NO_DISPLAY)
    {
      std::cerr << "eglGetDisplay (" << m_native_display << ") NG" << std::endl;
      std::cerr << "trying EGL_DEFAULT_DISPLAY" << std::endl;
      m_display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
    }

    assert (m_display != EGL_NO_DISPLAY);

    if (multisample < 2)
      multisample = 1;

    int major = -1;
    int minor = -1;
    if (!eglInitialize (m_display, &major, &minor))
      std::cerr << "eglInitialize NG" << std::endl;
    else
      std::cerr << "eglInitialize OK: " << major << '.' << minor << std::endl;

    if (!eglBindAPI (
    #ifdef USE_GL
      EGL_OPENGL_API
    #elif USE_GLES2
      EGL_OPENGL_ES_API
    #endif
       ))
    {
      std::cerr << "eglBindAPI NG" << std::endl;
      return;
    }

    std::cerr << "eglBindAPI OK" << std::endl;

    const EGLint config_attribs[] =
    {
/*
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_STENCIL_SIZE, 8,
*/
      #if USE_GL
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
      #elif USE_GLES2
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      #endif
        EGL_NONE
    };

    EGLint config_count = 0;
    if (!eglChooseConfig (m_display, config_attribs, 0, 0, &config_count))
    {
      std::cerr << "eglChooseConfig (1) NG" << std::endl;
      return;
    }
    if (config_count == 0)
    {
      std::cerr << "eglChooseConfig (1) NG (no configs)" << std::endl;
      return;
    }

    std::cerr << "eglChooseConfig (1) OK: " << config_count << std::endl;

    std::vector<EGLConfig> configs (config_count);
    if (!eglChooseConfig (m_display, config_attribs, &configs.front (),
                          config_count, &config_count))
    {
      std::cerr << "eglChooseConfig (2) NG" << std::endl;
      return;
    }

    if (config_count == 0)
    {
      std::cerr << "eglChooseConfig (2) NG (no configs)" << std::endl;
      return;
    }

    int use_cfg = -1;

    for (int i = 0; i < (int)configs.size (); ++i)
    {
      int buf, red, green, blue, alpha, depth, stencil, id, native_id,
          sample_bufs, samples;

      eglGetConfigAttrib (m_display, configs[i], EGL_CONFIG_ID, &id);
      bool native_id_ok = eglGetConfigAttrib (m_display, configs[i], EGL_NATIVE_VISUAL_ID, &native_id);
      eglGetConfigAttrib (m_display, configs[i], EGL_BUFFER_SIZE, &buf);
      eglGetConfigAttrib (m_display, configs[i], EGL_RED_SIZE, &red);
      eglGetConfigAttrib (m_display, configs[i], EGL_GREEN_SIZE, &green);
      eglGetConfigAttrib (m_display, configs[i], EGL_BLUE_SIZE, &blue);
      eglGetConfigAttrib (m_display, configs[i], EGL_ALPHA_SIZE, &alpha);
      eglGetConfigAttrib (m_display, configs[i], EGL_DEPTH_SIZE, &depth);
      eglGetConfigAttrib (m_display, configs[i], EGL_STENCIL_SIZE, &stencil);
      eglGetConfigAttrib (m_display, configs[i], EGL_SAMPLE_BUFFERS, &sample_bufs);
      eglGetConfigAttrib (m_display, configs[i], EGL_SAMPLES, &samples);

      if (samples < 2)
         samples = 1;

      const char* prefix_str = "";

      if (use_cfg == -1
          && native_id_ok
          && red == (int)pf.r_bits () && green == (int)pf.g_bits ()
          && blue == (int)pf.b_bits () && alpha == (int)pf.a_bits ()
          && depth == (int)ds.d_bits () && stencil == (int)ds.s_bits ()
          && multisample == samples)
      {
        use_cfg = i;
        prefix_str = ">> using ";
      }

      std::cerr << "\n" << prefix_str << "config " << i
                << "\n  id: " << id << " native id: " << native_id
                << "\n  buffer size: " << buf
                << " rgba_" << red << '_' << green << '_' << blue << '_' << alpha
                << " ds_" << depth << '_' << stencil
                << " sample buffers: " << sample_bufs << " samples: " << samples
                << std::endl;
    }

    if (use_cfg == -1)
    {
      std::cerr << "no matching config found" << std::endl;
      return;
    }

    m_config = configs[use_cfg];

    if (!eglGetConfigAttrib (m_display, m_config, EGL_NATIVE_VISUAL_ID,
                             &m_native_visual_id))
    {
      std::cerr << "eglGetConfigAttrib EGL_NATIVE_VISUAL_ID NG" << std::endl;
      return;
    }

    std::cerr << "eglGetConfigAttrib OK: " << m_native_visual_id << std::endl;
    
    m_init_ok = true;
  }

  virtual ~gldev_egl (void) override
  {
    if (m_surface && m_display)
      eglDestroySurface (m_display, m_surface);

    if (m_context && m_display)
      eglDestroyContext (m_display, m_context);

    if (m_display)
      eglTerminate (m_display);
  }

  virtual bool valid (void) const override { return m_init_ok; }

  virtual int native_visual_id (void) const override
  {
    return m_native_visual_id;
  }

  virtual void init_surface (window& ww) override
  {
    if (!valid ())
      return;

    EGLNativeWindowType w = (EGLNativeWindowType)ww.handle ();
    m_surface = eglCreateWindowSurface (m_display, m_config, w, 0);
    if (!m_surface)
    {
      std::cerr << "eglCreateWindowSurface NG" << std::endl;
      std::cerr << "trying default window" << std::endl;
      m_surface = eglCreateWindowSurface (m_display, m_config,
                                          (NativeWindowType)0, 0);
    }

    if (m_surface)
      std::cerr << "eglCreateWindowSurface OK" << std::endl;
    else
      std::cerr << "eglCreateWindowSurface NG" << std::endl;
  }

  virtual bool surface_valid (void) const { return m_surface; }

  virtual void create_context (int swapinterval)
  {
    if (!valid () || !surface_valid ())
      return;

    static const EGLint attr[] =
    {
    #ifdef USE_GLES2
      EGL_CONTEXT_CLIENT_VERSION, 2,
    #endif
      EGL_NONE
    };

    m_context = eglCreateContext (m_display, m_config, EGL_NO_CONTEXT, attr);
    if (!m_context)
    {
      std::cerr << "eglCreateContext NG" << std::endl;
      return;
    }

    std::cerr << "eglCreateContext OK" << std::endl;
    if (eglGetCurrentContext () != m_context)
    {  
      // init extensions here

      if (!eglMakeCurrent (m_display, m_surface, m_surface, m_context))
      {
        std::cerr << "eglMakeCurrent NG" << std::endl;
        return;
      }
      else
        std::cerr << "eglMakeCurrent OK" << std::endl;
    }

    if (!eglSwapInterval (m_display, swapinterval))
      std::cerr << "eglSwapInterval NG" << std::endl;
  }

  virtual bool context_valid (void) const override { return m_context; }

  virtual void swap_buffers (void) override
  {
    eglSwapBuffers (m_display, m_surface);
  }

  virtual void init_extensions (void) override
  {
    #ifdef USE_GLES2
      glMapBuffer = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress ("glMapBufferOES");
      glUnmapBuffer = (PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress ("glUnmapBufferOES");
      glInvalidateFramebuffer = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress ("glDiscardFramebufferEXT");

      if (glInvalidateFramebuffer == nullptr)
        glInvalidateFramebuffer = dummy_glInvalidateFramebuffer;
    #endif
  }
};

std::unique_ptr<gldev> gldev::make_new (display& d, pixel_format pf, ds_format ds,
					int multisample)
{
  return std::make_unique<gldev_egl> (d, pf, ds, multisample);
}
