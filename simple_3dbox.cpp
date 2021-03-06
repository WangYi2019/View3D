
#include "simple_3dbox.hpp"
#include "utils/langcomp.hpp"

using utils::vec3;
using utils::vec4;
using utils::mat4;

// ----------------------------------------------------------------------------

struct simple_3dbox::vertex
{
  vec3<float> pos;

  vertex (void) { };
  vertex (const vec3<float>& p) : pos (p) { }
  vertex (float x, float y, float z) : pos (x, y, z) { }
};

// ----------------------------------------------------------------------------

struct simple_3dbox::shader : public gl::shader
{
  uniform< mat4<float>, highp > mvp;
  uniform< vec4<float>, lowp > color;

  attribute< vec3<float>, highp > pos;

  shader (void)
  {
    named_parameter (mvp);
    named_parameter (color);
    named_parameter (pos);
  }

  virtual std::vector<const char*> vertex_shader_text_str (void) override { return { linenum_prefix R"gltext(

    void main (void)
    {
      gl_Position = mvp * vec4 (pos.x, pos.y, pos.z, 1.0);
    }

  )gltext" }; }

  virtual std::vector<const char*> fragment_shader_text_str (void) override { return { linenum_prefix R"gltext(

    void main (void)
    {
      gl_FragColor = color;
    }

  )gltext" }; }
};

std::shared_ptr<simple_3dbox::shader> simple_3dbox::g_shader;

// ----------------------------------------------------------------------------

struct simple_3dbox::mesh
{
  gl::buffer vertices;
  unsigned int vertices_count;

  gl::buffer solid_indices;
  unsigned int solid_indices_count;

  gl::buffer edge_indices;
  unsigned int edge_indices_count;

  mesh (void)
  {
    const float z_top = 1;
    const float z_bot = 0;

    std::vector<vertex> vtx =
    {
      { 0, 0, z_top }, { 1, 0, z_top }, { 1, 1, z_top }, { 0, 1, z_top },
      { 0, 0, z_bot }, { 1, 0, z_bot }, { 1, 1, z_bot }, { 0, 1, z_bot }
    };

    vertices = gl::buffer (gl::buffer::vertex, vtx);
    vertices_count = 8;

    std::vector<uint16_t> idx =
    {
      0, 1, 2, 0, 2, 3,
      4, 5, 6, 4, 6, 7,
      3, 2, 6, 3, 6, 7,
      2, 1, 5, 2, 5, 6,
      1, 0, 4, 1, 4, 5,
      0, 3, 7, 0, 7, 4
    };

    solid_indices = gl::buffer (gl::buffer::index, idx);
    solid_indices_count = (unsigned int)idx.size ();

    idx =
    {
      0, 1, 1, 2, 2, 3, 3, 0,
      4, 5, 5, 6, 6, 7, 7, 4,
      0, 4,
      1, 5,
      3, 7,
      2, 6
    };

    edge_indices = gl::buffer (gl::buffer::index, idx);
    edge_indices_count = (unsigned int)idx.size ();
  }
};

std::shared_ptr<simple_3dbox::mesh> simple_3dbox::g_mesh;

// ----------------------------------------------------------------------------

simple_3dbox::simple_3dbox (unsigned int id,
			    const vec3<double>& pos, const vec3<double>& size,
			    const vec4<float>& fill_color, const vec4<float>& edge_color)
: m_id (id), m_pos (pos), m_size (size), m_fill_color (fill_color), m_edge_color (edge_color)
{
  if (g_shader == nullptr)
    g_shader = std::make_shared<shader> ();
  if (g_mesh == nullptr)
    g_mesh = std::make_shared<mesh> ();

  m_shader = g_shader;
  m_mesh = g_mesh;
}

simple_3dbox::simple_3dbox (const simple_3dbox& rhs) = default;
simple_3dbox::simple_3dbox (simple_3dbox&& rhs) = default;
simple_3dbox& simple_3dbox::operator = (const simple_3dbox& rhs) = default;
simple_3dbox& simple_3dbox::operator = (simple_3dbox&& rhs) = default;

mat4<double> simple_3dbox::trv (void) const
{
  return
	mat4<double>::translate (m_pos.x, m_pos.y, m_pos.z)
	 * mat4<double>::scale (m_size);
}

void simple_3dbox::render (const mat4<double>& cam_trv,
			   const mat4<double>& proj_trv,
			   const mat4<double>& viewport_trv) const
{
  m_shader->activate ();
  m_shader->mvp = mat4<float> (proj_trv * cam_trv * trv ());
  m_shader->pos = gl::vertex_attrib (m_mesh->vertices, &vertex::pos);

  glDisable (GL_TEXTURE_2D);
  glEnable (GL_DEPTH_TEST);
  glDepthMask (false);
  glEnable (GL_CULL_FACE);


  m_shader->color = m_fill_color;

  if (m_fill_color.a < 1)
  {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);

  gl::draw_indexed (gl::triangles, sizeof (vertex),
		    m_mesh->solid_indices, gl::make_index_type<uint16_t> (),
		    m_mesh->solid_indices_count);


  m_shader->color = m_edge_color;

  if (m_edge_color.a < 1)
  {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);

  glLineWidth (1.5f);

  gl::draw_indexed (gl::lines, sizeof (vertex),
		    m_mesh->edge_indices, gl::make_index_type<uint16_t> (),
		    m_mesh->edge_indices_count);


  glDepthMask (true);
}

