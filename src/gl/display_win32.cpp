
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <iostream>
#include <cassert>

#include "utils/utils.hpp"
#include "display.hpp"
#include "pixel_format.hpp"

static input_event::key_code_t remap_key (unsigned int k);

struct display_win32 : display
{
  pixel_format m_pf;
  int m_swapinterval;
  int m_multisample;

  HINSTANCE m_hinst;
  HWND m_root_window;
  WNDCLASS m_window_class;
  ATOM m_window_class_atom;

  display_win32 (pixel_format pf, int swapinterval, int multisample)
  {
    m_pf = pf;
    m_swapinterval = swapinterval;
    m_multisample = m_multisample;

    m_hinst = GetModuleHandle (nullptr);
    m_root_window = GetDesktopWindow ();

    m_window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Size, And Own DC For Window.
    m_window_class.lpfnWndProc = window_proc;
    m_window_class.cbClsExtra = 0;
    m_window_class.cbWndExtra = 0;
    m_window_class.hInstance = m_hinst;
    m_window_class.hIcon = LoadIcon (nullptr, IDI_WINLOGO);
    m_window_class.hCursor = LoadCursor (nullptr, IDC_ARROW);
    m_window_class.hbrBackground = nullptr;
    m_window_class.lpszMenuName = nullptr;
    m_window_class.lpszClassName = "OpenGL";

    m_window_class_atom = RegisterClass (&m_window_class);
    assert (m_window_class_atom != 0);
  }

  virtual ~display_win32 (void) override
  {
    UnregisterClass ((LPCSTR)(intptr_t)m_window_class_atom, m_hinst);
  }

  virtual void* handle (void) const override { return m_root_window; }

  virtual std::unique_ptr<window>
  create_window (int visual_id, int width, int height) override;

  static LRESULT CALLBACK window_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

struct window_win32 : window
{
  HWND m_win;
  vec2<int> m_mouse_down_pos = { 0 };
  int m_mouse_down_button = 0;
  bool m_mouse_dragging = false;
  vec2<int> m_mouse_dragging_last_pos = { 0 };

  std::function<void (const input_event&)> m_input_clb;

  window_win32 (void)
  {
    m_win = 0;
  }

  void handle_mouse_up (const input_event& ee)
  {
    if (m_input_clb)
      m_input_clb (ee);

    if (m_mouse_down_button == ee.button)
    {
      if (length (ee.pos - m_mouse_down_pos) < 2)
      {
	input_event ee = { };
	ee.type = input_event::mouse_click;
	ee.pos = m_mouse_down_pos;
	ee.button = m_mouse_down_button;
	if (m_input_clb)
	  m_input_clb (ee);
      }
    }

    m_mouse_down_button = 0;
    m_mouse_down_pos = { 0 };
    m_mouse_dragging = false;
    m_mouse_dragging_last_pos = { 0 };
    ReleaseCapture ();
  }

  void handle_mouse_down (const input_event& ee)
  {
    if (m_input_clb)
      m_input_clb (ee);

    m_mouse_down_pos = ee.pos;
    m_mouse_down_button = ee.button;
    m_mouse_dragging_last_pos = ee.pos;
    SetCapture (m_win);
  }

