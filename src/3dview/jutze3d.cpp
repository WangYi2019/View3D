
#if defined (WIN32) && defined (JUTZE3D_EXPORTS)

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#if defined (WIN32) && !defined (_MSC_VER)

#include "mingw.thread.h"
#include <mutex>
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"

#else

#include <thread>
#include <mutex>
#include <condition_variable>

#endif

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
static std::atomic<int> g_thread_running = ATOMIC_VAR_INIT (0);

static std::unique_ptr<test_scene1> g_scene;

static void thread_func (void);
static void window_input_event_clb (const input_event& e);

static bool en_wireframe = false;
static bool en_debug_dist = false;
static bool quit = false;

static vec2<double> drag_start_img_pos;
static float drag_start_tilt_angle;
static mat4<double> drag_start_rot_trv;


enum
{
  WM_USER_3DVIEW_QUIT = WM_USER,
  WM_USER_3DVIEW_CREATE_WINDOW,
  WM_USER_3DVIEW_ENABLE_RENDERING,
  WM_USER_3DVIEW_DISABLE_RENDERING,
  WM_USER_3DVIEW_RESIZE_IMAGE,
  WM_USER_3DVIEW_CENTER_IMAGE,
  WM_USER_3DVIEW_FILL_IMAGE,
  WM_USER_3DVIEW_FILL_IMAGE_AREA,
  WM_USER_3DVIEW_UPDATE_IMAGE_AREA_1,
  WM_USER_3DVIEW_UPDATE_IMAGE_AREA_2,
  WM_USER_3DVIEW_ADD_BOX,
  WM_USER_3DVIEW_REMOVE_BOX,
  WM_USER_3DVIEW_REMOVE_ALL_BOXES,
  MW_USER_3DVIEW_SET_Z_SCALE,
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

struct fill_image_args
{
  float r;
  float g;
  float b;
  float z;
};

struct fill_image_area_args
{
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
  float r;
  float g;
  float b;
  float z;
};

struct update_image_area1_args
{
  unsigned int x, y;
  unsigned int width, height;
  const char* rgb_bmp_file;
  const char* height_bmp_file;
  unsigned int src_x, src_y;
  unsigned int src_width, src_height;
};

struct update_image_area2_args
{
  unsigned int x, y;
  unsigned int width, height;
  const void* rgb_data;
  unsigned int rgb_data_stride_bytes;
  pixel_format rgb_format;
  const void* height_data;
  unsigned int height_data_stride_bytes;
  pixel_format height_format;
};

struct add_box_args
{
  unsigned int x, y, z;
  unsigned int size_x, size_y, size_z;
  float fill_r, fill_g, fill_b, fill_a;
  float edge_r, edge_g, edge_b, edge_a;

  unsigned int new_box_obj_id;
};

struct remove_box_args
{
  unsigned int obj_id;
};

struct set_z_scale_args
{
  float value;
};

void post_thread_message_wait (unsigned int msg, void* args = nullptr)
{
  if (!g_thread.joinable ())
    return;

  std::atomic<int> done = ATOMIC_VAR_INIT (0);
  PostThreadMessage (GetThreadId (g_thread.native_handle ()), msg,
		     (WPARAM)&done, (LPARAM)args);
  while (done == false)
    std::this_thread::sleep_for (std::chrono::milliseconds (10));
}

void ack_thread_message (MSG& msg)
{
  static_assert (sizeof (void*) <= sizeof (WPARAM), "");
  std::atomic<int>* done = (std::atomic<int>*)msg.wParam;
  if (done != nullptr)
    *done = true;
}

JUTZE3D_API void
view3d_init (void)
{
  g_thread = std::thread (thread_func);

  while (g_thread_running == false)
    std::this_thread::sleep_for (std::chrono::milliseconds (10));
}

JUTZE3D_API void 
view3d_finish (void)
{
  if (g_thread.joinable ())
  {
    PostThreadMessage (GetThreadId (g_thread.native_handle ()), WM_USER_3DVIEW_QUIT, 0, 0);
    g_thread.join ();
  }
}

JUTZE3D_API void* 
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

  for (int i = 0; i < 100; ++i)
  {
    std::this_thread::sleep_for (std::chrono::milliseconds (20));
    if (g_window != nullptr)
      break;
  }

  if (g_window != nullptr)
    return g_window->handle ();

  return nullptr;
}


JUTZE3D_API void 
view3d_enable_render (void)
{
  post_thread_message_wait (WM_USER_3DVIEW_ENABLE_RENDERING);
}

