
/*

full image size: 2048x5 x 2048x4 = 10240 x 8192

max lod level = 6, i.e. factor 1/64

10240 x 8192 / 64 = 160 x 128 ( = 40K tris)

- determine visible coarse-level tiles
  - for each visible tile
      calculate projected screen size/area
      if differecnce to actual image size of actual tile image size too
      high -> subdivide


use max texture size: 2048 x 2048

"biggest" (coverage) coarse level tile limited by max texture size.
with 2048 x 2048: 20 tiles


because the height grid geometry is constant at 256 x 256, this is
256 x 256 x 20 x 2 = 2.6M tris

use max texture size: 4096 x 4096
10240 x 8192 / 4096 = 3 x 2 = 6
256 x 256 x 6 x 2 = 787K tris

*/

#include <limits>
#include <iostream>
#include <algorithm>

#include "tiled_image.hpp"

// ----------------------------------------------------------------------------

struct tiled_image::vertex
{
  vec2<float> pos;

  vertex (void) { }
  vertex (const vec2<float>& xy) : pos (xy) { }
  vertex (float x, float y) : pos (x, y) { }
};

// ----------------------------------------------------------------------------

class tiled_image::grid_mesh
{
public:
  grid_mesh (const vec2<uint32_t>& size)
  {
    std::cout << "grid_mesh " << size.x << " x " << size.y << std::endl;

    std::vector<vertex> vtx;
    vtx.reserve (size.x * size.y);

    for (unsigned int y = 0; y < size.y; ++y)
      for (unsigned int x = 0; x < size.y; ++x)
	vtx.emplace_back ( (vec2<float> (x, y) * (1.0f / vec2<float> (size - 1))) * 2 - 1 );

    // make sure that the top edge y is -1
    // make sure that the bottom edge y is +1
    for (unsigned int x = 0; x < size.x; ++x)
    {
      vtx[x].pos.y = -1;
      vtx[(size.y-1) * size.x].pos.y = +1;
    }

    // make sure that the left edge x is -1
    // make sure that the right edge x is +1
    for (unsigned int y = 0; y < size.y; ++y)
    {
      vtx[y * size.x + 0].pos.x = -1;
      vtx[y * size.x + size.x-1].pos.x = +1;
    }

    m_vertex_buffer = gl::buffer (gl::buffer::vertex, vtx);
    m_vertex_buffer_count = vtx.size ();

    if (m_vertex_buffer_count - 1 <= std::numeric_limits<uint16_t>::max ())
      build_index_buffers<uint16_t> (size);
    else
      build_index_buffers<uint32_t> (size);
  }

  const vec2<uint32_t>& size (void) const { return m_size; }

  struct size_equals
  {
    vec2<uint32_t> ref;

    size_equals (const vec2<uint32_t>& r) : ref (r) { }

    bool operator () (const grid_mesh& m) const
    {
      return m.size ().x == ref.x && m.size ().y == ref.y;
    }

    bool operator () (const std::shared_ptr<grid_mesh>& m) const
    {
      return operator () (*m);
    }
  };

private:
  vec2<uint32_t> m_size;

  gl::buffer m_vertex_buffer;
  unsigned int m_vertex_buffer_count;

  gl::buffer m_index_buffer;
  gl::index_type m_index_buffer_type;
  unsigned int m_index_buffer_count;

  gl::buffer m_wireframe_index_buffer;
  gl::index_type m_wireframe_index_buffer_type;
  unsigned int m_wireframe_index_buffer_count;

