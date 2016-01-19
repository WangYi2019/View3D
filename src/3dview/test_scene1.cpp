
#include <type_traits>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "test_scene1.hpp"
#include "tiled_image.hpp"

#include "s_expr.hpp"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

static inline float deg_to_rad (float val)
{
  return val * float (M_PI / 180);
}

test_scene1::test_scene1 (void)
{
  m_img_pos = { 0 };
  m_tilt_angle = 0;
  m_rotate_angle = 0;
  m_zoom = 1;
  m_last_proj_trv = mat4<double>::identity ();
  m_last_screen_size = { 1, 1 };
}

test_scene1::test_scene1 (const char* file_desc_file)
: test_scene1 ()
{
  s_expr file_desc;
  std::fstream (file_desc_file, std::ios::in | std::ios::binary) >> file_desc;

  for (auto&& i : file_desc)
  {
    if (i.symbol () || i.empty ())
      continue;

    auto&& a0 = i (0).name ();
    if (a0 == "size")
    {
      unsigned int w = i (1).as<unsigned int> ();
      unsigned int h = i (2).as<unsigned int> ();
      std::cout << "creating new image with size: " << w << " x " << h << std::endl;
      m_image = std::make_unique<tiled_image> (vec2<unsigned int> (w, h));
    }
    else if (a0 == "fov")
    {
      int x = i (1).as<int> ();
      int y = i (2).as<int> ();
      std::string rgb_file = i (3).as<std::string> ();
      std::string height_file = i (4).as<std::string> ();

      std::cout << "adding fov at " << x << "," << y << "\n"
		<< "   rgb image:    " << rgb_file << "\n"
		<< "   height image: " << height_file << "\n" << std::endl;

      if (m_image != nullptr)
	m_image->update (x, y, rgb_file.c_str (), height_file.c_str ());
    }
  }
}

test_scene1::~test_scene1 (void)
{
}

void test_scene1::reset_view (void)
{
  m_img_pos = { 0 };
  m_tilt_angle = 0;
  m_rotate_angle = 0;
  m_zoom = 1;
}

void test_scene1::resize_image (const vec2<unsigned int>& size)
{
  std::cout << "creating new image with size: " << size.x << " x " << size.y << std::endl;
  m_image = std::make_unique<tiled_image> (size);
  reset_view ();
}

void test_scene1::set_tilt_angle (float val)
{
  m_tilt_angle = std::min (80.0f, std::max (0.0f, val));
}

void test_scene1::set_rotate_angle (float val)
{
  m_rotate_angle = std::min (180.0f, std::max (-180.0f, val));
}

void test_scene1::set_zoom (float val)
{
//  m_zoom = std::min (10.0f, std::max (0.0001f, val));
  m_zoom = std::min (10.0f, std::max (-10.0f, val));
}

void test_scene1::center_image (const vec2<unsigned int>& image_point)
{
  double img_sz = std::max (m_image->size ().x, m_image->size ().y);

  auto&& pt = vec2<double> (image_point) * 2 - vec2<double> (m_image->size ());

  m_img_pos = -(vec2<double> (1 / img_sz) * pt);
}

vec2<double> test_scene1::screen_to_img (void) const
{
  // figure out the scale factor for screen coordinates to image coordinates.

  if (m_image == nullptr)
    return { 0 };

  double img_sz = std::max (m_image->size ().x, m_image->size ().y);

  auto cam_trv = calc_cam_trv (m_zoom, m_tilt_angle, m_rotate_angle, { 0, 0 });

  auto trv = m_last_proj_trv * cam_trv;

  auto p_img0 = vec4<double> (0, 0, 0, 1);
  auto p_img1 = vec4<double> (img_sz, 0, 0, 1);

  auto p_img2 = vec4<double> (0, 0, 0, 1);
  auto p_img3 = vec4<double> (0, img_sz, 0, 1);

  auto p_scr0 = homogenize (trv * p_img0).xy ();
  auto p_scr1 = homogenize (trv * p_img1).xy ();
  auto p_scr2 = homogenize (trv * p_img2).xy ();
  auto p_scr3 = homogenize (trv * p_img3).xy ();

  vec2<double> p_scr_len = { length (p_scr1 - p_scr0),
			     length (p_scr3 - p_scr2) };

  return (4.0 / m_last_screen_size) * (1.0 / p_scr_len);
}

mat4<double>
test_scene1::calc_cam_trv (float zoom, float tilt_angle, float rot_angle,
			   const vec2<double>& scroll) const
{
  if (m_image == nullptr)
    return mat4<double>::zero ();

  double img_unit_scale = 2.0 / std::max (m_image->size ().x, m_image->size ().y);

  return
      mat4<double>::identity ()

      // move camera away a bit to make sure it stays in front of znear.
//      * mat4<double>::translate (0, 0, -zoom - 0.001f)
      * mat4<double>::translate (0, 0, -zoom - M_PI/3 *2 + 1)

      // rotate around (0,0) center
      * mat4<double>::rotate_x (deg_to_rad (-tilt_angle))
      * mat4<double>::rotate_z (deg_to_rad (rot_angle))

      // apply scrolling in -1,+1 screen coordinates
      * mat4<double>::translate (vec3<double> (scroll, 0))

      // scale to -1,+1 range (based on max (width, height))
      * mat4<double>::scale (img_unit_scale, img_unit_scale, 1)

      // move image to (0,0) center
      * mat4<double>::translate (vec3<double> (vec2<double> (m_image->size ()) * -0.5, 0))

      * mat4<double>::identity ();
}

void test_scene1::render (unsigned int width, unsigned int height,
			 std::chrono::microseconds delta_time,
			 bool en_wireframe, bool en_debug_dist)
{
  glViewport (0, 0, width, height);
  glClearColor (0.5f, 0.5f, 0.5f, 1);
  glClearDepth (1.0f);
  glClearStencil (0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glEnable (GL_CULL_FACE);
  glFrontFace (GL_CW);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_2D);
  glDepthFunc (GL_LEQUAL);

  glDisable (GL_STENCIL_TEST);
  glDisable (GL_BLEND);

  m_last_screen_size = { width, height };
  m_last_proj_trv =
    mat4<double>::proj_perspective (deg_to_rad (60.0f), (float)width / -(float)height,
                                    0.00001f, 10000.0f);


  auto viewport_trv =
    mat4<double>::scale (width * 0.5, height * 0.5, 1, 1)
    * mat4<double>::translate (1, 1, 0);
/*
  m_rotate_angle += delta_time.count () * 0.0000001f;
  if (m_rotate_angle > 2*M_PI)
    m_rotate_angle -= 2*M_PI;
*/

  auto cam_trv = calc_cam_trv (m_zoom, m_tilt_angle, m_rotate_angle, m_img_pos);

  if (m_image != nullptr)
    m_image->render (cam_trv, m_last_proj_trv, viewport_trv, en_wireframe,
		     en_debug_dist);

  gl_check_log_error ();
}
