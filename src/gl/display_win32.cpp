
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <iostream>
#include <cassert>

#include "utils/utils.hpp"
#include "display.hpp"
#include "pixel_format.hpp"

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

  window_win32 (void)
  {
    m_win = 0;
  }

  LRESULT window_message (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
  {
    //std::cout << "window " << this << " message" << std::endl;

    if (msg == WM_CLOSE)
    {
      std::cout << "window closed" << std::endl;
      PostQuitMessage (0);
      return 0;
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

  virtual bool
  process_events (const std::function<void (const input_event&)>& clb) override
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

  std::cout << "creating new window " << w.get () << std::endl;

  HWND hwnd = CreateWindowEx (dwExStyle, "OpenGL", "",
				dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0,	// Window Position
				window_rect.right - window_rect.left,
				window_rect.bottom - window_rect.top,
				nullptr,	// No Parent Window
				nullptr,	// No Menu
				m_hinst,
				w.get ());	// WM_CREATE LPARAM value

  assert (hwnd);
  w->m_win = hwnd;
  return std::move (w);
}

LRESULT display_win32::window_proc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (msg == WM_CREATE && (void*)lparam != nullptr)
  {
    std::cout << "window_proc new window " << lparam << std::endl;

    SetWindowLongPtr (hwnd, GWLP_USERDATA, lparam);
    ((window_win32*)lparam)->m_win = hwnd;

    return DefWindowProc (hwnd, msg, wparam, lparam);
  }

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
