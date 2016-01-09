
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
#include <chrono>
#include <future>

#include "tiled_image.hpp"
#include "bmp_loader.hpp"

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

    m_size = size;

    std::vector<vertex> vtx;
    vtx.reserve (size.x * size.y);

    for (unsigned int y = 0; y < size.y; ++y)
      for (unsigned int x = 0; x < size.x; ++x)
	vtx.emplace_back (vec2<float> (x, y) * (1.0f / vec2<float> (size - 1)));

    // make sure that the top edge y is 0
    // make sure that the bottom edge y is 1
    for (unsigned int x = 0; x < size.x; ++x)
    {
      vtx[x].pos.y = 0;
      vtx[(size.y-1) * size.x].pos.y = 1;
    }

    // make sure that the left edge x is 0
    // make sure that the right edge x is 1
    for (unsigned int y = 0; y < size.y; ++y)
    {
      vtx[y * size.x + 0].pos.x = 0;
      vtx[y * size.x + size.x-1].pos.x = 1;
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

  void render_textured (void) const
  {
    gl::draw_indexed (gl::triangles, sizeof (vertex),
		      m_index_buffer, m_index_buffer_type, m_index_buffer_count);
  }

  void render_wireframe (void) const
  {
    gl::draw_indexed (gl::lines, sizeof (vertex),
		      m_wireframe_index_buffer, m_index_buffer_type,
		      m_wireframe_index_buffer_count);
  }

  void render_outline (void) const
  {
    gl::draw_indexed (gl::lines, sizeof (vertex),
		      m_outline_index_buffer, m_index_buffer_type,
		      m_outline_index_buffer_count);
  }

  const gl::buffer& vertex_buffer (void) const { return m_vertex_buffer; }

private:
  vec2<uint32_t> m_size;

  gl::buffer m_vertex_buffer;
  unsigned int m_vertex_buffer_count;

  // the index buffer type is the same for all index buffers.
  gl::index_type m_index_buffer_type;

  gl::buffer m_index_buffer;
  unsigned int m_index_buffer_count;

  gl::buffer m_wireframe_index_buffer;
  unsigned int m_wireframe_index_buffer_count;

  gl::buffer m_outline_index_buffer;
  unsigned int m_outline_index_buffer_count;

  template <typename IndexType>
  void build_index_buffers (const vec2<uint32_t>& size)
  {
    m_index_buffer_type = gl::make_index_type<IndexType> ();

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
    m_index_buffer_count = idx.size ();

    // wireframe index buffer (quad edges only)
    idx.reserve (size.x * size.y * 4);
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
      idx.push_back ((size.x-1 + 0) + ((y + 0) * size.x));
      idx.push_back ((size.x-1 + 0) + ((y + 1) * size.x));
    }

    for (unsigned int x = 0; x < size.x - 1; ++x)
    {
      idx.push_back ((x + 0) + ((size.y-1 + 0) * size.x));
      idx.push_back ((x + 1) + ((size.y-1 + 0) * size.x));
    }

    m_wireframe_index_buffer = gl::buffer (gl::buffer::index, idx);
    m_wireframe_index_buffer_count = idx.size ();

    // outline index buffer
    idx.reserve (4*2);
    idx.clear ();

    idx.push_back (0 + 0 * size.x);
    idx.push_back ((size.x - 1) + 0 * size.x);

    idx.push_back ((size.x - 1) + 0 * size.x);
    idx.push_back ((size.x - 1) + (size.y - 1) * size.x);

    idx.push_back ((size.x - 1) + (size.y - 1) * size.x);
    idx.push_back (0 + (size.y - 1) * size.x);

    idx.push_back (0 + (size.y - 1) * size.x);
    idx.push_back (0 + 0 * size.x);

    m_outline_index_buffer = gl::buffer (gl::buffer::index, idx);
    m_outline_index_buffer_count = idx.size ();
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
/*
    std::cout << "tile pos = (" << pos.x << "," << pos.y << ")"
	      << " size = (" << size.x << "," << size.y << ")"
	      << " lod = " << lod
	      << " phys.size = (" << physical_size.x << "," << physical_size.y << ")"
	      << std::endl;
*/
    auto i = std::find_if (g_grid_meshes.begin (), g_grid_meshes.end (),
			   grid_mesh::size_equals (physical_size));

    if (i == g_grid_meshes.end ())
    {
      m_grid_mesh = std::make_shared<grid_mesh> (physical_size);
      g_grid_meshes.push_back (m_grid_mesh);
    }
    else
      m_grid_mesh = *i;

    m_trv = mat4<double>::translate (vec3<double> (m_pos, 0))
	    * mat4<double>::scale (vec3<double> (m_size, 1));

    m_subtiles.fill (nullptr);
  }

  tile (const tile&) = delete;
  tile& operator = (const tile&) = delete;

  tile (tile&& rhs)
  : m_pos (std::move (rhs.m_pos)),
    m_size (std::move (rhs.m_size)),
    m_lod (std::move (rhs.m_lod)),
    m_grid_mesh (std::move (rhs.m_grid_mesh)),
    m_trv (std::move (rhs.m_trv)),
    m_subtiles (std::move (rhs.m_subtiles))
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
      m_trv = std::move (rhs.m_trv);
      m_subtiles = std::move (rhs.m_subtiles);
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

  // size of the tile in actual pixels (stored image coordinates).
  vec2<uint32_t> physical_size (void) const { return m_size >> m_lod; }


  // level-of-detail number.  0 = highest level, which is a 1:1
  // mapping of the original data and the coverage of this tile.
  // higher level-of-detail tiles have the same size but cover more
  // pixels in the original image.
  unsigned int lod (void) const { return m_lod; }

  unsigned int scale_factor (void) const { return 1 << m_lod; }

  const grid_mesh& mesh (void) const
  {
    assert (m_grid_mesh != nullptr);
    return *m_grid_mesh;
  }

  const mat4<double>& trv (void) const { return m_trv; }

  const std::array<tile*, 4> subtiles (void) const { return m_subtiles; }
  void set_subtiles (const std::array<tile*, 4>& t) { m_subtiles = t; }

  bool has_subtiles (void) const
  {
    for (auto&& t : m_subtiles)
      if (t != nullptr)
	return true;
    return false;
  }