  template <typename IndexType>
  void build_index_buffers (const vec2<uint32_t>& size)
  {
    // every cell in the grid consists of 2 triangles.
    std::vector<IndexType> idx;
    idx.reserve ((size.x - 1) * (size.y - 1) * 6);

    for (unsigned int y = 0; y < size.y - 1; ++y)
      for (unsigned int x = 0; x < size.x - 1; ++x)
      {
	idx.push_back ((x + 0) + ((y + 0) * size.x));
	idx.push_back ((x + 1) + ((y + 0) * size.x));
	idx.push_back ((x + 0) + ((y + 1) * size.x));

	idx.push_back ((x + 0) + ((y + 1) * size.x));
	idx.push_back ((x + 1) + ((y + 0) * size.x));
	idx.push_back ((x + 1) + ((y + 1) * size.x));
      }

    m_index_buffer = gl::buffer (gl::buffer::index, idx);
    m_index_buffer_type = gl::make_index_type<IndexType> ();
    m_index_buffer_count = idx.size ();

    // wireframe index buffer (quad edges only)
    idx.reserve (size.x * size.y * 2);
    idx.clear ();

    for (unsigned int y = 0; y < size.y - 1; ++y)
      for (unsigned int x = 0; x < size.x - 1; ++x)
      {
	idx.push_back ((x + 0) + ((y + 0) * size.x));
	idx.push_back ((x + 0) + ((y + 1) * size.x));

	idx.push_back ((x + 0) + ((y + 0) * size.x));
	idx.push_back ((x + 1) + ((y + 0) * size.x));
      }

    for (unsigned int y = 0; y < size.y - 1; ++y)
    {
      idx.push_back ((size.x-1 + 0) + ((y + 0) * size.y));
      idx.push_back ((size.x-1 + 0) + ((y + 1) * size.y));
    }

    for (unsigned int x = 0; x < size.x - 1; ++x)
    {
      idx.push_back ((x + 0) + ((size.y-1 + 0) * size.y));
      idx.push_back ((x + 1) + ((size.y-1 + 0) * size.y));
    }

    m_wireframe_index_buffer = gl::buffer (gl::buffer::index, idx);
    m_wireframe_index_buffer_type = gl::make_index_type<IndexType> ();
    m_wireframe_index_buffer_count = idx.size ();
  }

};

// shared grid meshes.
std::vector<std::shared_ptr<tiled_image::grid_mesh>> tiled_image::g_grid_meshes;


// ----------------------------------------------------------------------------

class tiled_image::tile
{
public:
  tile (const vec2<uint32_t>& pos, const vec2<uint32_t>& size, unsigned int lod)
  : m_pos (pos), m_size (size), m_lod (lod)
  {
    vec2<uint32_t> physical_size = std::max (size >> lod, { 1, 1 });

    std::cout << "tile physical size = " << physical_size.x << " x "
	      << physical_size.y << std::endl;

    auto i = std::find_if (g_grid_meshes.begin (), g_grid_meshes.end (),
			   grid_mesh::size_equals (size));

    if (i == g_grid_meshes.end ())
    {
      m_grid_mesh = std::make_shared<grid_mesh> (physical_size);
      g_grid_meshes.push_back (m_grid_mesh);
    }
  }

  tile (const tile&) = delete;
  tile& operator = (const tile&) = delete;

  tile (tile&& rhs)
  : m_pos (std::move (rhs.m_pos)),
    m_size (std::move (rhs.m_size)),
    m_lod (std::move (rhs.m_lod)),
    m_grid_mesh (std::move (rhs.m_grid_mesh))
  {
  }

  tile& operator = (tile&& rhs)
  {
    if (this != &rhs)
    {
      m_pos = std::move (rhs.m_pos);
      m_size = std::move (rhs.m_size);
      m_lod = std::move (rhs.m_lod);

      check_delete_grid_mesh_last_ref ();

      m_grid_mesh = std::move (rhs.m_grid_mesh);
    }
    return *this;
  }

  void check_delete_grid_mesh_last_ref (void)
  {
    if (m_grid_mesh != nullptr && m_grid_mesh.use_count () == 2)
    {
      auto i = std::find (g_grid_meshes.begin (), g_grid_meshes.end (), m_grid_mesh);
      if (i != g_grid_meshes.end ())
      {
	std::iter_swap (i, std::prev (g_grid_meshes.end ()));
	g_grid_meshes.pop_back ();
      }

      m_grid_mesh = nullptr;
    }
  }

