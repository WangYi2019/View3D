
#include <type_traits>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "test_scene1.hpp"
#include "tiled_image.hpp"
#include "simple_3dbox.hpp"

#include "s_expr/s_expr.hpp"
#include "utils/math.hpp"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

using utils::vec2;
using utils::vec3;
using utils::vec4;
using utils::mat4;

test_scene1::test_scene1 (void)
{
  m_img_pos = { 0 };
  m_tilt_angle = 0;
  m_rotate_trv = mat4<double>::identity ();
  m_zoom = 1;
  m_last_proj_trv = mat4<double>::identity ();
  m_last_screen_size = { 1, 1 };
  m_bAutoRotate = false;
}

test_scene1::test_scene1 (const char* file_desc_file)
: test_scene1 ()
{
  m_bAutoRotate = false; 
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
      m_image = std::make_unique<tiled_image> (vec2<unsigned int> (w, h), m_use_uint16_heightmap);

      m_image->set_heightmap_palette (
      {
	{ 60,  vec4<float> (  0,  54, 224, 255) * 1.0f/255.0f },
	{ 110, vec4<float> (  0, 171, 200, 255) * 1.0f/255.0f },
	{ 170, vec4<float> (  0, 223, 169, 255) * 1.0f/255.0f },
	{ 230, vec4<float> (  0, 255, 107, 255) * 1.0f/255.0f },
	{ 280, vec4<float> (  0, 255,  70, 255) * 1.0f/255.0f },
	{ 340, vec4<float> (109, 254,  72, 255) * 1.0f/255.0f },
	{ 400, vec4<float> (192, 235,  76, 255) * 1.0f/255.0f },
	{ 450, vec4<float> (248, 190,  70, 255) * 1.0f/255.0f },
	{ 510, vec4<float> (255, 127,  61, 255) * 1.0f/255.0f },
	{ 570, vec4<float> (255,  66,  57, 255) * 1.0f/255.0f },

/*
	{   0,  vec4<float> (  64*0, 64*0, 64*0, 255) * 1.0f/255.0f },
	{ 200,  vec4<float> (  64*1, 64*1, 64*1, 255) * 1.0f/255.0f },
	{ 400,  vec4<float> (  64*2, 64*2, 64*2, 255) * 1.0f/255.0f },
	{ 600,  vec4<float> (  64*3, 64*3, 64*3, 255) * 1.0f/255.0f },
	{ 800,  vec4<float> (  64*4, 64*4, 64*4, 255) * 1.0f/255.0f },
*/
      });
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
    else if (a0 == "fill")
    {
      int x = i (1).as<int> ();
      int y = i (2).as<int> ();
      int w = i (3).as<int> ();
      int h = i (4).as<int> ();

      float r = i (5).as<float> ();
      float g = i (6).as<float> ();
      float b = i (7).as<float> ();
      float z = i (8).as<float> ();

      if (m_image != nullptr)
	m_image->fill (x, y, w, h, r, g, b, z);
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
  m_rotate_trv = mat4<double>::identity ();
  m_zoom = 1;
  m_bAutoRotate = false; 
}

void test_scene1::resize_image (const vec2<unsigned int>& size)
{
  std::cout << "creating new image with size: " << size.x << " x " << size.y << std::endl;
  m_image = std::make_unique<tiled_image> (size, m_use_uint16_heightmap);
  reset_view ();
}

void test_scene1::set_tilt_angle (float val)
{
  m_tilt_angle = std::min (80.0f, std::max (0.0f, val));
}

void test_scene1::set_zoom (float val)
{
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

  auto cam_trv = calc_cam_trv (m_zoom, m_tilt_angle, { 0, 0 });

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
test_scene1::calc_cam_trv (float zoom, float tilt_angle,
			   const vec2<double>& scroll) const
{
  if (m_image == nullptr)
    return mat4<double>::zero ();

  // scale factor to convert image coordinates to -1..+1 coordinates.
  // this is needed because the image coordinates can't be displayed 1:1.
  // the exact scale factor is not so important actually, but it has to
  // be in a range which will allow displaying the whole image on the screen
  // when zoomed out.
  // if the scale factor is too big, the image will appear
  // too big and will require higher m_zoom value to fit the image on the screen.
  // if m_zoom is too big, the image will intersect the z-far plane and there
  // will be z fighting artefacts.
  double unit_scale = 2.0 / std::max (m_image->size ().x, m_image->size ().y);

  return
      mat4<double>::identity ()

      // move camera away a bit to make sure it stays in front of znear.
//      * mat4<double>::translate (0, 0, -zoom - 0.001f)
      * mat4<double>::translate (0, 0, -zoom - M_PI/3 *2 + 1)

      * mat4<double>::rotate_x (utils::deg_to_rad (-tilt_angle))

      // apply scrolling in -1,+1 screen coordinates
      * mat4<double>::translate (vec3<double> (scroll, 0))

      // rotate (normally around the z axis around some point)
      * m_rotate_trv

      // scale image into to -1,+1 range
      * mat4<double>::scale (unit_scale, unit_scale, unit_scale * m_z_scale)

      // move image to (0,0) center
      * mat4<double>::translate (vec3<double> (vec2<double> (m_image->size ()) * -0.5, 0))

      * mat4<double>::identity ();
}

unsigned int
test_scene1::add_box (const vec3<unsigned int>& board_pos,
		      const vec3<unsigned int>& box_size,
		      const vec4<float>& fill_color, const vec4<float>& edge_color)
{
  m_boxes.emplace_back (m_next_boxid++,
			vec3<double> (board_pos.x, board_pos.y, board_pos.z),
			vec3<double> (box_size), fill_color, edge_color);

  return m_boxes.back ().id ();
}

void
test_scene1::remove_box (unsigned int objid)
{
  auto i = std::find_if (m_boxes.begin (), m_boxes.end (),
			 [&] (auto&& b) { return b.id () == objid; });

  if (i != m_boxes.end ())
    m_boxes.erase (i);
}

void
test_scene1::remove_all_boxes (void)
{
  m_boxes.clear ();
  m_next_boxid = 0;
}

void
test_scene1::set_z_scale (float val)
{
  m_z_scale = val;
}

void
test_scene1::AutoRotate()
{
	m_bAutoRotate = !m_bAutoRotate;
}

void test_scene1::render (unsigned int width, unsigned int height,
			 std::chrono::microseconds delta_time,
			 bool en_wireframe, bool en_stairs_mode,
			 bool en_debug_dist, bool en_heightmap)
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

  vec2<float> aspect = width > height
		       ? vec2<float> (1, (float)width / -(float)height)
		       : vec2<float> ((float)height / (float)width, -1);

  m_last_proj_trv =
    mat4<double>::proj_perspective (utils::deg_to_rad (60.0f), aspect.x, aspect.y,
				    0.0001f, 1000.0f);

  auto viewport_trv =
    mat4<double>::scale (width * 0.5, height * 0.5, 1, 1)
    * mat4<double>::translate (1, 1, 0);
	
  if (m_bAutoRotate) 
  {
	  float r = utils::deg_to_rad(delta_time.count() * 0.00001f);
	  m_rotate_trv =
		  mat4<double>::translate(vec3<double>(-m_img_pos, 0))
		  * mat4<double>::rotate_z(r)
		  * mat4<double>::translate(vec3<double>(m_img_pos, 0))
		  * m_rotate_trv;
  }

  auto cam_trv = calc_cam_trv (m_zoom, m_tilt_angle, m_img_pos);

  if (m_image != nullptr)
  {
    m_image->render (cam_trv, m_last_proj_trv, viewport_trv,
		     en_wireframe, en_stairs_mode, en_debug_dist,
		     en_heightmap);
  }

  gl_check_log_error ();

  for (auto&& b : m_boxes)
    b.render (cam_trv, m_last_proj_trv, viewport_trv);
}
