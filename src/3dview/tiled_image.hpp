
#ifndef includeguard_tiled_image_hpp_includeguard
#define includeguard_tiled_image_hpp_includeguard

#include <memory>
#include <array>
#include <vector>

#include "gl/gl.hpp"
#include "utils/vec_mat.hpp"
#include "image.hpp"


class tiled_image
{
public:
  static constexpr unsigned int max_lod_level = 5;
  static constexpr unsigned int tile_grid_size = 128;
  static constexpr unsigned int max_lod_scale_factor = 1 << max_lod_level;
  static constexpr unsigned int texture_tile_size = max_lod_scale_factor * tile_grid_size;

  // larger textures are not safe it seems.
  static_assert (texture_tile_size <= 4096, "");

  tiled_image (void);
  tiled_image (const vec2<uint32_t>& size);

  tiled_image (const tiled_image&) = delete;
  tiled_image (tiled_image&&);

  tiled_image& operator = (const tiled_image&) = delete;
  tiled_image& operator = (tiled_image&&);

  ~tiled_image (void);

  const vec2<uint32_t>& size (void) const { return m_size; }
  bool empty (void) const { return m_size.x == 0 || m_size.y == 0; }

  void fill (int32_t x, int32_t y, uint32_t width, uint32_t height,
	     unsigned int r, unsigned int g, unsigned int b,
	     unsigned int z);

  void fill (unsigned int r, unsigned int g, unsigned int b,
	     unsigned int z)
  {
    fill (0, 0, size ().x, size ().y, r, g, b, z);
  }

  void update (int32_t x, int32_t y, uint32_t width, uint32_t height,
	       const char* rgb_bmp_file,
	       const char* height_bmp_file,
	       unsigned int src_x, unsigned int src_y,
	       unsigned int src_width, unsigned int src_height);

  void update (uint32_t x, uint32_t y, uint32_t width, uint32_t height,
	       const void* rgb_data, uint32_t rgb_data_stride_bytes,
	       const void* height_data, uint32_t height_data_stride_bytes);

private:
  struct vertex;
  struct shader;
  class grid_mesh;
  class tile;

  // shader and geomety is shared amongst image instances.
  static std::vector<std::shared_ptr<grid_mesh>> g_grid_meshes;
  static std::shared_ptr<shader> g_shader;

  // size of the whole image.
  vec2<uint32_t> m_size;

  // for simplicity, keep the whole image mipmaps in memory.
  // one FOV image is 2048x2048 @ 32 bpp = 16 MByte, 20 FOVs = 320 MByte.
  std::array<image, max_lod_level> m_rgb_image;
  std::array<image, max_lod_level> m_height_image;

  // a reference to the shared shader.
  std::shared_ptr<shader> m_shader;

  // all tiles in the image.
  //std::vector<std::unique_ptr<tile>> m_tiles;
  std::vector<tile> m_tiles;


  void generate_mipmaps (void);
};

#endif // includeguard_tiled_image_hpp_includeguard
