
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
};


// ----------------------------------------------------------------------------

struct tiled_image::shader : public gl::shader
{
  uniform< mat4<float>, highp > mvp;
  uniform< vec4<float>, lowp > offset_color;
  uniform< vec4<float>, lowp > color;

  uniform< sampler2D, mediump > color_texture;
  uniform< sampler2D, mediump > height_texture;

  uniform< float, highp> zbias;
  uniform< float, highp> zscale;

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
    named_parameter (zscale);
  }

  vertex_shader_text
  (
    varying vec2 color_uv;

    void main (void)
    {
      vec4 height = texture2D (height_texture, pos);

      color_uv = pos;
      gl_Position = mvp * vec4 (pos, height.r * zscale + zbias, 1.0);
    }
  )

  fragment_shader_text
  (
    varying vec2 color_uv;

    void main (void)
    {
      gl_FragColor = texture2D (color_texture, color_uv) * color + offset_color;
    }
  )
};

std::shared_ptr<tiled_image::shader> tiled_image::g_shader;

// ----------------------------------------------------------------------------

struct tiled_image::texture_key
{
  unsigned int lod;
  vec2<unsigned int> img_pos;

  // assume that image coorindates are max. 29 bits (536870912 x 536870912 pixels)
  uint64_t packed;

  texture_key (void) = default;
  texture_key (const texture_key&) = default;
  texture_key (unsigned int l, const vec2<unsigned int>& p)
  : lod (l), img_pos (p)
  {
    packed = (l & 63)
	     | (((uint64_t)p.x & ((1u << 29)-1)) << 6)
	     | (((uint64_t)p.y & ((1u << 29)-1)) << (6+29));
  }

  bool operator < (const texture_key& rhs) const
  {
    return packed < rhs.packed;
  }


  friend std::ostream& operator << (std::ostream& out, const tiled_image::texture_key& k)
  {
    return out << k.lod << " " << k.img_pos.x << " " << k.img_pos.y
		<< " (" << std::hex << k.packed << std::dec << ")";
  }
};

void tiled_image::load_texture_tile::operator () (const texture_key& k, gl::texture& tex)
{
//  std::cout << "load_texture_tile "
//	    << k.lod << " " << k.img_pos.x << "," << k.img_pos.y << std::endl;

  auto&& img = m_img.get ()[k.lod];

  if (tex.empty () || tex.format () != img.format ())
  {
    tex = gl::texture (img.format (), { texture_tile_size });
    tex.set_address_mode_u (gl::texture::clamp);
    tex.set_address_mode_v (gl::texture::clamp);
    tex.set_min_filter (gl::texture::linear);
    tex.set_mag_filter (gl::texture::linear);
  }

  // the image position in the key is in the lod=0 coordinate system.
  // the actual position depends on the lod value.
  auto&& subimg = img.subimg (vec2<int> (k.img_pos >> k.lod), { texture_tile_size });

  tex.upload (subimg.data (), { 0 }, subimg.size (), subimg.bytes_per_line ());
}

// ----------------------------------------------------------------------------

tiled_image::tiled_image (void) : tiled_image (vec2<uint32_t> (0, 0)) { }

tiled_image::tiled_image (const vec2<uint32_t>& size)
: m_size (size),
  m_rgb_texture_cache (load_texture_tile (m_rgb_image), 1024),
  m_height_texture_cache (load_texture_tile (m_height_image), 1024)
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
  m_rgb_texture_cache (std::move (rhs.m_rgb_texture_cache)),
  m_height_texture_cache (std::move (rhs.m_height_texture_cache)),
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
    m_rgb_texture_cache = std::move (rhs.m_rgb_texture_cache);
    m_height_texture_cache = std::move (rhs.m_height_texture_cache);
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


struct tiled_image::tile_visibility
{
  bool visible;
  double image_area;
  double display_area; 
};