  LRESULT window_message (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
  {
    //std::cout << "window " << this << " message" << std::endl;
    input_event ee;

    switch (msg)
    {
      default:
	break;

      case WM_CLOSE:
	std::cout << "window closed" << std::endl;
	ReleaseCapture ();
	PostQuitMessage (0);
	return 0;

      case WM_LBUTTONDOWN:
	ee.type = input_event::mouse_down;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	ee.button = 1;
	handle_mouse_down (ee);
	break;

      case WM_RBUTTONDOWN:
	ee.type = input_event::mouse_down;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	ee.button = 3;
	handle_mouse_down (ee);
	break;

      case WM_LBUTTONUP:
	ee.type = input_event::mouse_up;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	ee.button = 1;
	handle_mouse_up (ee);
	break;

      case WM_RBUTTONUP:
	ee.type = input_event::mouse_up;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	ee.button = 3;
	handle_mouse_up (ee);
	break;

      case WM_MOUSEMOVE:
	ee.type = input_event::mouse_move;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	if (m_input_clb)
	  m_input_clb (ee);

	if (m_mouse_down_button != 0)
	{
	  if (length (ee.pos - m_mouse_down_pos) >= 2)
	  {
	    ee = { };
	    ee.type = input_event::mouse_drag;
	    ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	    ee.drag_start_pos = m_mouse_down_pos;
	    ee.drag_delta = ee.pos - m_mouse_dragging_last_pos;
	    ee.drag_abs = ee.pos - m_mouse_down_pos;
	    ee.button = m_mouse_down_button;
	    if (m_input_clb)
	      m_input_clb (ee);

	    m_mouse_dragging_last_pos = { LOWORD (lparam), HIWORD (lparam) };
	  }
	}
	break;

      case WM_MOUSEWHEEL:
	ee.type = input_event::mouse_wheel;
	ee.pos = { LOWORD (lparam), HIWORD (lparam) };
	ee.wheel_delta = GET_WHEEL_DELTA_WPARAM (wparam) < 0 ? 1 : -1;
	if (m_input_clb)
	  m_input_clb (ee);
	break;

      case WM_KEYDOWN:
	ee.type = input_event::key_down;
	ee.keycode = remap_key (wparam);
	if (m_input_clb)
	  m_input_clb (ee);
	break;

      case WM_KEYUP:
	ee.type = input_event::key_up;
	ee.keycode = remap_key (wparam);
	if (m_input_clb)
	  m_input_clb (ee);
	break;
    }

    return DefWindowProc (hwnd, msg, wparam, lparam);
  }

  virtual ~window_win32 (void) override
  {
    close ();
  }

  virtual void set_title (const char* val) override
  {
    SetWindowText (m_win, val);
  }

  virtual void close (void) override
  {
    SetWindowLongPtr (m_win, GWLP_USERDATA, 0);
    DestroyWindow (m_win);
  }

  virtual void* handle (void) const override
  {
    return (void*)m_win;
  }

  virtual void show (void) override
  {
    ShowWindow (m_win, SW_SHOW);
    SetForegroundWindow (m_win);
    SetFocus (m_win);
  }

  virtual void
  set_input_event_clb (const std::function<void (const input_event&)>& f) override
  {
    m_input_clb = f;
  }

  virtual bool
  process_events (void) override
  {
    MSG msg;
    if (PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
	return false;
      else
      {
	TranslateMessage (&msg);
	DispatchMessage (&msg);
      }
    }

    return true;
  }
};


std::unique_ptr<window>
display_win32::create_window (int visual_id, int width, int height)
{
  RECT window_rect;
  window_rect.left = 0;
  window_rect.right = width;
  window_rect.top = 0;
  window_rect.bottom = height;

  DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
  DWORD dwStyle = WS_OVERLAPPEDWINDOW;

  AdjustWindowRectEx (&window_rect, dwStyle, false, dwExStyle);

  auto w = std::make_unique<window_win32> ();

  std::cout << "creating new window " << (void*)w.get () << std::endl;

  HWND hwnd = CreateWindowEx (dwExStyle, "OpenGL", "",
				dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0,	// Window Position
				window_rect.right - window_rect.left,
				window_rect.bottom - window_rect.top,
				nullptr,	// No Parent Window
				nullptr,	// No Menu
				m_hinst,
				w.get ());	// WM_CREATE LPARAM value

  std::cout << "new window hwnd: " << hwnd << std::endl;
  assert (hwnd);
  w->m_win = hwnd;

  SetWindowLongPtr (hwnd, GWLP_USERDATA, (LPARAM)w.get ());

  return std::move (w);
}

LRESULT display_win32::window_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
/*
// somehow this doesn't work on wine.
// LPARAM is not the same in WM_CREATE as passed to CreateWindowEx.
// or maybe there is some other misunderstanding....

  if (msg == WM_CREATE && (void*)lparam != nullptr)
  {
    std::cout << "window_proc new window " << (void*)lparam << std::endl;

    SetWindowLongPtr (hwnd, GWLP_USERDATA, lparam);
    ((window_win32*)lparam)->m_win = hwnd;

    return DefWindowProc (hwnd, msg, wparam, lparam);
  }
*/
  auto w = (window_win32*)GetWindowLongPtr (hwnd, GWLP_USERDATA);

  if (w != nullptr)
    return w->window_message (hwnd, msg, wparam, lparam);
  else
    return DefWindowProc (hwnd, msg, wparam, lparam);
}

std::unique_ptr<display> display::make_new (pixel_format pf, int swapinterval,
					    int multisample)
{
  return std::make_unique<display_win32> (pf, swapinterval, multisample);
}

// -----------------------------------------------------------------------


static input_event::key_code_t remap_key (unsigned int k)
{
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
  switch (k)
  {
    default: return input_event::no_key;
    case VK_ESCAPE: return input_event::key_esc;
    case VK_F1: return input_event::key_f1;
    case VK_F2: return input_event::key_f2;
    case VK_F3: return input_event::key_f3;
    case VK_F4: return input_event::key_f4;
    case VK_F5: return input_event::key_f5;
    case VK_F6: return input_event::key_f6;
    case VK_F7: return input_event::key_f7;
    case VK_F8: return input_event::key_f8;
    case VK_F9: return input_event::key_f9;
    case VK_F10: return input_event::key_f10;
    case VK_F11: return input_event::key_f11;
    case VK_F12: return input_event::key_f12;
    case VK_SPACE: return input_event::key_space;
  }
}

