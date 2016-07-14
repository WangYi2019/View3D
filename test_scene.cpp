
#include <type_traits>
#include <vector>
#include <iostream>

#include "utils/bits.hpp"
#include "img/bmp_loader.hpp"

#include "test_scene.hpp"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

using utils::vec2;
using utils::vec4;
using utils::mat4;

using utils::ceil_pow2;

using img::image;
using img::load_bmp_image;
using img::pixel_format;

struct test_scene::vertex
{
  vec2<float> pos;

  vertex (void) { }
  vertex (const vec2<float>& xy) : pos (xy) { }
  vertex (float x, float y) : pos (x, y) { }
};

struct test_scene::shader : public gl::shader
{
  uniform< mat4<float>, highp > mvp;
  uniform< vec4<float>, lowp > color;
  uniform< vec4<float>, lowp > offset_color;

  uniform< sampler2D, mediump > color_texture;
  uniform< sampler2D, mediump > height_texture;

  uniform< float, highp> zbias;

  attribute< vec2<float>, highp > pos;

  shader (void)
  {
    named_parameter (mvp);
    named_parameter (pos);
    named_parameter (color);
    named_parameter (offset_color);
    named_parameter (color_texture);
    named_parameter (height_texture);
    named_parameter (zbias);
  }

  vertex_shader_text
  (
    varying vec2 color_uv;

    void main (void)
    {
      // the vertex positions are in the range -1 ... +1
      // the texture coordinates are in the range 0...1
      vec2 uv = (pos + 1.0) * 0.5;

      vec4 height = texture2D (height_texture, uv);

      color_uv = uv;
      gl_Position = mvp * vec4 (pos, height.r * 0.1 + zbias, 1.0);
    }
  )

  fragment_shader_text
  (
    varying vec2 color_uv;

    void main (void)
    {
      gl_FragColor = texture2D (color_texture, color_uv) + offset_color;
    }
  )
};


test_scene::test_scene (const char* rgb_img_file, const char* height_img_file, unsigned int lod)
{
  lod = std::max (lod, (unsigned int)1);

  image color_img;
  image height_img;


  color_img = load_bmp_image (rgb_img_file);
//  height_img = load_bmp_image (height_img_file);


  if (color_img.empty ())
  {
    color_img = image (pixel_format::rgba_f32, { 32, 32 });
    color_img.fill ({ 1, 0, 0, 0 });
  }

  if (height_img.empty ())
  {
    height_img = image (pixel_format::l_8, { 32, 32 });
    height_img.fill ({ 0, 0, 0, 0 });
   // height_img.fill (0.125f, 0.5, 0.25f, 1);
  }

/*
  {
    image tmp (pixel_format::l_8, { 32, 32 });
    tmp.fill ({ 0.125f, 0.5, 0.25f, 1 });

    tmp.copy_to ({0, 0}, {16, 8}, color_img);
    //color_img.fill (0, 0, 16, 8, 0.0f, 0.5f, 0.0f, 0);
  }
*/

  {
    image tmp (pixel_format::rgba_8888,
	       { ceil_pow2 (color_img.width ()), ceil_pow2 (color_img.height ()) });

//    color_img.copy_to (tmp);
/*
    tmp = tmp.pyr_down ();
    tmp = tmp.pyr_down ();
    tmp = tmp.pyr_down ();
    tmp = tmp.pyr_down ();

    color_img = std::move (tmp);
*/
    
//    tmp.pyr_down_to (color_img);

std::cout << "color_img = " << color_img.width () << " x " << color_img.height () << std::endl;
  }


/*
  {
    image tmp = color_img.subimg ({0, 0}, {512,512});
//    color_img = tmp.pyr_down ();
//    tmp.pyr_down_to (color_img);
    tmp.copy_to (color_img);
    tmp.pyr_down_to (color_img);
  }
*/

/*
  m_color_texture = gl::texture (color_img.format (), { 2048*2, 2048*2 });

  m_color_texture.upload (color_img.data (), { 1024, 1024*2 },
			  color_img.size (), color_img.bytes_per_line ());
*/


  m_color_texture = gl::texture (color_img.format (), color_img.size (),
				 color_img.data (), color_img.bytes_per_line ());


  m_color_texture.set_min_filter (gl::texture::linear_mipmap_linear);
  m_color_texture.set_mag_filter (gl::texture::linear);
  m_color_texture.generate_mipmaps ();

  m_height_texture = gl::texture (height_img.format (), height_img.size (),
				  height_img.data (), height_img.bytes_per_line ());
  m_height_texture.set_min_filter (gl::texture::linear_mipmap_linear);
  m_height_texture.set_mag_filter (gl::texture::linear);
  m_height_texture.generate_mipmaps ();

  const unsigned int gridsize = std::max (2u, std::max (height_img.width (),
						       height_img.height ()) / lod);
  typedef uint32_t index_type;

  std::vector<vertex> vtx;
  vtx.reserve (gridsize * gridsize);

  for (unsigned int y = 0; y < gridsize; ++y)
    for (unsigned int x = 0; x < gridsize; ++x)
      vtx.emplace_back ( (vec2<float> (x, y) * (1.0f / (gridsize - 1))) * 2 - 1 );

  // make sure that the top edge y is -1
  // make sure that the bottom edge y is +1
  for (unsigned int x = 0; x < gridsize; ++x)
  {
    vtx[x].pos.y = -1;
    vtx[(gridsize-1)*gridsize].pos.y = +1;
  }

  // make sure that the left edge x is -1
  // make sure that the right edge x is +1
  for (unsigned int y = 0; y < gridsize; ++y)
  {
    vtx[y * gridsize + 0].pos.x = -1;
    vtx[y * gridsize + gridsize-1].pos.x = +1;
  }

  m_vertex_buffer = gl::buffer (gl::buffer::vertex, vtx);
  m_vertex_buffer_count = (unsigned int)vtx.size ();

  // every cell in the grid consists of 2 triangles.
  std::vector<index_type> idx;
  idx.reserve ((gridsize - 1) * (gridsize - 1) * 6);

  for (unsigned int y = 0; y < gridsize - 1; ++y)
    for (unsigned int x = 0; x < gridsize - 1; ++x)
    {
      idx.push_back ((x + 0) + ((y + 0) * gridsize));
      idx.push_back ((x + 1) + ((y + 0) * gridsize));
      idx.push_back ((x + 0) + ((y + 1) * gridsize));

      idx.push_back ((x + 0) + ((y + 1) * gridsize));
      idx.push_back ((x + 1) + ((y + 0) * gridsize));
      idx.push_back ((x + 1) + ((y + 1) * gridsize));
    }

  m_index_buffer = gl::buffer (gl::buffer::index, idx);
  m_index_buffer_type = gl::make_index_type<index_type> ();
  m_index_buffer_count = (unsigned int)idx.size ();

  // wireframe index buffer (quad edges only)
  idx.reserve (gridsize * gridsize * 2);
  idx.clear ();

  for (unsigned int y = 0; y < gridsize - 1; ++y)
    for (unsigned int x = 0; x < gridsize - 1; ++x)
    {
      idx.push_back ((x + 0) + ((y + 0) * gridsize));
      idx.push_back ((x + 0) + ((y + 1) * gridsize));

      idx.push_back ((x + 0) + ((y + 0) * gridsize));
      idx.push_back ((x + 1) + ((y + 0) * gridsize));
    }

  for (unsigned int y = 0; y < gridsize - 1; ++y)
  {
    idx.push_back ((gridsize-1 + 0) + ((y + 0) * gridsize));
    idx.push_back ((gridsize-1 + 0) + ((y + 1) * gridsize));
  }

  for (unsigned int x = 0; x < gridsize - 1; ++x)
  {
    idx.push_back ((x + 0) + ((gridsize-1 + 0) * gridsize));
    idx.push_back ((x + 1) + ((gridsize-1 + 0) * gridsize));
  }

  m_wireframe_index_buffer = gl::buffer (gl::buffer::index, idx);
  m_wireframe_index_buffer_type = gl::make_index_type<index_type> ();
  m_wireframe_index_buffer_count = (unsigned int)idx.size ();


  m_frame_number = 0;
  m_rotate_angle = 0;

  gl_check_log_error ();

  m_shader = std::make_unique<shader> ();

  gl_check_log_error ();
}