private:
  vec2<uint32_t> m_pos;
  vec2<uint32_t> m_size;
  unsigned int m_lod;

  std::shared_ptr<grid_mesh> m_grid_mesh;

  mat4<double> m_trv;

  // one tile (lower detail level) is subdivided into
  // 4 tiles (higher detail level)
  std::array<tile*, 4> m_subtiles;

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
      vec2 uv = pos;

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

  // setup mipmaps for the whole image.
  {
    vec2<unsigned int> sz (size);
    for (unsigned int i = 0; i < max_lod_level && sz.x > 0 && sz.y > 0;
	 ++i, sz /= 2)
    {
      std::cout << "tiled_image new mipmap level " << sz.x << " x " << sz.y << std::endl;
/*
      // for lower levels, use lower color resolution images.
      m_rgb_image[i] = image (sz.x <= 512 || sz.y <= 512
			      ? pixel_format::rgb_555
			      : pixel_format::rgba_8888, sz);
*/
      m_rgb_image[i] = image (pixel_format::rgba_8888, sz);
//    m_rgb_image[i] = image (pixel_format::bgr_888, sz);
      m_rgb_image[i].fill ({ 0 });

      m_height_image[i] = image (pixel_format::l_8, sz);
      m_height_image[i].fill ({ 0 });
    }
  }

  // setup tiles
  for (unsigned int i = 0; i < max_lod_level; ++i)
  {
    const auto physical_tile_size = tile_grid_size << i;

    auto num_tiles_tile = (size + (physical_tile_size-1)) / physical_tile_size;
    m_tiles[i].reserve (num_tiles_tile.x * num_tiles_tile.y);

    std::cout << "lod " << i
	      << " num_tiles_tile = " << num_tiles_tile.x << " x " << num_tiles_tile.y << std::endl;

    for (unsigned int y = 0; y < size.y; y += physical_tile_size)
      for (unsigned int x = 0; x < size.x; x += physical_tile_size)
      {
	vec2<uint32_t> tile_tl (x, y);
	vec2<uint32_t> tile_br = std::min (tile_tl + physical_tile_size, size);

	m_tiles[i].emplace_back (tile_tl, tile_br - tile_tl, i);
      }
  }

  // link tiles
  for (unsigned int i = max_lod_level-1; i > 0; --i)
  {
    const auto physical_tile_size = tile_grid_size << i;
    auto num_tiles = (size + (physical_tile_size-1)) / physical_tile_size;

    const auto parent_physical_tile_size = tile_grid_size << (i-1);
    auto parent_num_tiles = (size + (parent_physical_tile_size-1)) / parent_physical_tile_size;


    for (unsigned int y = 0; y < num_tiles.y; ++y)
      for (unsigned int x = 0; x < num_tiles.x; ++x)
      {
	// this is the lower detail level tile, which has up to 4 higher level
	// tiles.  notice that at the borders there might be some missing
	// higher level tiles.
	auto& this_tile = m_tiles[i].at (x + y * num_tiles.x);
	auto& subtiles = m_tiles[i-1];

	std::array<tile*, 4> t;

	vec2<unsigned int> subcoords[4] =
	{
	  { x*2 + 0, y*2 + 0 },
	  { x*2 + 1, y*2 + 0 },
	  { x*2 + 0, y*2 + 1 },
	  { x*2 + 1, y*2 + 1 }
	};

	for (unsigned int ii = 0; ii < 4; ++ii)
	{
	  if (subcoords[ii].x < parent_num_tiles.x
	      && subcoords[ii].y < parent_num_tiles.y)
	    t[ii] = &subtiles.at (subcoords[ii].x + subcoords[ii].y*parent_num_tiles.x);
	  else
	    t[ii] = nullptr;
	}
	this_tile.set_subtiles (t);
      }
  }

}

