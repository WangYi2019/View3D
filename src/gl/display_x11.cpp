
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

  vec2<int> m_mouse_down_pos = { 0 };
  int m_mouse_down_button = 0;
  bool m_mouse_dragging = false;
  vec2<int> m_mouse_dragging_last_pos = { 0 };

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

  virtual bool
  process_events (const std::function<void (const input_event&)>& clb) override
  {
    while (XPending (m_disp))
    {
      XEvent e;
      input_event ee;

      XNextEvent (m_disp, &e);

      switch (e.type)
      {
	case KeyPress:
	  std::cout << "key press " << e.xkey.keycode << std::endl;

	  if (XLookupKeysym (&e.xkey, 0) == XK_Escape)
	    return false;

	  ee.type = input_event::key_down;
	  ee.keycode = e.xkey.keycode;
	  clb (ee);
	  break;

	case KeyRelease:
	  std::cout << "key release " << e.xkey.keycode << std::endl;
	  ee.type = input_event::key_up;
	  ee.keycode = e.xkey.keycode;
	  clb (ee);
	  break;

	case ClientMessage:
	  return false;

	case MotionNotify:
	  //std::cout << "motion notify " << e.xmotion.state << std::endl;
	  ee.type = input_event::mouse_move;
	  ee.pos = { e.xmotion.x, e.xmotion.y };
	  clb (ee);

	  if (m_mouse_down_button != 0)
	  {
	    if (length (ee.pos - m_mouse_down_pos) >= 2)
	    {
	      ee = { };
	      ee.type = input_event::mouse_drag;
	      ee.pos = { e.xmotion.x, e.xmotion.y };
	      ee.drag_start_pos = m_mouse_down_pos;
	      ee.drag_delta = ee.pos - m_mouse_dragging_last_pos;
	      ee.drag_abs = ee.pos - m_mouse_down_pos;
	      ee.button = m_mouse_down_button;
	      clb (ee);

	      m_mouse_dragging_last_pos = { e.xmotion.x, e.xmotion.y };
	    }
	  }
	  break;

	case ButtonPress:
	  //std::cout << "button press " << e.xbutton.button << std::endl;

	  if (e.xbutton.button > 3)
	  {
	    // mouse wheel
	  }
	  else
	  {
	    ee.type = input_event::mouse_down;
	    ee.pos = { e.xbutton.x, e.xbutton.y };
	    ee.button = e.xbutton.button;
	    clb (ee);

	    m_mouse_down_pos = ee.pos;
	    m_mouse_down_button = ee.button;
	    m_mouse_dragging_last_pos = ee.pos;
	  }

	  break;

	case ButtonRelease:
	  // std::cout << "button release " << e.xbutton.button << std::endl;

	  if (e.xbutton.button > 3)
	  {
	    // mouse wheel
	    ee.type = input_event::mouse_wheel;
	    ee.pos = { e.xbutton.x, e.xbutton.y };
	    ee.wheel_delta = 0;
	    if (e.xbutton.button == 4)
	      ee.wheel_delta = -1;
	    else if (e.xbutton.button == 5)
	      ee.wheel_delta = 1;
	    clb (ee);
	  }
	  else
	  {
	    ee.type = input_event::mouse_up;
	    ee.pos = { e.xbutton.x, e.xbutton.y };
	    ee.button = e.xbutton.button;
	    clb (ee);

	    if (m_mouse_down_button == ee.button)
	    {
	      if (length (ee.pos - m_mouse_down_pos) < 2)
	      {
		ee = { };
		ee.type = input_event::mouse_click;
		ee.pos = m_mouse_down_pos;
		ee.button = m_mouse_down_button;
		clb (ee);
	      }
	    }

	    m_mouse_down_button = 0;
	    m_mouse_down_pos = { 0 };
	    m_mouse_dragging = false;
	    m_mouse_dragging_last_pos = { 0 };
	  }
	  break;
      }
    }

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

    winattr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask
			 | ButtonReleaseMask | PointerMotionMask;

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