JUTZE3D_API void 
view3d_disable_render (void)
{
  post_thread_message_wait (WM_USER_3DVIEW_DISABLE_RENDERING);
}

JUTZE3D_API void 
view3d_set_z_scale (float val)
{
  set_z_scale_args args = { val };
  post_thread_message_wait (MW_USER_3DVIEW_SET_Z_SCALE, &args);
}


JUTZE3D_API void 
view3d_resize_image (unsigned int width_pixels, unsigned int height_pixels)
{
  resize_image_args args = { width_pixels, height_pixels };
  post_thread_message_wait (WM_USER_3DVIEW_RESIZE_IMAGE, &args);
}

JUTZE3D_API void 
view3d_center_image (unsigned int x, unsigned int y,
		     double x_rotate, double y_rotate)
{
  center_image_args args = { x, y, x_rotate, y_rotate };
  post_thread_message_wait (WM_USER_3DVIEW_CENTER_IMAGE, &args);
}

JUTZE3D_API void 
view3d_fill_image (float r, float g, float b, float z)
{
  fill_image_args args = { r, g, b, z };
  post_thread_message_wait (WM_USER_3DVIEW_FILL_IMAGE, &args);
}

JUTZE3D_API void 
view3d_fill_image_area (unsigned int x, unsigned int y,
		 unsigned int width, unsigned int height,
		 float r, float g, float b, float z)
{
  fill_image_area_args args = { x, y, width, height, r, g, b, z };
  post_thread_message_wait (WM_USER_3DVIEW_FILL_IMAGE_AREA, &args);
}

JUTZE3D_API void 
view3d_update_image_area_1 (unsigned int x, unsigned int y,
			    unsigned int width, unsigned int height,
			    const char* rgb_bmp_file,
			    const char* height_bmp_file,
			    unsigned int src_x, unsigned int src_y,
			    unsigned int src_width, unsigned int src_height)
{
  update_image_area1_args args = { x, y, width, height, rgb_bmp_file,
				   height_bmp_file, src_x, src_y,
				   src_width, src_height };
  post_thread_message_wait (WM_USER_3DVIEW_UPDATE_IMAGE_AREA_1, &args);
}


JUTZE3D_API void
view3d_update_image_area_2 (unsigned int x, unsigned int y,
			    unsigned int width, unsigned int height,
			    const void* rgb_data,  unsigned int rgb_data_stride_bytes,
			    unsigned int rgb_format,
			    const void* height_data, unsigned int height_data_stride_bytes)
{
  pixel_format rgb_format_pf;
  switch (rgb_format)
  {
    case 0: rgb_format_pf = pixel_format::rgb_888; break;
    case 1: rgb_format_pf = pixel_format::bgr_888; break;
    case 2: rgb_format_pf = pixel_format::rgba_8888; break;
    default: return;
  }

  update_image_area2_args args = { x, y, width, height, rgb_data, rgb_data_stride_bytes,
				   rgb_format_pf,
				   height_data, height_data_stride_bytes,
				   pixel_format::l_8 };
  post_thread_message_wait (WM_USER_3DVIEW_UPDATE_IMAGE_AREA_2, &args);
}

JUTZE3D_API unsigned int
view3d_add_box (unsigned int board_pos_x, unsigned int board_pos_y, unsigned int board_pos_z,
		unsigned int box_size_x, unsigned int box_size_y, unsigned int box_size_z,
		float fill_r, float fill_g, float fill_b, float fill_a,
		float edge_r, float edge_g, float edge_b, float edge_a)
{
  add_box_args args = { board_pos_x, board_pos_y, board_pos_z,
			box_size_x, box_size_y, box_size_z,
			fill_r, fill_g, fill_b, fill_a,
			edge_r, edge_g, edge_b, edge_a,
			0 };

  post_thread_message_wait (WM_USER_3DVIEW_ADD_BOX, &args);

  return args.new_box_obj_id;
}

JUTZE3D_API void
view3d_remove_box (unsigned int obj_id)
{
  remove_box_args args = { obj_id };
  post_thread_message_wait (WM_USER_3DVIEW_REMOVE_BOX, &args);
}

JUTZE3D_API void
view3d_remove_all_boxes (void)
{
  post_thread_message_wait (WM_USER_3DVIEW_REMOVE_ALL_BOXES);
}


