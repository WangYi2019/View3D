
#include <type_traits>
#include <vector>
#include <iostream>

#include "test_scene1.hpp"
#include "tiled_image.hpp"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

test_scene1::test_scene1 (void)
{
  m_image = std::make_unique<tiled_image> (vec2<unsigned int> (4096, 4096));

  m_image->update (0, 0, "outputRGB.bmp", "output.bmp");
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
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_2D);
//  glDepthFunc (GL_LEQUAL);
  glDepthFunc (GL_LESS);

  glDisable (GL_STENCIL_TEST);
  glDisable (GL_BLEND);

  mat4<float> proj_trv =
    mat4<float>::proj_perspective (M_PI/3, (float)width / (float)height,
                                   0.1f, 10000.0f);

  m_rotate_angle += delta_time.count () * 0.0000001f;
  if (m_rotate_angle > 2*M_PI)
    m_rotate_angle -= 2*M_PI;

#if 1
  mat4<float> cam_trv = mat4<float>::identity ()
//      * mat4<float>::translate (0, 0, -0.5f - std::abs (std::sin (frame_number * 0.0025f)) * 5)
      * mat4<float>::translate (0, 0, -2.0f)
      * mat4<float>::rotate_x (M_PI * 0.24f)
      * mat4<float>::rotate_z (m_rotate_angle)
      * mat4<float>::identity ();
#else
  mat4<float> cam_trv = mat4<float>::identity ()
      * mat4<float>::translate (0, 0, -0.5)
      * mat4<float>::rotate_x (0)
      * mat4<float>::rotate_z (0)
      * mat4<float>::identity ();
#endif

  auto mvp = proj_trv * cam_trv;

  m_image->render (mvp);

  gl_check_log_error ();
}