template <typename T> static inline T
triangle_area (const vec2<T>& p0, const vec2<T>& p1, const vec2<T>& p2)
{
  auto a = length (p1 - p0);
  auto b = length (p2 - p1);
  auto c = length (p2 - p0);

  return std::sqrt ( (a + b + c) * (b + c - a) * (c + a - b) * (a + b - c) ) / 4;
}


tiled_image::tile_visibility
tiled_image::calc_tile_visibility (const tile& t,
				   const mat4<double>& proj_cam_trv,
				   const mat4<double>& viewport_trv,
				   float zscale) const
{
  // treat the tile as a bounding box with height = 0...

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
    // original point
    vec4<double> p;

    // point in clip space (perspective transform, not homogenized)
    vec4<double> pc;

    // point in screen space (actual screen coordinates)
    vec4<double> ps;

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
    bool plane_side (plane_bit p) const { return planes & p; }
  };

  std::array<point, 8> corners =
  {{
    { vec4<double> (t.pos ().x, t.pos ().y, 0, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y, 0, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y + t.size ().y, 0, 1) },
    { vec4<double> (t.pos ().x, t.pos ().y + t.size ().y, 0, 1) },

    { vec4<double> (t.pos ().x, t.pos ().y, zscale, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y, zscale, 1) },
    { vec4<double> (t.pos ().x + t.size ().x, t.pos ().y + t.size ().y, zscale, 1) },
    { vec4<double> (t.pos ().x, t.pos ().y + t.size ().y, zscale, 1) }
  }};

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

    is_visible = visible;
  }

#ifdef tile_visibility_log
  std::cout << "   is_visible = " << is_visible << std::endl;
#endif

  tile_visibility res;

  if (is_visible == visible)
  {
    res.visible = true;

    // calculate the tile area
    // as a simplification, we use the longest edge for level-of-detail
    // selection.
#define use_max_edge_length

#ifdef use_max_edge_length
    auto phys_sz = t.physical_size ();
    res.image_area = std::max (phys_sz.x, phys_sz.y);
#else
    auto phys_sz = t.physical_size ();
    res.image_area = phys_sz.x * phys_sz.y;
#endif

    // if a corner of the tile is in front of the znear plane, we can't
    // do a simple projection and would have to do znear clipping to calculate
    // the actual area.  to avoid that, assume that it's very close to the
    // viewport (camera) and set it to something that would cause the selection
    // of a higher detail level.
    bool intersects_znear = false;
    for (const auto& c : corners)
      if (c.plane_side (near) == false)
      {
	intersects_znear = true;
	break;
      }

#ifdef tile_visibility_log
    std::cout << "   intersects_znear = " << intersects_znear << std::endl;
#endif

    if (intersects_znear)
      res.display_area = res.image_area * 32;
    else
    {
      // project points into screen space and find the longest edge.
      for (auto& c : corners)
	c.ps = viewport_trv * homogenize (c.pc);

#ifdef use_max_edge_length
      res.display_area = 0;

      // this uses only the bottom plane of the tile box.
      for (unsigned int i = 0; i < 4; ++i)
      {
	// this assumes that the corner points are in clockwise order.
	unsigned int ii = (i + 1) & 3;

	auto edge_len = length (corners[ii].ps.xy () - corners[i].ps.xy ());

	#ifdef tile_visibility_log
	std::cout << "edge (" << corners[i].ps.x << ", " << corners[i].ps.y << "), ("
		  << corners[ii].ps.x << ", " << corners[ii].ps.y << ")  len: "
		  << edge_len << std::endl;
	#endif
	res.display_area = std::max (res.display_area, edge_len);
      }
#else
      res.display_area =
	triangle_area (corners[0].ps.xy (), corners[1].ps.xy (), corners[2].ps.xy ())
	+ triangle_area (corners[0].ps.xy (), corners[2].ps.xy (), corners[3].ps.xy ());
#endif
    }
  }
  else
  {
    res.visible = false;
    res.image_area = 0;
    res.display_area = 0;
  }


