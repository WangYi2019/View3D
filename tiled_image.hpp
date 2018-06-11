
#ifndef includeguard_tiled_image_hpp_includeguard
#define includeguard_tiled_image_hpp_includeguard

#include <memory>
#include <array>
#include <vector>

#include "gl/gl.hpp"
#include "utils/vec_mat.hpp"
#include "utils/lru_cache.hpp"
#include "img/image.hpp"

class tiled_image
{
public:
  // how many LOD levels we have
  static constexpr unsigned int max_lod_level = 6;
  static constexpr unsigned int max_lod_scale_factor = 1 << max_lod_level;

  // to avoid creases textures have to have some sampling border.
  static constexpr unsigned int texture_border = 8;

  // the grid size of one geometry tile.  the grid size is constant for each
  // detail level, but the tile is rendered bigger.
  // the grid size defines the subdivision of the top detail level.  bigger
  // grid size means fewer top-level tiles and more geometry per frame.
  static constexpr unsigned int tile_grid_size = 128 - texture_border*2;

  // the size of one texture tile (color or height texture).
  // texture tiles are independent of the grid size.  e.g. at lower detail
  // levels one texture tile might cover the whole image, while at the highest
  // level, multiple texture tiles might be needed to cover the whole image.
  // it's OK to have multiple geometry tiles per texture tile.  it's not OK
  // to have multiple texture tiles per geometry tile.

  // if the mipmapping of the gpu is used, the lowest mipmap level geometry
  // tiles must not cross a texture border.
  // static constexpr unsigned int texture_tile_size = max_lod_scale_factor * tile_grid_size;

  // if texture tiles are managed and selected manually for each mipmap level
  // and automatic gpu mipmapping is not used, we have more freedom on the
  // texture tile size.
  static constexpr unsigned int texture_tile_size = 128 - texture_border*2;

  // for now texture tiles are the same as geometry tiles.
  static_assert (texture_tile_size == tile_grid_size, "");

  static_assert (texture_tile_size >= tile_grid_size, "");

  // textures > 4096 can be problematic it seems.
  static_assert (texture_tile_size <= 4096, "");

  tiled_image (bool use_uint16_heightmap = false);
  tiled_image (const utils::vec2<uint32_t>& size, bool use_uint16_heightmap = false);

  tiled_image (const tiled_image&) = delete;
  tiled_image (tiled_image&&);

  tiled_image& operator = (const tiled_image&) = delete;
  tiled_image& operator = (tiled_image&&);

  ~tiled_image (void);

  const utils::vec2<uint32_t>& size (void) const { return m_size; }
  bool empty (void) const { return m_size.x == 0 || m_size.y == 0; }

  void fill (int32_t x, int32_t y, uint32_t width, uint32_t height,
	     float r, float g, float b, float z);

  void fill (float r, float g, float b, float z)
  {
    fill (0, 0, size ().x, size ().y, r, g, b, z);
  }

  void update (int32_t x, int32_t y,
	       const char* rgb_bmp_file,
	       const char* height_bmp_file,
	       unsigned int src_x, unsigned int src_y,
	       unsigned int src_width, unsigned int src_height);

  void update (int32_t x, int32_t y,
	       const char* rgb_bmp_file,
	       const char* height_bmp_file);

  void update (uint32_t x, uint32_t y, uint32_t width, uint32_t height,
	       const void* rgb_data, uint32_t rgb_data_stride_bytes,
	       img::pixel_format rgb_format,
	       const void* height_data, uint32_t height_data_stride_bytes,
	       img::pixel_format height_format);

  void render (const utils::mat4<double>& cam_trv, const utils::mat4<double>& proj_trv,
	       const utils::mat4<double>& viewport_trv,
	       bool render_wireframe,
	       bool stairs_mode,
	       bool debug_dist) const;

private:
  struct vertex;
  struct shader;
  class grid_mesh;
  class tile;
  struct tile_visibility;
  struct texture_key;

  class cpu_image;

  // since the gpu texture format might be different from the cpu image
  // store that, as part of the image.
  class cpu_image : public img::image
  {
  public:
    cpu_image (void) : img::image (), m_texture_format (img::pixel_format::invalid) { }

    cpu_image (img::pixel_format pf, const utils::vec2<unsigned int>& size,
	       img::pixel_format texture_format)
    : img::image (pf, size), m_texture_format (texture_format)
    {
    }

    auto texture_format (void) const { return m_texture_format; }

  private:
    img::pixel_format m_texture_format;
  };

  struct load_texture_tile
  {
    std::reference_wrapper<std::array<cpu_image, max_lod_level>> m_img;

    load_texture_tile (std::array<cpu_image, max_lod_level>& img) : m_img (img) { }
    load_texture_tile (void) = delete;
    load_texture_tile (const load_texture_tile&) = default;
    load_texture_tile (load_texture_tile&&) = default;
    load_texture_tile& operator = (const load_texture_tile&) = default;
    load_texture_tile& operator = (load_texture_tile&&) = default;

    // replace the specified entry for the specified key.
    void operator () (const texture_key& key, gl::texture& entry);
  };

  // shader and geomety is shared amongst image instances.
  static std::vector<std::shared_ptr<grid_mesh>> g_grid_meshes;
  static std::shared_ptr<shader> g_shader;

  // size of the whole image.
  utils::vec2<uint32_t> m_size;

  // z scale of the heightmap texture.  depends on the texel format used.
  // e.g. r8 = 1/256, r16 = 1/65536, r16ui = 1, r32f = 1
  // although integer textures are too restrictive and not useful.
  float m_texture_z_scale;

  // for simplicity, keep the whole image mipmaps in memory.
  // one FOV image is 2048x2048 @ 32 bpp = 16 MByte, 20 FOVs = 320 MByte.
  std::array<cpu_image, max_lod_level> m_rgb_image;
  std::array<cpu_image, max_lod_level> m_height_image;

  // a reference to the shared shader.
  std::shared_ptr<shader> m_shader;

  // all tiles in the image.
  std::array<std::vector<tile>, max_lod_level> m_tiles;

  // texture tile cache
  mutable utils::lru_cache<texture_key, gl::texture, load_texture_tile> m_rgb_texture_cache;
  mutable utils::lru_cache<texture_key, gl::texture, load_texture_tile> m_height_texture_cache;

  // candidate tiles for display.  modified during rendering.
  mutable std::vector<const tile*> m_candidate_tiles;

  // actually visible tiles for display.   modified during rendering.
  mutable std::vector<const tile*> m_visible_tiles;

  struct update_region
  {
    utils::vec2<unsigned int> tl;
    utils::vec2<unsigned int> br;
  };

  static std::array<update_region, max_lod_level>
  update_mipmaps (std::array<cpu_image, max_lod_level>& img,
		  const utils::vec2<unsigned int>& top_level_xy,
		  const utils::vec2<unsigned int>& top_level_size);

  static void
  invalidate_texture_cache (utils::lru_cache<texture_key, gl::texture, load_texture_tile>& cache,
			    const std::array<update_region, max_lod_level>& regions);

  tile_visibility
  calc_tile_visibility (const tile& t,
			const utils::mat4<double>& proj_cam_trv,
			const utils::mat4<double>& viewport_trv,
			float zscale) const;
};

#endif // includeguard_tiled_image_hpp_includeguard