tiled_image::tiled_image (tiled_image&& rhs)
: m_size (std::move (rhs.m_size)),
  m_rgb_image (std::move (rhs.m_rgb_image)),
  m_height_image (std::move (rhs.m_height_image)),
  m_shader (std::move (rhs.m_shader)),
  m_tiles (std::move (rhs.m_tiles)),
  m_candidate_tiles (std::move (rhs.m_candidate_tiles)),
  m_visible_tiles (std::move (rhs.m_visible_tiles))
{
  rhs.m_size = { 0 };
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
    m_candidate_tiles = std::move (rhs.m_candidate_tiles);
    m_visible_tiles = std::move (rhs.m_visible_tiles);

    if (g_shader != nullptr && g_shader.use_count () == 1)
      g_shader = nullptr;

    rhs.m_size = { 0 };
  }
  return *this;
}

tiled_image::~tiled_image (void)
{
  if (m_shader != nullptr && m_shader.use_count () == 2)
    g_shader = nullptr;
}


void tiled_image::fill (int32_t x, int32_t y, uint32_t width, uint32_t height,
			float r, float g, float b, float z)
{
  auto rgb_area = m_rgb_image[0].fill ({ x, y }, { width, height }, { r, g, b, 1 });
  auto z_area = m_height_image[0].fill ({ x, y }, { width, height }, { z, z, z, 1 });

  update_mipmaps (m_rgb_image, rgb_area.top_left, rgb_area.size);
  update_mipmaps (m_height_image, z_area.top_left, z_area.size);
}

