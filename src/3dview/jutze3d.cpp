
#if defined (WIN32) && defined (JUTZE3D_EXPORTS)

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "mingw.thread.h"
#include <mutex>
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#include <atomic>

#include <chrono>
#include <limits>
#include <iostream>
#include <algorithm>

#include "gl/display.hpp"
#include "gl/gldev.hpp"
#include "gl/gl.hpp"
#include "gl/pixel_format.hpp"

#include "tiled_image.hpp"
#include "test_scene1.hpp"

#include "jutze3d.hpp"

static std::unique_ptr<display> g_display;
static std::unique_ptr<gldev> g_gldev;
static std::unique_ptr<window> g_window;

static std::thread g_thread;

static void thread_func (void);

enum
{
  WM_USER_3DVIEW_QUIT = WM_USER,
  WM_USER_3DVIEW_CREATE_WINDOW,
  WM_USER_3DVIEW_ENABLE_RENDERING,
  WM_USER_3DVIEW_DISABLE_RENDERING,
  WM_USER_3DVIEW_RESIZE_IMAGE,
  WM_USER_3DVIEW_CENTER_IMAGE,
  WM_
};

struct create_window_args
{
  unsigned int desktop_pos_x;
  unsigned int desktop_pos_y;
  unsigned int width;
  unsigned int height;
  const char* title;
};

struct resize_image_args
{
  unsigned int width;
  unsigned int height;
};

struct center_image_args
{
  unsigned int x;
  unsigned int y;
  double x_rotate;
  double y_rotate;
};

void JUTZE3D_API
view3d_init (void)
{
  g_thread = std::thread (thread_func);
//  MessageBox (nullptr, "It works!", "TEST MOON PLEASE IGNORE", MB_OK);
}

void JUTZE3D_API
view3d_finish (void)
{
  if (g_thread.joinable ())
  {
    PostThreadMessage (GetThreadId (g_thread.native_handle ()), WM_USER_3DVIEW_QUIT, 0, 0);
    g_thread.join ();
  }
}

void* JUTZE3D_API
view3d_new_window (unsigned int desktop_pos_x, unsigned int desktop_pos_y,
		   unsigned int width, unsigned int height, const char* title)
{
  if (!g_thread.joinable ())
    return nullptr;

  if (g_window == nullptr)
  {
    create_window_args args { desktop_pos_x, desktop_pos_y, width, height, title };
    PostThreadMessage (GetThreadId (g_thread.native_handle ()),
		       WM_USER_3DVIEW_CREATE_WINDOW, 0, (LPARAM)&args);
  }

  for (int i = 0; i < 10; ++i)
  {
    std::this_thread::sleep_for (std::chrono::milliseconds (20));
    if (g_window != nullptr)
      break;
  }

  if (g_window != nullptr)
    return g_window->handle ();

  return nullptr;
}


void JUTZE3D_API
view3d_enable_render (void)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;

  SendMessage ((HWND)g_window->handle (), WM_USER_3DVIEW_ENABLE_RENDERING, 0, 0);
}

void JUTZE3D_API
view3d_disable_render (void)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;

  SendMessage ((HWND)g_window->handle (), WM_USER_3DVIEW_DISABLE_RENDERING, 0, 0);
}


void JUTZE3D_API
resize_image (unsigned int width_pixels, unsigned int height_pixels)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;

  resize_image_args args = { width_pixels, height_pixels };

  SendMessage ((HWND)g_window->handle (), WM_USER_3DVIEW_RESIZE_IMAGE, 0, (LPARAM)&args);
}

void JUTZE3D_API
view3d_center_image (unsigned int x, unsigned int y,
		     double x_rotate, double y_rotate)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;

  center_image_args args = { x, y, x_rotate, y_rotate };
  SendMessage ((HWND)g_window->handle (), WM_USER_3DVIEW_CENTER_IMAGE, 0, (LPARAM)&args);
}

void JUTZE3D_API
fill_image (float r, float g, float b, float z)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;

}

void JUTZE3D_API
fill_image_area (unsigned int x, unsigned int y,
		 unsigned int width, unsigned int height,
		 float r, float g, float b, float z)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;
}

void JUTZE3D_API
update_image_area_1 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const char* rgb_bmp_file,
		     const char* height_bmp_file,
		     unsigned int src_x, unsigned int src_y,
		     unsigned int src_width, unsigned int src_height)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;
}


void JUTZE3D_API
update_image_area_2 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const void* rgb_data,  unsigned int rgb_data_stride_bytes,
		     const void* height_data, unsigned int height_data_stride_bytes)
{
  if (!g_thread.joinable () || g_window == nullptr)
    return;
}