  ~tile (void)
  {
    check_delete_grid_mesh_last_ref ();
  }

  // position of the tile in the original image.
  const vec2<uint32_t>& pos (void) const { return m_pos; }

  // size of the tile in pixels (original image coordinates).
  const vec2<uint32_t>& size (void) const { return m_size; }

  // level-of-detail number.  0 = highest level, which is a 1:1
  // mapping of the original data and the coverage of this tile.
  // higher level-of-detail tiles have the same size but cover more
  // pixels in the original image.
  unsigned int lod (void) const { return m_lod; }

  unsigned int scale_factor (void) const { return 1 << m_lod; }


private:
  vec2<uint32_t> m_pos;
  vec2<uint32_t> m_size;
  unsigned int m_lod;

  std::shared_ptr<grid_mesh> m_grid_mesh;

  // textures from the cache.
  //std::shared_ptr<gl::texture> m_rgb_texture;
  //std::shared_ptr<gl::texture> m_height_texture;
};


// ----------------------------------------------------------------------------

struct tiled_image::shader : public gl::shader
{
  uniform< mat4<float>, highp > mvp;
  uniform< vec4<float>, lowp > offset_color;

  uniform< sampler2D, mediump > color_texture;
  uniform< sampler2D, mediump > height_texture;

  uniform< float, highp> zbias;

  attribute< vec2<float>, highp > pos;

  shader (void)
  {
    named_parameter (mvp);
    named_parameter (pos);
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

std::shared_ptr<tiled_image::shader> tiled_image::g_shader;

// ----------------------------------------------------------------------------

tiled_image::tiled_image (void)
: m_size (0, 0)
{
}

tiled_image::tiled_image (const vec2<uint32_t>& size)
: m_size (size)
{
  if (size.x == 0 || size.y == 0)
    return;

  if (g_shader == nullptr)
    g_shader = std::make_shared<shader> ();

  m_shader = g_shader;

  // setup full image mipmaps.
  m_rgb_image[0] = image (pixel_format::rgba_8888, size.x, size.y);
  m_height_image[0] = image (pixel_format::l_8, size.x, size.y);

  generate_mipmaps ();


  // build tile map
}

tiled_image::tiled_image (tiled_image&& rhs)
: m_size (std::move (rhs.m_size)),
  m_rgb_image (std::move (rhs.m_rgb_image)),
  m_height_image (std::move (rhs.m_height_image)),
  m_shader (std::move (rhs.m_shader)),
  m_tiles (std::move (rhs.m_tiles))
{
  rhs.m_size = { 0, 0 };
}

tiled_image& tiled_image::operator = (tiled_image&& rhs)
{
  if (this != &rhs)
  {
    m_size = std::move (rhs.m_size);
    m_rgb_image = std::move (rhs.m_rgb_image);
    m_height_image = std::move (rhs.m_height_image);
    m_shader = std::move (rhs.m_shader);
    m_tiles = std::move (rhs.m_tiles);

    if (g_shader != nullptr && g_shader.use_count () == 1)
      g_shader = nullptr;

    rhs.m_size = { 0, 0 };
  }
  return *this;
}

tiled_image::~tiled_image (void)
{
  if (m_shader != nullptr && m_shader.use_count () == 2)
    g_shader = nullptr;
}


void tiled_image::generate_mipmaps (void)
{
  for (unsigned int i = 1; i < max_lod_level; ++i)
  {
    m_rgb_image[i] = m_rgb_image[i - 1].pyr_down ();
    m_height_image[i] = m_height_image[i - 1].pyr_down ();
  }
}

void tiled_image::fill (int32_t x, int32_t y, uint32_t width, uint32_t height,
			unsigned int r, unsigned int g, unsigned int b,
			unsigned int z)
{

}