void
tiled_image::update (int32_t x, int32_t y,
		     const char* rgb_bmp_file,
		     const char* height_bmp_file)
{
  update (x, y, rgb_bmp_file, height_bmp_file, 0, 0,
	  std::numeric_limits<unsigned int>::max (),
	  std::numeric_limits<unsigned int>::max ());
}

void
tiled_image::update (int32_t x, int32_t y,
		     const char* rgb_bmp_file,
		     const char* height_bmp_file,
		     unsigned int src_x, unsigned int src_y,
		     unsigned int src_width, unsigned int src_height)
{
  auto t0 = std::chrono::high_resolution_clock::now ();

  // load and copy the new data to the top-level mipmap level and then selectively
  // update the mipmap pyramid.

  auto t1 = std::chrono::high_resolution_clock::now ();

  auto u0 = std::async (std::launch::deferred,
    [&] (void)
    {
      try
      {
	image img = load_bmp_image (rgb_bmp_file);

	auto area = img.copy_to ({ src_x, src_y }, { src_width, src_height },
				 m_rgb_image[0], { x, y });

	update_mipmaps (m_rgb_image, area.dst_top_left, area.size);
      }
      catch (const std::exception& e)
      {
	std::cerr << "exception when loading image " << rgb_bmp_file << ": " << e.what () << std::endl;
      }
    });

  auto u1 = std::async (std::launch::async,
    [&] (void)
    {
      try
      {
	image img = load_bmp_image (height_bmp_file);

	auto area = img.copy_to ({ src_x, src_y },  { src_width, src_height },
				 m_height_image[0], { x, y });

	update_mipmaps (m_height_image, area.dst_top_left, area.size);
      }
      catch (const std::exception& e)
      {
	std::cerr << "exception when loading image " << height_bmp_file << ": " << e.what () << std::endl;
      }
    });

  u0.get ();
  u1.get ();

  auto t2 = std::chrono::high_resolution_clock::now ();

  std::cout << "tiled_image update "
	    << " t1-t0 = " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count ()
	    << " t2-t1 = " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count ()
	    << " t2-t0 = " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t0).count ()
	    << std::endl;
}

void
tiled_image::update_mipmaps (std::array<image, max_lod_level>& img,
			     const vec2<unsigned int>& top_level_xy,
			     const vec2<unsigned int>& top_level_size)
{
  auto xy = top_level_xy;
  auto size = top_level_size;

  for (unsigned int i = 0; i < img.size () - 1; ++i)
  {
    const image& src_level = img[i];
    image& dst_level = img[i + 1];

    // source and destination area coordinates for simple 2x2 averaging.
    auto dst_xy = xy / 2;
    auto dst_size = size / 2;

    // notice that this "rounds" the source coordinates
    auto src_xy = dst_xy * 2;
    auto src_size = dst_size * 2;

    // if the source region is outside the image, stop here.
    if (src_xy.x >= src_level.size ().x || src_xy.y >= src_level.size ().y
	|| src_size.x > src_level.size ().x || src_size.y > src_level.size ().y
	|| dst_size.x == 0 || dst_size.y == 0)
    {
      std::cout << "update_mipmaps terminating"
		<< " src_xy = (" << src_xy.x << "," << src_xy.y << ")"
		<< " src_size = (" << src_size.x << "," << src_size.y << ")"
		<< " src image size = (" << src_level.size ().x << "," << src_level.size ().y << ")"
		<< std::endl;
      break;
    }

    std::cout << "update_mipmaps"
	      << " src_xy = (" << src_xy.x << "," << src_xy.y << ")"
	      << " src_size = (" << src_size.x << "," << src_size.y << ")"
	      << " dst_xy = (" << dst_xy.x << "," << dst_xy.y << ")"
	      << " dst_size = (" << dst_size.x << "," << dst_size.y << ")"
	      << std::endl;

    src_level.subimg (vec2<int> (src_xy), src_size)
		.pyr_down_to (dst_level.subimg (vec2<int> (dst_xy), dst_size));

    xy = dst_xy;
    size = dst_size;
  }
}