void thread_func (void)
{
  unsigned int multisample = 0;
  unsigned int swapinterval = 1;
  auto pf = pixel_format::rgba_8888;
  auto ds = ds_format::ds_24_8;

  g_display = display::make_new (pf, swapinterval, multisample);
  g_gldev = gldev::make_new (*g_display, pf, ds, multisample);


  if (!g_gldev->valid ())
  {
    MessageBox (nullptr, "g_gldev NG", "Error", MB_OK);
    g_display = nullptr;
    g_gldev = nullptr;
    return;
  }

  bool shutting_down = false;
  bool en_rendering = false;

  unsigned int window_width = 0;
  unsigned int window_height = 0;

  MSG msg;

  // this forces the creation of the thread message queue.
  PeekMessage (&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

  auto prev_time = std::chrono::high_resolution_clock::now ();

  while (!shutting_down)
  {
    g_thread_running = true;

    if (!en_rendering)
      GetMessage (&msg, nullptr, 0, 0);
    else
    {
      if (!PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE))
        goto Lcontinue;
    }

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
        break;

      case WM_USER_3DVIEW_CREATE_WINDOW:
      {
	auto&& args = *(create_window_args*)msg.lParam;
	auto new_win = g_display->create_window (g_gldev->native_visual_id (),
					         args.width, args.height);
	if (new_win == nullptr)
	  std::cerr << "new window NG" << std::endl;
	else
	{
	  std::cerr << "show window hwnd " << new_win->handle () << std::endl;
	  window_width = args.width;
	  window_height = args.height;
	  new_win->set_input_event_clb (window_input_event_clb);
	  new_win->set_title (args.title),
	  new_win->show ();

	  std::cerr << "gldev init surface" << std::endl;
	  g_gldev->init_surface (*new_win);
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
	      g_scene = std::make_unique<test_scene1> ();

	      g_window = std::move (new_win);
	    }
	  }
	}
	break;
      }

      case WM_USER_3DVIEW_DISABLE_RENDERING:
	std::cout << "WM_USER_3DVIEW_DISABLE_RENDERING" << std::endl;
	en_rendering = false;
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_ENABLE_RENDERING:
	std::cout << "WM_USER_3DVIEW_ENABLE_RENDERING" << std::endl;
	en_rendering = true;
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_RESIZE_IMAGE:
	std::cout << "WM_USER_3DVIEW_RESIZE_IMAGE" << std::endl;
	if (g_scene != nullptr)
	{
	  auto&& args = *(resize_image_args*)msg.lParam;
	  g_scene->resize_image ({ args.width, args.height });
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_CENTER_IMAGE:
	if (g_scene != nullptr)
	{
	  auto&& args = *(center_image_args*)msg.lParam;
	  g_scene->center_image ({ args.x, args.y });
	  g_scene->set_tilt_angle ((float)args.x_rotate);
	  g_scene->set_rotate_trv (mat4<double>::rotate_z (args.y_rotate));
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_FILL_IMAGE:
	if (g_scene != nullptr && g_scene->image () != nullptr)
	{
	  auto&& args = *(fill_image_args*)msg.lParam;
	  g_scene->image ()->fill (args.r, args.g, args.b, args.z);
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_FILL_IMAGE_AREA:
	if (g_scene != nullptr && g_scene->image () != nullptr)
	{
	  auto&& args = *(fill_image_area_args*)msg.lParam;

	  g_scene->image ()->fill (args.x, args.y, args.width, args.height,
				   args.r, args.g, args.b, args.z);
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_UPDATE_IMAGE_AREA_1:
	if (g_scene != nullptr && g_scene->image () != nullptr)
	{
	  auto&& args = *(update_image_area1_args*)msg.lParam;
	  g_scene->image ()->update (args.x, args.y, args.rgb_bmp_file,
				     args.height_bmp_file,
				     args.src_x, args.src_y,
				     args.src_width, args.src_height);
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_UPDATE_IMAGE_AREA_2:
	if (g_scene != nullptr && g_scene->image () != nullptr)
	{
	  auto&& args = *(update_image_area2_args*)msg.lParam;
	  g_scene->image ()->update (args.x, args.y, args.width, args.height,
				     args.rgb_data, args.rgb_data_stride_bytes,
				     args.rgb_format,
				     args.height_data, args.height_data_stride_bytes,
				     args.height_format);
	}
	ack_thread_message (msg);
	break;


      case WM_USER_3DVIEW_ADD_BOX:
	if (g_scene != nullptr)
	{
	  auto&& args = *(add_box_args*)msg.lParam;
	  args.new_box_obj_id = g_scene->add_box (
		{ args.x, args.y, args.z },
		{ args.size_x, args.size_y, args.size_z },
		{ args.fill_r, args.fill_g, args.fill_b, args.fill_a },
		{ args.edge_r, args.edge_g, args.edge_b, args.edge_a });
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_REMOVE_BOX:
	if (g_scene != nullptr)
	{
	  auto&& args = *(remove_box_args*)msg.lParam;
	  g_scene->remove_box (args.obj_id);
	}
	ack_thread_message (msg);
	break;

      case WM_USER_3DVIEW_REMOVE_ALL_BOXES:
	if (g_scene != nullptr)
	  g_scene->remove_all_boxes ();
	ack_thread_message (msg);
	break;

      case MW_USER_3DVIEW_SET_Z_SCALE:
	if (g_scene != nullptr)
	{
	  auto&& args = *(set_z_scale_args*)msg.lParam;
	  g_scene->set_z_scale (args.value);
	}
	ack_thread_message (msg);
	break;

      default:
	TranslateMessage (&msg);
	DispatchMessage (&msg);
	break;
    }

Lcontinue:
    auto cur_time = std::chrono::high_resolution_clock::now ();
    auto delta_time = prev_time - cur_time;

    if (en_rendering && g_scene != nullptr && g_window != nullptr)
    {
      vec2<int> win_sz = g_window->client_size ();

      g_scene->render (win_sz.x, win_sz.y,
		       std::chrono::duration_cast<std::chrono::microseconds> (delta_time),
		       en_wireframe, en_debug_dist);

      g_gldev->swap_buffers ();
    }
    prev_time = cur_time;
  }

  g_scene = nullptr;
  g_window = nullptr;
  g_gldev = nullptr;
  g_display = nullptr;
}

static void window_input_event_clb (const input_event& e)
{
  if (g_scene == nullptr)
    return;

      switch (e.type)
      {
	case input_event::key_down:
		if (e.keycode == input_event::key_esc)
		{
			//quit = true; 
			ShowWindow(HWND(g_window->handle()),SW_HIDE);
			g_scene->resize_image({ 10, 10 });
		}
	  else if (e.keycode == input_event::key_f1)
		g_scene->AutoRotate();
	  else if (e.keycode == input_event::key_f2)
	    en_debug_dist = !en_debug_dist;
	  else if (e.keycode == input_event::key_f3)
		en_wireframe = !en_wireframe;
	  else if (e.keycode == input_event::key_space)
	    g_scene->reset_view ();
	  break;
/*
	case input_event::mouse_move:
	  std::cout << "mouse move " << e.pos.x << ", " << e.pos.y << std::endl;
	  break;

	case input_event::mouse_down:
	  std::cout << "mouse down " << e.pos.x << ", " << e.pos.y
		    << "  " << e.button << std::endl;
	  break;

	case input_event::mouse_up:
	  std::cout << "mouse up " << e.pos.x << ", " << e.pos.y
		    << "  " << e.button << std::endl;
	  break;
*/
	case input_event::mouse_down:
	  if (e.button == input_event::button_left)
	    drag_start_img_pos = g_scene->img_pos ();
	  if (e.button == input_event::button_right)
	  {
	    drag_start_tilt_angle = g_scene->tilt_angle ();
	    drag_start_rot_trv = g_scene->rotate_trv ();
	    drag_start_img_pos = g_scene->img_pos ();
	  }
	  break;

	case input_event::mouse_click:
	  std::cout << "mouse click " << e.pos.x << ", " << e.pos.y
		    << "  " << e.button << std::endl;
	  break;

	case input_event::mouse_drag:
	  if (e.button == input_event::button_left)
	  {
	    // move the image by e.drag_abs pixels on the screen.
	    // we need to know how many pixels on the screen are.
	    g_scene->set_img_pos (drag_start_img_pos
				+ vec2<double> (e.drag_abs) * g_scene->screen_to_img ());
	  }
	  if (e.button == input_event::button_right)
	  {
	    g_scene->set_tilt_angle (drag_start_tilt_angle - e.drag_abs.y * 0.1f);

	    // rotate around the currently visible image center.
	    // the image center is captured when the user presses the mouse
	    // button to begin the rotation...
	    g_scene->set_rotate_trv (
		  mat4<double>::translate (vec3<double> (-drag_start_img_pos, 0))
		* mat4<double>::rotate_z (deg_to_rad (e.drag_abs.x * 0.1f))
		* mat4<double>::translate (vec3<double> (drag_start_img_pos, 0))
		* drag_start_rot_trv);
	  }
	  break;

	case input_event::mouse_wheel:
	  g_scene->set_zoom (g_scene->zoom () + e.wheel_delta * 0.025f);
	  break;
      }
}

#endif // #if defined (WIN32) && defined (JUTZE3D_EXPORTS)