void thread_func (void)
{
  unsigned int multisample = 0;
  unsigned int swapinterval = 1;
  auto pf = pixel_format::rgba_8888;
  auto ds = ds_format::ds_24_8;

  g_display = display::make_new (pf, swapinterval, multisample);
  g_gldev = gldev::make_new (*g_display, pf, ds, multisample);

  std::unique_ptr<test_scene1> scene;

  if (!g_gldev->valid ())
  {
    MessageBox (nullptr, "g_gldev NG", "Error", MB_OK);
    g_display = nullptr;
    g_gldev = nullptr;
    return;
  }

  bool shutting_down = false;
  bool en_rendering = false;
  bool en_wireframe = false;
  bool en_debug_dist = false;

  unsigned int window_width = 0;
  unsigned int window_height = 0;

  MSG msg;

  // this forces the creation of the thread message queue.
  PeekMessage (&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

  auto prev_time = std::chrono::high_resolution_clock::now ();

  while (!shutting_down)
  {
    if (!en_rendering)
      GetMessage (&msg, nullptr, 0, 0);
    else
      PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE);

    switch (msg.message)
    {
      case WM_QUIT:
      case WM_USER_3DVIEW_QUIT:
	shutting_down = true;
        break;

      case WM_CLOSE:
	if (shutting_down)
	{
	  TranslateMessage (&msg);
	  DispatchMessage (&msg);
	  break;
	}

	if (g_window != nullptr && g_window->handle () == msg.hwnd)
	{
	  ShowWindow (msg.hwnd, SW_HIDE);
	  break;
	}

      case WM_USER_3DVIEW_CREATE_WINDOW:
      {
	auto&& args = *(create_window_args*)msg.lParam;
	g_window = g_display->create_window (g_gldev->native_visual_id (),
					     args.width, args.height);
	if (g_window == nullptr)
	  std::cerr << "g_window NG" << std::endl;
	else
	{
	  std::cerr << "show window" << std::endl;
	  window_width = args.width;
	  window_height = args.height;
	  g_window->set_title (args.title),
	  g_window->show ();

	  std::cerr << "gldev init surface" << std::endl;
	  g_gldev->init_surface (*g_window);
	  if (!g_gldev->surface_valid ())
	    std::cerr << "g_gldev->surface_valid NG" << std::endl;
	  else
	  {
	    std::cerr << "gldev create context" << std::endl;
	    g_gldev->create_context (swapinterval);
	    if (!g_gldev->context_valid ())
	      std::cerr << "g_gldev->context_valid NG" << std::endl;
	    else
	    {
	      std::pair<GLenum, const char*> glstrs[] =
	      {
		std::make_pair (GL_VENDOR,   "GL_VENDOR: "),
		std::make_pair (GL_RENDERER, "GL_RENDERER:"),
		std::make_pair (GL_VERSION,  "GL_VERSION: "),
		std::make_pair (GL_SHADING_LANGUAGE_VERSION, "GL_SHADING_LANGUAGE_VERSION: "),
		//    std::make_pair (GL_EXTENSIONS, "GL_EXTENSIONS: ")
	      };

	      for (auto&& i : glstrs)
              {
		const char* s = (const char*)glGetString (i.first);
		if (s == nullptr)
		  s = "(null)";
		std::cerr << i.second << s << std::endl;
	      }

	      std::cerr << "gldev init extensions" << std::endl;
	      g_gldev->init_extensions ();
	      scene = std::make_unique<test_scene1> ();
	    }
	  }
	}
	break;
      }

      case WM_USER_3DVIEW_DISABLE_RENDERING:
	en_rendering = false;
	break;

      case WM_USER_3DVIEW_ENABLE_RENDERING:
	en_rendering = true;
	break;

      case WM_USER_3DVIEW_RESIZE_IMAGE:
	if (scene != nullptr)
	{
	  auto&& args = *(resize_image_args*)msg.lParam;
	  scene->resize_image ({ args.width, args.height });
	}
	break;

      case WM_USER_3DVIEW_CENTER_IMAGE:
	if (scene != nullptr)
	{

	}
	break;

      default:
	TranslateMessage (&msg);
	DispatchMessage (&msg);
	break;
    }

    auto cur_time = std::chrono::high_resolution_clock::now ();
    auto delta_time = prev_time - cur_time;

    if (en_rendering && scene != nullptr && g_window != nullptr)
    {
      scene->render (window_width, window_height,
		     std::chrono::duration_cast<std::chrono::microseconds> (delta_time),
		     en_wireframe, en_debug_dist);

      g_gldev->swap_buffers ();
    }
    prev_time = cur_time;
  }

  scene = nullptr;
  g_window = nullptr;
  g_gldev = nullptr;
  g_display = nullptr;
}


#endif // #if defined (WIN32) && defined (JUTZE3D_EXPORTS)
