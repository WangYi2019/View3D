
#include <type_traits>
#include <vector>
#include <iostream>
#include <string>

#include "test_scene1.hpp"
#include "tiled_image.hpp"

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

  m_image = std::make_unique<tiled_image> (vec2<unsigned int> (8469/1, 10192/1));

//  m_image->update (0, 0, "outputRGB.bmp", "output.bmp");

  std::string base_path = "/home/yam/20151130133409/";

  static const struct
  {
    const char* rgb_img;
    const char* height_img;
    unsigned int x, y;
  } images[] =
  {
    { "1/20151130133409view1.bmp", "1/20151130133409view1-3d3-13.bmp", 6348, 8152 },
    { "2/20151130133409view2.bmp", "2/20151130133409view2-3d3-13.bmp", 4416, 8152 },
    { "3/20151130133409view3.bmp", "3/20151130133409view3-3d3-13.bmp", 2483, 8152 },
    { "4/20151130133409view4.bmp", "4/20151130133409view4-3d3-13.bmp",  549, 8152 },

    { "5/20151130133409view5.bmp", "5/20151130133409view5-3d3-13.bmp", 6348, 6248 },
    { "6/20151130133409view6.bmp", "6/20151130133409view6-3d3-13.bmp", 4416, 6248 },
    { "7/20151130133409view7.bmp", "7/20151130133409view7-3d3-13.bmp", 2483, 6248 },
    { "8/20151130133409view8.bmp", "8/20151130133409view8-3d3-13.bmp",  549, 6248 },


    {  "9/20151130133409view9.bmp",   "9/20151130133409view9-3d3-13.bmp",  6348, 4344 },
    { "10/20151130133409view10.bmp", "10/20151130133409view10-3d3-13.bmp", 4416, 4344 },
    { "11/20151130133409view11.bmp", "11/20151130133409view11-3d3-13.bmp", 2483, 4344 },
    { "12/20151130133409view12.bmp", "12/20151130133409view12-3d3-13.bmp",  549, 4344 },

    { "13/20151130133409view13.bmp", "13/20151130133409view13-3d3-13.bmp", 6348, 2440 },
    { "14/20151130133409view14.bmp", "14/20151130133409view14-3d3-13.bmp", 4416, 2440 },
    { "15/20151130133409view15.bmp", "15/20151130133409view15-3d3-13.bmp", 2483, 2440 },
    { "16/20151130133409view16.bmp", "16/20151130133409view16-3d3-13.bmp",  549, 2440 },

    { "17/20151130133409view17.bmp", "17/20151130133409view17-3d3-13.bmp", 6348, 536 },
    { "18/20151130133409view18.bmp", "18/20151130133409view18-3d3-13.bmp", 4416, 536 },
    { "19/20151130133409view19.bmp", "19/20151130133409view19-3d3-13.bmp", 2483, 536 },
    { "20/20151130133409view20.bmp", "20/20151130133409view20-3d3-13.bmp",  549, 536 },

  };

  for (auto&& i : images)
    m_image->update (i.x, i.y,
		     (base_path + i.rgb_img).c_str (),
		     (base_path + i.height_img).c_str ());

}

test_scene1::~test_scene1 (void)
{
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

vec2<double> test_scene1::screen_to_img (void) const
{
  // figure out the scale factor for screen coordinates to image coordinates.

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

  m_image->render (cam_trv, m_last_proj_trv, viewport_trv, en_wireframe,
		  en_debug_dist);

  gl_check_log_error ();
}
