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
#include "gl/pixel_format.hpp"

#include "test_scene.hpp"
#include "test_scene1.hpp"

#include "bmp_loader.hpp"

int main (int argc, const char* argv[])
{
  if (argc < 11)
  {
    std::cout << "usage: "
                 "<executable> <width> <height> <swapinterval> <rgba> "
                 "<ds> <multisample> <rgb image> <height img> <lod level> <enable wireframe>"
                 "\n"
                 "example:  800 480 0 RGBA_8888 DS_24_8 0 outputRGB.bmp output.bmp 1 0"
              << std::endl;

    return 0;
  }

  const int window_width = std::atoi (argv[1]);
  const int window_height = std::atoi (argv[2]);
  const int swap_interval = std::atoi (argv[3]);
  const pixel_format rgba = pixel_format (argv[4]);
  const ds_format ds = ds_format (argv[5]);
  const int multisample = std::atoi (argv[6]);
  const char* rgb_img = argv[7];
  const char* height_img = argv[8];
  const unsigned int lod = std::stoul (argv[9]);
  const bool en_wireframe = std::stoi (argv[10]);

  std::cout << "using window size: " << window_width << " x " << window_height
            << std::endl;
  std::cout << "using swap interval: " << swap_interval << std::endl;
  std::cout << "using rgba: " << rgba << std::endl;
  std::cout << "using ds: " << ds << std::endl;
  std::cout << "using multisample: " << multisample << std::endl;
  std::cout << "using rgb image: " << rgb_img << std::endl;
  std::cout << "using height image: " << height_img << std::endl;
  std::cout << "using lod: " << lod << std::endl;
  std::cout << "using enable wireframe: " << en_wireframe;

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


  test_scene1 scene;

  auto prev_time = std::chrono::high_resolution_clock::now ();

  while (win->process_events ())
  {
    auto cur_time = std::chrono::high_resolution_clock::now ();
    auto delta_time = prev_time - cur_time;

    scene.render (window_width, window_height,
		  std::chrono::duration_cast<std::chrono::microseconds> (delta_time),
		  en_wireframe);

    dev->swap_buffers ();

    prev_time = cur_time;
  }

abort_main_loop:;
  return 0;
}
