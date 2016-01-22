
#ifndef includeguard_test_scene1_includeguard
#define includeguard_test_scene1_includeguard

#include <chrono>
#include <memory>
#include "utils/vec_mat.hpp"

class tiled_image;
class simple_3dbox;

class test_scene1
{
public:
  test_scene1 (void);
  test_scene1 (const char* file_desc_file);
  ~test_scene1 (void);

  void render (unsigned int width, unsigned int height,
	       std::chrono::microseconds delta_time,
	       bool en_wireframe, bool en_debug_dist);

  const vec2<double>& img_pos (void) const { return m_img_pos; }
  void set_img_pos (const vec2<double>& v) { m_img_pos = v; }

  vec2<double> screen_to_img (void) const;

  float tilt_angle (void) const { return m_tilt_angle; }
  void set_tilt_angle (float val);

  float rotate_angle (void) const { return m_rotate_angle; }
  void set_rotate_angle (float val);

  float zoom (void) const { return m_zoom; }
  void set_zoom (float val);

  void reset_view (void);

  const std::unique_ptr<tiled_image>& image (void) const { return m_image; }
  void resize_image (const vec2<unsigned int>& size);

  void center_image (const vec2<unsigned int>& image_point);

  unsigned int
  add_box (const vec2<unsigned int>& board_pos,
	   const vec3<unsigned int>& box_size,
	   const vec4<float>& fill_color, const vec4<float>& edge_color);

  void remove_box (unsigned int objid);
  void remove_all_boxes (void);

private:
  std::unique_ptr<tiled_image> m_image;
  std::vector<simple_3dbox> m_boxes;

  unsigned int m_frame_number;
  float m_rotate_angle;

  float m_tilt_angle;
  float m_zoom;

  vec2<double> m_img_pos;

  mat4<double> m_last_proj_trv;
  vec2<double> m_last_screen_size;

  unsigned int m_next_boxid = 0;

  mat4<double> calc_cam_trv (float zoom, float tilt_angle, float rot_angle,
			     const vec2<double>& scroll) const;
};

#endif // includeguard_test_scene_includeguard
