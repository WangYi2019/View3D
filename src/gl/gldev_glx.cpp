#include "gl.hpp"

#include <X11/Xlib.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

#include <iostream>
#include <memory>
#include <vector>
#include <cassert>
#include <cstring>

#include "gldev.hpp"
#include "display.hpp"
#include "img/pixel_format.hpp"

namespace
{
PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT_;
PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA_;
PFNGLXGETSWAPINTERVALMESAPROC glXGetSwapIntervalMESA_;
}

struct gldev_glx : gldev
{
  bool m_init_ok;
  Display* m_disp;
  GLXFBConfig* m_all_configs;
  int m_all_configs_count;
  GLXFBConfig m_config;
  int m_native_visual_id;
  Window m_window;
  GLXContext m_context;

  gldev_glx (display& d, pixel_format pf, ds_format ds, int multisample)
  {
    m_init_ok = false;
    m_disp = (Display*)d.handle ();
    m_config = 0;
    m_native_visual_id = 0;
    m_window = 0;
    m_context = 0;

    if (multisample < 2)
      multisample = 1;

    if (m_disp == NULL)
    {
      std::cerr << "gldev_glx display is null" << std::endl;
      return;
    }

    int major = -1;
    int minor = -1;
    if (!glXQueryVersion (m_disp, &major, &minor))
    {
      std::cerr << "glXQueryVersion NG" << std::endl;
      return;
    }

    std::cerr << "glXQueryVersion OK: " << major << '.' << minor << std::endl;

    if ((major < 1) || (major == 1 && minor < 3))
    {
      std::cerr << "GL version NG" << std::endl;
      return;
    }

#if 1
    static int attr[] =
    {
      GLX_X_RENDERABLE, True,
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
//      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,   /* VMware NG  */
//      GLX_RED_SIZE, 8,
//      GLX_GREEN_SIZE, 8,
//      GLX_BLUE_SIZE, 8,
//      GLX_ALPHA_SIZE, 8,
//      GLX_DEPTH_SIZE, 24,   /* VMWare NG  */
//      GLX_DEPTH_SIZE, 16,   /* VMWare OK  */
//      GLX_STENCIL_SIZE, 8,
      GLX_DOUBLEBUFFER, True,
      None
    };
#endif

    m_all_configs = glXChooseFBConfig (m_disp, DefaultScreen (m_disp),
                                       attr, &m_all_configs_count);
    if (m_all_configs == NULL)
    {
      std::cerr << "glXChooseFBConfig NG" << std::endl;
      return;
    }

    if (m_all_configs_count < 1)
    {
      std::cerr << "glXChooseFBConfig NG (no configs)" << std::endl;
      return;
    }

    std::cerr << "glXChooseFBConfig OK: " << m_all_configs_count << std::endl;

    int use_cfg = -1;

    for (int i = 0; i < m_all_configs_count; ++i)
    {
      int buf = 0, red = 0, green = 0, blue = 0, alpha = 0, depth = 0,
          stencil = 0, id = 0, native_id = 0, sample_bufs = 0, samples = 0;

      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_FBCONFIG_ID, &id);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_VISUAL_ID, &native_id);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_BUFFER_SIZE, &buf);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_RED_SIZE, &red);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_GREEN_SIZE, &green);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_BLUE_SIZE, &blue);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_ALPHA_SIZE, &alpha);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_DEPTH_SIZE, &depth);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_STENCIL_SIZE, &stencil);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_SAMPLE_BUFFERS, &sample_bufs);
      glXGetFBConfigAttrib (m_disp, m_all_configs[i], GLX_SAMPLES, &samples);

      if (samples < 2)
        samples = 1;

      const char* prefix_str = "";

      if (use_cfg == -1
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

    m_config = m_all_configs[use_cfg];

    if (glXGetFBConfigAttrib (m_disp, m_config, GLX_VISUAL_ID,
                              &m_native_visual_id) != Success)
    {
      std::cerr << "glXGetFBConfigAttrib NG" << std::endl;
      return;
    }

    std::cerr << "glXGetFBConfigAttrib OK: " << m_native_visual_id << std::endl;

    m_init_ok = true;
  }

  virtual ~gldev_glx (void) override
  {
    if (m_context)
      glXDestroyContext (m_disp, m_context);
    XFree (m_all_configs);
  }

  virtual bool valid (void) const override { return m_init_ok; }

  virtual int native_visual_id (void) const override
  {
    return m_native_visual_id;
  }

  virtual void init_surface (window& w) override
  {
    if (!valid ())
      return;

    // assume that the window system is X11.
    m_window = (Window)w.handle ();
  }

  virtual bool surface_valid (void) const { return m_window != 0; }

  virtual void create_context (int swapinterval)
  {
    if (!valid () || !surface_valid ())
      return;

    m_context = glXCreateNewContext (m_disp, m_config, GLX_RGBA_TYPE, 0, True);
    if (m_context == NULL)
    {
      std::cerr << "glXCreateNewContext NG" << std::endl;
      return;
    }

    std::cerr << "glXCreateNewContext OK" << std::endl;

    if (glXGetCurrentContext () != m_context)
    {
      if (!glXMakeCurrent (m_disp, m_window, m_context))
      {
        std::cerr << "glXMakeCurrent NG" << std::endl;
        return;
      }
      else
        std::cerr << "glXMakeCurrent OK" << std::endl;
    }

    const char* extstr = glXQueryExtensionsString (m_disp, 0);
    if (extstr == NULL)
      extstr = "";

    if (std::strstr (extstr, "GLX_EXT_swap_control"))
      glXSwapIntervalEXT_ = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress ((const GLubyte*)"glXSwapIntervalEXT");

    if (std::strstr (extstr, "GLX_MESA_swap_control"))
    {
      glXSwapIntervalMESA_ = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddress ((const GLubyte*)"glXSwapIntervalMESA");
      glXGetSwapIntervalMESA_ = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddress ((const GLubyte*)"glXGetSwapIntervalMESA");
    }

    const unsigned int desired_swap = swapinterval;
    unsigned int actual_swap = -1;

    if (glXSwapIntervalEXT_)
    {
      glXSwapIntervalEXT_ (m_disp, m_window, desired_swap);
      glXQueryDrawable (m_disp, m_window, GLX_SWAP_INTERVAL_EXT, &actual_swap);
    }

    if (glXSwapIntervalMESA_)
    {
      glXSwapIntervalMESA_ (desired_swap);
      actual_swap = glXGetSwapIntervalMESA_ ();
    }

    if (actual_swap != desired_swap)
      std::cerr << "set swap interval NG (" << actual_swap << ")" << std::endl;
  }

  virtual bool context_valid (void) const override { return m_context != NULL; }

  virtual void swap_buffers (void) override
  {
    glXSwapBuffers (m_disp, m_window);
  }


  virtual void init_extensions (void) override
  {

  }
};

std::unique_ptr<gldev> gldev::make_new (display& d, pixel_format pf, ds_format ds,
					int multisample)
{
  return std::make_unique<gldev_glx> (d, pf, ds, multisample);
}
