
#ifndef includeguard_test_scene_includeguard
#define includeguard_test_scene_includeguard

#include <chrono>
#include <memory>
#include "gl/gl.hpp"

class test_scene
{
public:
  test_scene (const char* rgb_img, const char* height_img, unsigned int lod);
  ~test_scene (void);

  void render (unsigned int width, unsigned int height,
	       std::chrono::microseconds delta_time,
	       bool en_wireframe);

private:
  struct vertex;
  struct shader;

  std::unique_ptr<shader> m_shader;

  gl::buffer m_vertex_buffer;
  unsigned int m_vertex_buffer_count;

  gl::buffer m_index_buffer;
  gl::index_type m_index_buffer_type;
  unsigned int m_index_buffer_count;

  gl::buffer m_wireframe_index_buffer;
  gl::index_type m_wireframe_index_buffer_type;
  unsigned int m_wireframe_index_buffer_count;

  gl::texture m_color_texture;
  gl::texture m_height_texture;

  unsigned int m_frame_number;
  float m_rotate_angle;
};

#endif // includeguard_test_scene_includeguard
