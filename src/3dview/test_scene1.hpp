
#ifndef includeguard_test_scene1_includeguard
#define includeguard_test_scene1_includeguard

#include <chrono>
#include <memory>

class tiled_image;

class test_scene1
{
public:
  test_scene1 (void);
  ~test_scene1 (void);

  void render (unsigned int width, unsigned int height,
	       std::chrono::microseconds delta_time,
	       bool en_wireframe);

private:
  std::unique_ptr<tiled_image> m_image;

  unsigned int m_frame_number;
  float m_rotate_angle;
};

#endif // includeguard_test_scene_includeguard