// ---------------------------------------------------------------------------
const unsigned int INSIDE = 0; // 0000
const unsigned int LEFT = 1;   // 0001
const unsigned int RIGHT = 2;  // 0010
const unsigned int BOTTOM = 4; // 0100
const unsigned int TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

// ASSUME THAT xmax, xmin, ymax and ymin are global constants.

const double viewport_xmin = -1;
const double viewport_ymin = -1;
const double viewport_xmax = 1;
const double viewport_ymax = 1;

static unsigned int ComputeOutCode (const vec2<double>& p)
{
  unsigned code = INSIDE;

  if (p.x < viewport_xmin)
    code |= LEFT;
  else if (p.x > viewport_xmax)
    code |= RIGHT;
  if (p.y < viewport_ymin)
    code |= BOTTOM;
  else if (p.y > viewport_ymax)
    code |= TOP;

  return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (xmin, ymin) to (xmax, ymax).
static bool CohenSutherlandLineClipAndDraw (vec2<double> p0, vec2<double> p1)
{
  unsigned int outcode0 = ComputeOutCode (p0);
  unsigned int outcode1 = ComputeOutCode (p1);
  bool accept = false;

  while (true)
  {
    if (!(outcode0 | outcode1))
    {
      accept = true;
      break;
    }
    else if (outcode0 & outcode1)
      break;
    else
    {
      // failed both tests, so calculate the line segment to clip
      // from an outside point to an intersection with clip edge
      double x, y;

      // At least one endpoint is outside the clip rectangle; pick it.
      unsigned int outcodeOut = outcode0 ? outcode0 : outcode1;

      // Now find the intersection point;
      // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
      if (outcodeOut & TOP)
      {
        x = p0.x + (p1.x - p0.x) * (viewport_ymax - p0.y) / (p1.y - p0.y);
        y = viewport_ymax;
      }
      else if (outcodeOut & BOTTOM)
      {
        x = p0.x + (p1.x - p0.x) * (viewport_ymin - p0.y) / (p1.y - p0.y);
        y = viewport_ymin;
      }
      else if (outcodeOut & RIGHT)
      {
        y = p0.y + (p1.y - p0.y) * (viewport_xmax - p0.x) / (p1.x - p0.x);
        x = viewport_xmax;
      }
      else if (outcodeOut & LEFT)
      {
        y = p0.y + (p1.y - p0.y) * (viewport_xmin - p0.x) / (p1.x - p0.x);
        x = viewport_xmin;
      }

       // Now we move outside point to intersection point to clip
       // and get ready for next pass.
      if (outcodeOut == outcode0)
      {
        p0 = { x, y };
        outcode0 = ComputeOutCode(p0);
      }
      else
      {
        p1 = { x, y };
        outcode1 = ComputeOutCode(p1);
      }
    }
  }
/*
	if (accept) {
               // Following functions are left for implementation by user based on
               // their platform (OpenGL/graphics.h etc.)
               DrawRectangle(xmin, ymin, xmax, ymax);
               LineSegment(x0, y0, x1, y1);
	}
*/
  return accept;
}

// ---------------------------------------------------------------------------


struct tiled_image::tile_visibility
{
  bool visible;
  double image_area;
  double display_area; 
};

#if 0
static bool is_edge_visible (const vec2<double>& a, const vec2<double>& b)
{
  return CohenSutherlandLineClipAndDraw (a, b);
}

tiled_image::tile_visibility
tiled_image::calc_tile_visibility (const tile& t,
				   const mat4<double>& proj_cam_trv,
				   const mat4<double>& viewport_proj_cam_trv) const
{
  tile_visibility res;
  res.image_area = t.size ().x * t.size ().y;

  vec4<double> corners[] =
  {
    { t.pos ().x, t.pos ().y, 0, 1 },
    { t.pos ().x + t.size ().x, t.pos ().y, 0, 1 },
    { t.pos ().x + t.size ().x, t.pos ().y + t.size ().y, 0, 1 },
    { t.pos ().x, t.pos ().y + t.size ().y, 0, 1 }
  };

  for (auto& p : corners)
  {
//    auto ptrv = viewport_proj_cam_trv * p;
    auto ptrv = proj_cam_trv * p;
    p = { homogenize (ptrv).xyz (), ptrv.w };
  }

  std::cout << "calc tile visibility " << std::endl;

  int visible_count = 0;

  vec2<int> corner_vis[4];

  for (unsigned int i = 0; i < 4; ++i)
  {
    const auto& p = corners[i];
    auto& v = corner_vis[i];

    std::cout.precision (8);
    std::cout << "   (" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")";

    if (p.x < -1)
      v.x = -1;
    else if (p.x > 1)
      v.x = 1;
    else
      v.x = 0;

    if (p.y < -1)
      v.y = -1;
    else if (p.y > 1)
      v.y = 1;
    else
      v.y = 0;

    std::cout << "\t(" << v.x << ", " << v.y << ")";

    std::cout << std::endl;
    std::cout.precision (6);
  }

  bool e0 = is_edge_visible (corners[0].xy (), corners[1].xy ());
  bool e1 = is_edge_visible (corners[1].xy (), corners[2].xy ());
  bool e2 = is_edge_visible (corners[2].xy (), corners[3].xy ());
  bool e3 = is_edge_visible (corners[3].xy (), corners[0].xy ());

  std::cout << "e = " << e0 << " " << e1 << " " << e2 << " " << e3 << std::endl;

  std::cout << std::endl;

  res.visible = true;
  res.display_area = 0;

  return res;
}
#endif


tiled_image::tile_visibility
tiled_image::calc_tile_visibility (const tile& t,
				   const mat4<double>& proj_cam_trv,
				   const mat4<double>& viewport_proj_cam_trv) const
{
  // treat the tile as a bounding box with height = 0...

  tile_visibility res;

  enum plane_bit
  {
    left = 1 << 0,
    right = 1 << 1,
    top = 1 << 2,
    bottom = 1 << 3,
    near = 1 << 4,
    far = 1 << 5,

    all_3d_planes = left | right | top | bottom | near | far,
    all_2d_planes = left | right | top | bottom
  };

  struct point
  {
    vec4<double> p;
    vec4<double> pc;
    uint8_t planes = 0;

    point (void) = default;
    point (const point&) = default;
    point (const vec4<double>& pp) : p (pp) { }

    bool is_top (void) const { return planes & top; }
    bool is_bottom (void) const { return planes & bottom; }
    bool is_left (void) const { return planes & left; }
    bool is_right (void) const { return planes & right; }
    bool is_near (void) const { return planes & near; }
    bool is_far (void) const { return planes & far; }

    // true: on the inside of the frustum
    // false: on the outside of the frustum
    bool plane_side (unsigned int p) const { return planes & (1 << p); }
  };

  point corners[] =
  {
    { vec4<double> (t.pos ().x, t.pos ().y, 0, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y, 0, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y + t.size ().y, 0, 1) },
    { vec4<double> (t.pos ().x, t.pos ().y + t.size ().y, 0, 1) }
  };

  for (auto& c : corners)
  {
    // transform point into clip space: pc = (xc, yc, zc, wc)
    // do not homogenize it.
    const auto& pc = c.pc = proj_cam_trv * c.p;

    c.planes |= -pc.w < pc.x ? right : 0;
    c.planes |=  pc.x < pc.w ? left : 0;
    c.planes |=  pc.y < pc.w ? top : 0;
    c.planes |= -pc.w < pc.y ? bottom : 0;
    c.planes |= -pc.w < pc.z ? near : 0;
    c.planes |=  pc.z < pc.w ? far : 0;
  }

// #define tile_visibility_log

#ifdef tile_visibility_log
  for (const auto& c : corners)
  {
    std::cout << "   (" << c.pc.x << ", " << c.pc.y << ", " << c.pc.z << ", " << c.pc.w << ")\n"
	      << "      f: " << c.is_far () << "    " << c.is_top () << "      " << (int)c.planes << "\n"
	      << "            " << c.is_left () << "   " << c.is_right () << "\n"
	      << "      n: " << c.is_near () << "    " << c.is_bottom () << "\n";
  }
#endif

  enum
  {
    unknown = -1,
    invisible,
    visible,
  };

  int is_visible = unknown;

  if (is_visible == unknown)
  {
    // check if any point is completely inside the frustum.  if that's the
    // case the thing is definitely visible.
    for (const auto& c : corners)
      if (c.planes == all_3d_planes)
      {
	is_visible = visible;
	break;
      }
  }

  if (is_visible == unknown)
  {
    // check if all points are on the outer side of a frustum plane.
    // a point is on the outer side of a plane if the plane bit is 0.
    unsigned int or_sum = 0;

    for (const auto& c : corners)
      or_sum |= c.planes;

    // if all points are on the other side of a plane that bit will be 0 in
    // the OR sum.
    for (unsigned int i = 0; i < 6; ++i)
      if ((or_sum & (1 << i)) == 0)
      {
	is_visible = invisible;
	break;
      }
  }

  if (is_visible == unknown)
  {
    // all points are outside, but not on the same side of a single plane.
    // the frustum box might be completely inside of the tested box.
    // check this condition only for the left,right,top,bottom planes.
    // count the number of points of each plane that are on the outer side
    // of that plane.  if each plane has 1 point, there are no intersections
    // and the frustum is completely enclosed.
    unsigned int pout[6] = { 0, 0, 0, 0, 0, 0 };

    for (const auto& c : corners)
      for (unsigned int i = 0; i < 6; ++i)
        if (c.plane_side (i) == false)
	  pout[i] += 1;

#ifdef tile_visibility_log
    std::cout << "pout = ";
    for (unsigned int i = 0; i < 6; ++i)
      std::cout << pout[i] << " ";
    std::cout << std::endl;
#endif
  }

  if (is_visible == unknown)
  {
    // all points are outside, but not on the same side of a single plane.
    // if the edges of the tested box intersect the frustum box, it's visible.
    // if there is no intersection, the frustum box might be completely
    // inside of the tested box.

  }


#ifdef tile_visibility_log
  std::cout << "   is_visible = " << is_visible;
  std::cout << std::endl;
#endif

  res.visible = !(is_visible == invisible);
  res.image_area = 0;
  res.display_area = 0;

  return res;
}



void tiled_image::render (const mat4<double>& cam_trv, const mat4<double>& proj_trv,
			  const mat4<double>& viewport_trv)
{
  m_shader->activate ();
  m_shader->color_texture = 0;
  m_shader->height_texture = 0;

  glDisable (GL_TEXTURE_2D);
  glDisable (GL_DEPTH_TEST);

  static const std::array<vec4<float>, max_lod_level> lod_colors =
  {
    vec4<float> (1, 1, 1, 1),
    vec4<float> (1, 0, 0, 0),
    vec4<float> (0, 1, 0, 0),
    vec4<float> (0, 0, 1, 0),
    vec4<float> (1, 0, 1, 1),
//    vec4<float> (1, 1, 0, 1),
  };

/*
- visibility candidate list
    contains tiles that could be potentially visible.

- visibility list
    entries from the candidate list are moved to the visibility list
    after making sure that a candidate is really visible.

- for each candidate tile
   - each element either goes into the visibility list or not
     in any case it is removed from the candidate list

   - calculate tile area / screen area ratio (mipmap D) and visibility
   - if tile is visible and mipmap D < 1 (or > 1 ...)
      - add all its sub-tiles (higher detail level) to the candidate list
   - if tile is visible and mipmap D = 1
      - add this tile to the visibility list.
*/

  const auto proj_cam_trv = proj_trv * cam_trv;
  const auto viewport_proj_cam_trv = viewport_trv * proj_cam_trv;

  m_candidate_tiles.clear ();
  m_visible_tiles.clear ();

  // initially add all lowest level tiles as candidates.
  for (auto&& t : m_tiles.back ())
  {
    m_candidate_tiles.push_back (&t);
  }

#ifdef tile_visibility_log
std::cout << std::endl << std::endl;
#endif

  while (!m_candidate_tiles.empty ())
  {
    tile* t = m_candidate_tiles.back ();
    m_candidate_tiles.pop_back ();

    auto tv = calc_tile_visibility (*t, proj_cam_trv, viewport_proj_cam_trv);
    if (tv.visible)
    {
      m_visible_tiles.push_back (t);
    }
  }

  std::cout << "visible tiles: " << m_visible_tiles.size () << std::endl;

  // render tiles from lowest detail level to highest detail level.
  // notice that lower detail level = higher lod number.
  m_shader->zbias = 0.00001f;

  std::sort (m_visible_tiles.begin (), m_visible_tiles.end (),
	     [] (const tile* a, const tile* b)
	     {
	       return b->lod () < a->lod ();
	     });

  for (const tile* t : m_visible_tiles)
  {
    m_shader->mvp = (mat4<float>)(proj_cam_trv * t->trv ());
    m_shader->pos = gl::vertex_attrib (t->mesh ().vertex_buffer (), &vertex::pos);
    m_shader->offset_color = lod_colors[t->lod ()];

    glLineWidth (0.5f * t->lod ());
    t->mesh ().render_outline ();

    glLineWidth (0.025f * t->lod ());
    t->mesh ().render_wireframe ();
  }

#if 0
  for (unsigned int i = 0; i < max_lod_level; ++i)
//  for (int i = max_lod_level - 1; i >= 0; --i)
  {
    if (lod_colors[i].a == 0)
      continue;

    m_shader->zbias = 0.00001f * (i + 1) * 0;
    m_shader->offset_color = lod_colors[i];

    for (const auto& t : m_tiles[i])
    {
      auto mvp = proj_cam_trv * t.trv ();

      m_shader->mvp = (mat4<float>)mvp;

      m_shader->pos = gl::vertex_attrib (t.mesh ().vertex_buffer (), &vertex::pos);

      if (i == max_lod_level - 1 && 0)
      {
	glLineWidth (0.5f);
	t.mesh ().render_wireframe ();
      }

      {
	glLineWidth (1.5f);
	t.mesh ().render_outline ();
      }
    }
  }
#endif

#if 0
  for (int i = max_lod_level - 1; i >= 3; --i)
  {
    m_shader->zbias = 0.00001f * (i + 1) * 0;

    for (const auto& t : m_tiles[i])
    {
      glLineWidth (1.5f * i);
      m_shader->mvp = (mat4<float>)(proj_cam_trv * t.trv ());
      m_shader->pos = gl::vertex_attrib (t.mesh ().vertex_buffer (), &vertex::pos);
      m_shader->offset_color = lod_colors[t.lod ()];

      t.mesh ().render_outline ();

      for (const auto& tt : t.subtiles ())
      {
	if (tt == nullptr)
	  continue;

        glLineWidth (1.5f * i / 2);
        m_shader->mvp = (mat4<float>)(proj_cam_trv * tt->trv ());
        m_shader->pos = gl::vertex_attrib (tt->mesh ().vertex_buffer (), &vertex::pos);
        m_shader->offset_color = lod_colors[tt->lod ()];

        tt->mesh ().render_outline ();
      }
    }
  }
#endif
}

