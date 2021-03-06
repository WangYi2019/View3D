#include <iostream>
#include <cassert>
#include <utility>
#include <memory>
#include <fstream>
#include <iomanip>
#include <chrono>

#include "gl/display.hpp"
#include "gl/gldev.hpp"
#include "gl/gl.hpp"

#include "test_scene.hpp"
#include "test_scene1.hpp"

#include "utils/langcomp.hpp"
#include "utils/math.hpp"

using utils::vec2;
using utils::vec3;
using utils::mat4;

using img::pixel_format;

int main (int argc, const char* argv[])
{
  if (argc < 8)
  {
    std::cout << "usage: "
                 "<executable> <width> <height> <swapinterval> <rgba> "
                 "<ds> <multisample> <file description file>"
                 "\n"
                 "example:  800 480 0 rgba8 d24_s8 0 files.txt"
              << std::endl;

    return 0;
  }

  const int window_width = std::atoi (argv[1]);
  const int window_height = std::atoi (argv[2]);
  const int swap_interval = std::atoi (argv[3]);
  const pixel_format rgba = pixel_format (argv[4]);
  const pixel_format ds = pixel_format (argv[5]);
  const int multisample = std::atoi (argv[6]);
  const char* file_desc_file = argv[7];
  bool en_wireframe = false;
  bool en_debug_dist = false;
  bool en_stairs_mode = false;
  bool en_heightmap = false;

  std::cout << "using window size: " << window_width << " x " << window_height
            << std::endl;
  std::cout << "using swap interval: " << swap_interval << std::endl;
  std::cout << "using rgba: " << rgba << std::endl;
  std::cout << "using ds: " << ds << std::endl;
  std::cout << "using multisample: " << multisample << std::endl;
  std::cout << "using file desc file: " << file_desc_file << std::endl;

  auto disp = display::make_new (rgba, swap_interval, multisample);
  auto dev = gldev::make_new (*disp, rgba, ds, multisample);

  assert (dev->valid ());

  auto win = disp->create_window (dev->native_visual_id (),
				  window_width, window_height);
  assert (win.get () != nullptr);

  win->set_title ("3D View");
  win->show ();

  dev->init_surface (*win);
  assert (dev->surface_valid ());

  dev->create_context (swap_interval);
  assert (dev->context_valid ());

  std::pair<GLenum, const char*> glstrs[] =
  {
    std::make_pair (GL_VENDOR,   "GL_VENDOR: "),
    std::make_pair (GL_RENDERER, "GL_RENDERER:"),
    std::make_pair (GL_VERSION,  "GL_VERSION: "),
    std::make_pair (GL_SHADING_LANGUAGE_VERSION, "GL_SHADING_LANGUAGE_VERSION: "),
//    std::make_pair (GL_EXTENSIONS, "GL_EXTENSIONS: ")
  };

  for (int i = 0; i < (int)countof (glstrs); ++i)
  {
    const char* s = (const char*)glGetString (glstrs[i].first);
    if (s == nullptr)
      s = "(null)";
    std::cerr << glstrs[i].second << s << std::endl;
  }

  dev->init_extensions ();


  test_scene1 scene (file_desc_file);
//  test_scene scene (rgb_img, height_img, lod);
/*
  scene.add_box ({ 0, 0, 0 }, { 32, 32, 32 },
		 { 0.0f, 1.0f, 0.0f, 0.5f },
		 { 0.0f, 1.0f, 0.0f, 1.0f });

  scene.add_box ({ 210, 0, 32 }, { 20, 200, 64-32 },
		 { 0.0f, 1.0f, 0.0f, 0.5f },
		 { 0.0f, 1.0f, 0.0f, 1.0f });
*/
/*
  scene.add_box ({ 8469/2, 10192/2, 128 }, { 1000, 1000, 256 },
		 { 0.0f, 1.0f, 0.0f, 0*0.5f },
		 { 0.0f, 1.0f, 0.0f, 0.0f });

  scene.add_box ({ 8469/2 + 1000, 10192/2, 0 }, { 1000, 1000, 128 },
		 { 0.0f, 1.0f, 0.0f, 0*0.5f },
		 { 0.0f, 1.0f, 0.0f, 1.0f });

  scene.add_box ({ 8469/4, 10192/2, 1 }, { 200, 200, 128 },
		 { 0.0f, 1.0f, 0.0f, 0*0.5f },
		 { 0.0f, 1.0f, 0.0f, 1.0f });

  scene.add_box ({ 1050, 1225, 0 }, { 100, 200, 200 },
		 { 0.0f, 1.0f, 0.0f, 0*0.5f },
		 { 0.0f, 1.0f, 0.0f, 1.0f });
*/

  auto prev_time = std::chrono::high_resolution_clock::now ();

  vec2<double> drag_start_img_pos;
  float drag_start_tilt_angle;
  mat4<double> drag_start_rot_trv;

  bool quit = false;

  win->set_input_event_clb (
    [&] (auto&& e)
    {
      switch (e.type)
      {
	case input_event::key_down:
	  if (e.keycode == input_event::key_esc)
	    quit = true;
	  else if (e.keycode == input_event::key_f1)
	    en_wireframe = !en_wireframe;
	  else if (e.keycode == input_event::key_f2)
	    en_debug_dist = !en_debug_dist;
	  else if (e.keycode == input_event::key_f3)
	    en_stairs_mode = !en_stairs_mode;
	  else if (e.keycode == input_event::key_f5)
	    en_heightmap = !en_heightmap;
	  else if (e.keycode == input_event::key_space)
	    scene.reset_view ();
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
	    drag_start_img_pos = scene.img_pos ();
	  if (e.button == input_event::button_right)
	  {
	    drag_start_tilt_angle = scene.tilt_angle ();
	    drag_start_rot_trv = scene.rotate_trv ();
	    drag_start_img_pos = scene.img_pos ();
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
	    // we need to know how many pixels on the screen are 
	    scene.set_img_pos (drag_start_img_pos
				+ vec2<double> (e.drag_abs) * scene.screen_to_img ());
	  }
	  if (e.button == input_event::button_right)
	  {
	    scene.set_tilt_angle (drag_start_tilt_angle - e.drag_abs.y * 0.1f);

	    // rotate around the currently visible image center.
	    // the image center is captured when the user presses the mouse
	    // button to begin the rotation...
	    scene.set_rotate_trv (
		  mat4<double>::translate (vec3<double> (-drag_start_img_pos, 0))
		* mat4<double>::rotate_z (utils::deg_to_rad (e.drag_abs.x * 0.1f))
		* mat4<double>::translate (vec3<double> (drag_start_img_pos, 0))
		* drag_start_rot_trv);
	  }
	  break;

	case input_event::mouse_wheel:
	  scene.set_zoom (scene.zoom () + e.wheel_delta * 0.0125f);
		scene.screen_to_img ();
	  break;
      }
    });


  while (win->process_events () && !quit)
  {
    vec2<int> win_sz = win->client_size ();

    auto cur_time = std::chrono::high_resolution_clock::now ();
    auto delta_time = prev_time - cur_time;

    scene.render (win_sz.x, win_sz.y,
		  std::chrono::duration_cast<std::chrono::microseconds> (delta_time),
		  en_wireframe, en_stairs_mode, en_debug_dist, en_heightmap);

    dev->swap_buffers ();

    prev_time = cur_time;
  }

abort_main_loop:;
  return 0;
}


// http://www.mingw.org/wiki/sampledll
// http://www.mingw.org/wiki/Interoperability_of_Libraries_Created_by_Different_Compiler_Brands

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms632593(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644946(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644927(v=vs.85).aspx

