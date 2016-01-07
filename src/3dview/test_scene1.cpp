
#include <type_traits>
#include <vector>
#include <iostream>
#include <string>

#include "test_scene1.hpp"
#include "tiled_image.hpp"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

test_scene1::test_scene1 (void)
{
  m_image = std::make_unique<tiled_image> (vec2<unsigned int> (8469, 10192));

//  m_image->update (0, 0, "outputRGB.bmp", "output.bmp");

  std::string base_path = "/home/oleg/20151130133409/";

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
/*
  for (auto&& i : images)
    m_image->update (i.x, i.y,
		     (base_path + i.rgb_img).c_str (),
		     (base_path + i.height_img).c_str ());
*/
}

test_scene1::~test_scene1 (void)
{
}

void test_scene1::render (unsigned int width, unsigned int height,
			 std::chrono::microseconds delta_time,
			 bool en_wireframe)
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


  auto proj_trv =
    mat4<double>::proj_perspective (M_PI/3, (float)width / -(float)height,
                                    0.1f, 10000.0f);

  m_rotate_angle += delta_time.count () * 0.0000001f;
  if (m_rotate_angle > 2*M_PI)
    m_rotate_angle -= 2*M_PI;

  auto cam_trv = mat4<double>::identity ()

      // move camera away a bit
      * mat4<double>::translate (0, 0, -1.5)

      // rotate around (0,0) center
//      * mat4<double>::rotate_x (m_rotate_angle)
      * mat4<double>::rotate_x (M_PI * -0.24f)
      * mat4<double>::rotate_z (m_rotate_angle)

      // scale to -1,+1 range (based on max (width, height))
      * mat4<double>::scale (vec3<double> ( (2 / vec2<double> (std::max (m_image->size ().x, m_image->size ().y))) * 0.5, 1))

      // move image to (0,0) center
      * mat4<double>::translate (vec3<double> (vec2<double> (m_image->size ()) * -0.5, 0))

      * mat4<double>::identity ();

  m_image->render (cam_trv, proj_trv);

  gl_check_log_error ();
}
