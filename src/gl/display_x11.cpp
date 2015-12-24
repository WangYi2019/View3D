
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include <iostream>
#include <cassert>

#include "utils/utils.hpp"
#include "display.hpp"
#include "pixel_format.hpp"

struct window_x11 : window
{
  Window m_win;
  Display* m_disp;

  window_x11 (Window w, Display* d) : m_win (w), m_disp (d) { }

  virtual ~window_x11 (void) override { close (); }

  virtual void set_title (const char* val) override
  {
    XStoreName (m_disp, m_win, val);
  }

  virtual void close (void) override
  {
    if (m_disp != NULL && m_win != 0)
      XDestroyWindow (m_disp, m_win);
    m_disp = NULL;
    m_win = 0;
  }

  virtual void* handle (void) const override
  {
    return (void*)m_win;
  }

  virtual void show (void) override
  {
    XMapWindow (m_disp, m_win);
  }

  virtual bool process_events (void) override
  {
    XEvent e;
    if (!XPending (m_disp))
      return true;

    XNextEvent (m_disp, &e);
    if (e.type == KeyPress)
    {
      if (XLookupKeysym (&e.xkey, 0) == XK_Escape)
        return false;
    }
    else if (e.type == ClientMessage)
      return false;

    return true;
  }
};

struct display_x11 : display
{
  Display* m_disp;
  Window m_root_window;

  display_x11 (pixel_format/* pf*/, int/* swapinterval*/, int/*multisample*/)
  {
    m_disp = XOpenDisplay (NULL);
    assert (m_disp);

    m_root_window = RootWindow (m_disp, DefaultScreen (m_disp));
    assert (m_root_window);
  }

  virtual ~display_x11 (void) override
  {
    if (m_disp)
      XCloseDisplay (m_disp);
  }

  virtual void* handle (void) const override { return m_disp; }

  virtual std::unique_ptr<window>
  create_window (int visual_id, int width, int height) override
  {
    XVisualInfo vi;
    vi.visualid = visual_id;

    int vipcount;
    XVisualInfo* vip = XGetVisualInfo (m_disp, VisualIDMask, &vi, &vipcount);

    if (vip == nullptr)
      std::cerr << "XGetVisualInfo (" << visual_id << ") NG" << std::endl;

    XSetWindowAttributes winattr;
    winattr.background_pixel = 0;
    winattr.border_pixel = 0;
    if (vip != nullptr)
      winattr.colormap = XCreateColormap (m_disp, m_root_window, vip->visual,
                                          AllocNone);
   else
      winattr.colormap = DefaultColormap (m_disp, 0);

    winattr.event_mask = KeyPressMask;
    const unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap
                               | CWEventMask;

    Window win = XCreateWindow (m_disp, m_root_window, 0, 0,
                                width, height, 0, 
                                vip != nullptr ? vip->depth : CopyFromParent,
                                InputOutput,
                                vip != nullptr ? vip->visual : CopyFromParent,
                                mask, &winattr);
    if (vip != nullptr)
      XFree (vip);

    if (!win)
    {
      std::cerr << "XCreateWindow NG" << std::endl;
      return std::unique_ptr<window> ();
    }

    Atom wmdel = XInternAtom (m_disp, "WM_DELETE_WINDOW", True);
    XSetWMProtocols (m_disp, win, &wmdel, 1);

    return std::make_unique<window_x11> (win, m_disp);
  }
};

std::unique_ptr<display> display::make_new (pixel_format pf, int swapinterval,
					    int multisample)
{
  return std::make_unique<display_x11> (pf, swapinterval, multisample);
}
