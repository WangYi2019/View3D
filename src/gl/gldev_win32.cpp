
// The include order is important here.
// We don't define GL_GLEXT_PROTOTYPES so that we can declare the function
// names as variables, which are loaded at runtime but only in this translation
// unit.  All the other translation units which use the gl functions normally
// should see the extension prototypes.
#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>
#include <GL/glext.h>


#include <iostream>
#include <memory>
#include <vector>
#include <cassert>
#include <cstring>

#define concat1(x,y) x ## y
#define concat(x,y) concat1(x,y)

#define glextfunc(t,n) t concat(n, _);

#include "glext_funcs.x.hpp"

#undef glextfunc
#undef concat1
#undef concat

#include "gl.hpp"
#include "gldev.hpp"
#include "display.hpp"
#include "pixel_format.hpp"
#include "utils/utils.hpp"

struct gldev_win32 : gldev
{
  display& m_display;
  pixel_format m_pixel_format;
  ds_format m_ds_format;
  int m_multisample;

  GLuint m_native_visual_id;

  HDC m_hdc;
  HWND m_hwnd;
  HGLRC m_context;
  PIXELFORMATDESCRIPTOR m_pfd;

  gldev_win32 (display& d, pixel_format pf, ds_format ds, int multisample)
  : m_display (d), m_pixel_format (pf), m_ds_format (ds), m_multisample (multisample)
  {
    m_native_visual_id = 0;
    m_context = 0;
    m_hwnd = 0;
    m_hdc = 0;

    m_pfd =
    {
      sizeof (PIXELFORMATDESCRIPTOR),
      1,					// Version Number
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,			// Request An RGBA Format
      (unsigned char)m_pixel_format.bits_per_pixel (),	// color bits
      (unsigned char)m_pixel_format.r_bits (),  // red bits
      0,  // red shift
      (unsigned char)m_pixel_format.g_bits (),  // green bits
      0,  // green shift
      (unsigned char)m_pixel_format.b_bits (),  // blue bits
      0,  // blue shift
      (unsigned char)m_pixel_format.a_bits (),  // alpha bits
      0,  // alpha shift
      0,				// Accumulation Buffer
      0, 0, 0, 0,			// Accumulation Bits Ignored
      (unsigned char)m_ds_format.d_bits (),	// depth bits
      (unsigned char)m_ds_format.s_bits (),	// stencil bits
      0,				// Auxiliary Buffer
      PFD_MAIN_PLANE,			// Main Drawing Layer
      0,				// Reserved
      0, 0, 0				// Layer Masks Ignored
    };

    if (m_multisample < 2)
      m_multisample = 0;


    // create a temporary window and a temporary context to get the pixelformat
    // for multisampling.
    auto tmp_win = d.create_window (0, 640, 480);
    assert (tmp_win != nullptr);

    HWND hwnd = (HWND)tmp_win->handle ();
    HDC hdc = GetDC (hwnd);

    m_native_visual_id = ChoosePixelFormat (hdc, &m_pfd);
    std::cerr << "ChoosePixelFormat (1) = " << m_native_visual_id << std::endl;

    if (!SetPixelFormat (hdc, m_native_visual_id, &m_pfd))
      std::cerr << "SetPixelFormat (1) NG" << std::endl;

    auto ctx = wglCreateContext (hdc);
    if (!ctx)
      std::cerr << "wglCreateContext (1) NG" << std::endl;

    wglMakeCurrent (hdc, ctx);

    if (m_multisample)
    {
      PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
	(PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress ("wglChoosePixelFormatARB");
      if (wglChoosePixelFormatARB)
      {
	int iattr[] =
	{
	  WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	  WGL_COLOR_BITS_ARB, (int)m_pixel_format.bits_per_pixel (),
	  WGL_ALPHA_BITS_ARB, (int)m_pixel_format.a_bits (),
	  WGL_DEPTH_BITS_ARB, (int)m_ds_format.d_bits (),
	  WGL_STENCIL_BITS_ARB, (int)m_ds_format.s_bits (),
	  WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	  WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
	  WGL_SAMPLES_ARB, m_multisample,
	  0, 0
        };
 
	float fattr[] = { 0, 0 };
	int pixelformat = 0;
	unsigned int numformats = 0;

	bool valid = wglChoosePixelFormatARB (hdc, iattr, fattr, 1, &pixelformat, &numformats) != 0;

	if (!valid)
	  std::cerr << "wglChoosePixelFormatAGB NG" << std::endl;
	else
	{
	  std::cerr << "wglChoosePixelFormatARB = " << pixelformat << std::endl;
	  m_native_visual_id = pixelformat;
	}
      }
    }

    wglMakeCurrent (hdc, nullptr);
    wglDeleteContext (ctx);
    ReleaseDC (hwnd, hdc);
  }

  virtual ~gldev_win32 (void) override
  {
    if (m_hdc)
    {
      wglMakeCurrent (m_hdc, nullptr);
      wglDeleteContext (m_context);
      ReleaseDC (m_hwnd, m_hdc);
    }
  }

  virtual bool valid (void) const override
  {
    return m_native_visual_id != 0;
  }

  virtual int native_visual_id (void) const override
  {
    return m_native_visual_id;
  }

  virtual void init_surface (window& w) override
  {
    if (!valid ())
      return;

    m_hwnd = (HWND)w.handle ();
    m_hdc = GetDC (m_hwnd);

    if (!SetPixelFormat (m_hdc, m_native_visual_id, &m_pfd))
    {
      std::cout << "SetPixelFormat (2) NG" << std::endl;
      ReleaseDC (m_hwnd, m_hdc);
      m_hdc = 0;
    }
  }

  virtual bool surface_valid (void) const
  {
    return m_hdc != 0;
  }

  virtual void create_context (int swapinterval)
  {
    if (!valid () || !surface_valid ())
      return;

   m_context = wglCreateContext (m_hdc);
   if (!m_context)
   {
     std::cerr << "wglCreateContext (2) NG" << std::endl;
     return;
   }

   if (!wglMakeCurrent (m_hdc, m_context))
   {
     std::cerr << "wglMakeCurrent (2) NG" << std::endl;
     wglDeleteContext (m_context);
     m_context = 0;
   }
  }

  virtual bool context_valid (void) const override
  {
    return m_context != 0;
  }

  virtual void swap_buffers (void) override
  {
    SwapBuffers (m_hdc);
  }

  virtual void init_extensions (void) override
  {
    HMODULE m = LoadLibraryA ("opengl32.dll");

    auto resolve = [m] (const char* n)
    {
      auto f = wglGetProcAddress (n);
      if (f == (decltype(f))-1 || f == (decltype(f))0 || f == (decltype(f))1
	  || f == (decltype(f))2 || f == (decltype(f))3)
	f = (decltype(f))GetProcAddress (m, n);
      return f;
    };

    #define quote(x) # x
    #define concat1(x,y) x ## y
    #define concat(x,y) concat1(x,y)

    #define glextfunc(t,n) concat(n, _) = (decltype (concat(n, _)))resolve (quote (n));

    #include "glext_funcs.x.hpp"

    #undef glextfunc
    #undef quote
    #undef concat1
    #undef concat
  }
};


std::unique_ptr<gldev> gldev::make_new (display& d, pixel_format pf, ds_format ds,
					int multisample)
{
  return std::make_unique<gldev_win32> (d, pf, ds, multisample);
}