#ifdef tile_visibility_log
  std::cout << std::endl;
#endif

  return res;
}



void tiled_image::render (const mat4<double>& cam_trv, const mat4<double>& proj_trv,
			  const mat4<double>& viewport_trv, bool render_wireframe,
			  bool debug_dist)
{
  const float zscale = 0.05f;

  m_shader->activate ();
  m_shader->color_texture = 0;
  m_shader->height_texture = 1;


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
//break;
  }

//#define per_frame_log

#if defined (tile_visibility_log) || defined (per_frame_log)

std::cout
<< "\n\n=========================================="
<< std::endl;

#endif

  while (!m_candidate_tiles.empty ())
  {
    tile* t = m_candidate_tiles.back ();
    m_candidate_tiles.pop_back ();

    auto tv = calc_tile_visibility (*t, proj_cam_trv, viewport_trv, zscale);
    if (tv.visible)
    {
      double lod_d = tv.display_area / tv.image_area;
/*
#ifdef per_frame_log
      std::cout << "visible tile image area = " << tv.image_area
		<< " disp area: " << tv.display_area
		<< " lod: " << t->lod ()
		<< " lod d: " << lod_d << std::endl;
#endif
*/

#ifdef use_max_edge_length
      const double d_threshold = 2;
#else
      const double d_threshold = 2;
#endif

      if (t->has_subtiles () && lod_d > d_threshold && t->lod () > 0)
      {
	for (tile* subtile : t->subtiles ())
	  if (subtile != nullptr)
	    m_candidate_tiles.push_back (subtile);
      }
      else
	m_visible_tiles.push_back (t);
    }
  }

#ifdef per_frame_log
  std::cout << "visible tiles: " << m_visible_tiles.size () << std::endl;
#endif

  // render tiles from lowest detail level to highest detail level.
  // notice that lower detail level = higher lod number.
  std::sort (m_visible_tiles.begin (), m_visible_tiles.end (),
	     [] (const tile* a, const tile* b)
	     {
	       return b->lod () < a->lod ();
	     });

  auto proj_cam_trv2 = proj_cam_trv;

  if (debug_dist)
    proj_cam_trv2 = proj_trv * mat4<double>::translate (0, 0, -1) * cam_trv;


  glEnable (GL_TEXTURE_2D);
  glEnable (GL_DEPTH_TEST);

  m_shader->offset_color = { 0 };
  m_shader->zbias = 0;
  m_shader->color = { 1 };

  m_shader->zscale = zscale;

  for (const tile* t : m_visible_tiles)
  {
    m_shader->mvp = (mat4<float>)(proj_cam_trv2 * t->trv ());
    m_shader->pos = gl::vertex_attrib (t->mesh ().vertex_buffer (), &vertex::pos);

    auto&& t0 = m_rgb_texture_cache.get ({ t->lod (), t->pos () });
    auto&& t1 = m_height_texture_cache.get ({ t->lod (), t->pos () });

    t0.bind (0);
    t1.bind (1);

    t->mesh ().render_textured ();
  }


  if (render_wireframe)
  {
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);

    m_shader->color = { 0 };

    for (const tile* t : m_visible_tiles)
    {
      m_shader->mvp = (mat4<float>)(proj_cam_trv2 * t->trv ());
      m_shader->pos = gl::vertex_attrib (t->mesh ().vertex_buffer (), &vertex::pos);
      m_shader->offset_color = lod_colors[t->lod ()];

      auto&& t0 = m_rgb_texture_cache.get ({ t->lod (), t->pos () });
      auto&& t1 = m_height_texture_cache.get ({ t->lod (), t->pos () });

      t0.bind (0);
      t1.bind (1);

      // glLineWidth (0.025f * t->lod () + 0.125f);
      // t->mesh ().render_wireframe ();

      glLineWidth (0.5f * t->lod () + 0.125f);
      t->mesh ().render_outline ();
    }
  }
}