test_scene::~test_scene (void)
{
}

void test_scene::render (unsigned int width, unsigned int height,
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

  m_shader->activate ();

  mat4<float> proj_trv =
    mat4<float>::proj_perspective ((float)(M_PI/3), 1, (float)width / -(float)height,
                                   0.1f, 10000.0f);

  m_rotate_angle += delta_time.count () * 0.0000001f;
  if (m_rotate_angle > (float)(2*M_PI))
    m_rotate_angle -= (float)(2*M_PI);

#if 0
  mat4<float> cam_trv = mat4<float>::identity ()
//      * mat4<float>::translate (0, 0, -0.5f - std::abs (std::sin (frame_number * 0.0025f)) * 5)
      * mat4<float>::translate (0, 0, -4.0f)
      * mat4<float>::rotate_x (M_PI * -0.24f)
      * mat4<float>::rotate_z (m_rotate_angle)
      * mat4<float>::identity ();
#else
  mat4<float> cam_trv = mat4<float>::identity ()
      * mat4<float>::translate (0, 0, -3.5f)
      * mat4<float>::rotate_x (0)
      * mat4<float>::rotate_z (0)
      * mat4<float>::identity ();
#endif


  m_shader->mvp = proj_trv * cam_trv;
  m_shader->pos = gl::vertex_attrib (m_vertex_buffer, &vertex::pos);

  m_color_texture.bind (0);
  m_height_texture.bind (1);

  m_shader->color_texture = 0;
  m_shader->height_texture = 1;

  m_shader->offset_color = vec4<float> (0, 0, 0, 0);
  m_shader->color = vec4<float> (1, 0, 0, 0);
  m_shader->zbias = 0;
  gl::draw_indexed (gl::triangles, sizeof (vertex),
		    m_index_buffer, m_index_buffer_type, m_index_buffer_count);


  if (en_wireframe)
  {
    m_shader->color = vec4<float> (1, 1, 1, 1);
    m_shader->offset_color = vec4<float> (1, 1, 1, 1);
    m_shader->zbias = 0.001f;

    glLineWidth (0.5f);

    gl::draw_indexed (gl::lines, sizeof (vertex),
		      m_wireframe_index_buffer, m_wireframe_index_buffer_type,
		      m_wireframe_index_buffer_count);
  }

  gl_check_log_error ();
}
