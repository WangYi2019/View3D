#ifndef includeguard_simple_3dbox_hpp_includeguard
#define includeguard_simple_3dbox_hpp_includeguard

#include <memory>
#include "utils/vec_mat.hpp"
#include "gl/gl.hpp"

class simple_3dbox
{
public:
  simple_3dbox (void) = delete;

  simple_3dbox (unsigned int id,
		const utils::vec3<double>& pos, const utils::vec3<double>& size,
		const utils::vec4<float>& fill_color, const utils::vec4<float>& edge_color);

  simple_3dbox (const simple_3dbox& rhs);
  simple_3dbox (simple_3dbox&& rhs);

  simple_3dbox& operator = (const simple_3dbox& rhs);
  simple_3dbox& operator = (simple_3dbox&& rhs);

  unsigned int id (void) const { return m_id; }


  void render (const utils::mat4<double>& cam_trv, const utils::mat4<double>& proj_trv,
	       const utils::mat4<double>& viewport_trv, float zscale) const;

  utils::mat4<double> trv (float zscale) const;

private:
  struct vertex;
  struct shader;
  struct mesh;

  static std::shared_ptr<shader> g_shader;
  static std::shared_ptr<mesh> g_mesh;

  unsigned int m_id;

  utils::vec3<double> m_pos;
  utils::vec3<double> m_size;

  utils::vec4<float> m_fill_color;
  utils::vec4<float> m_edge_color;

  std::shared_ptr<shader> m_shader;
  std::shared_ptr<mesh> m_mesh;
};

#endif // includeguard_simple_3dbox_hpp_includeguard
