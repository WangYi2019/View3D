#ifndef includeguard_jutze3d_hpp_includeguard
#define includeguard_jutze3d_hpp_includeguard

#ifdef JUTZE3D_EXPORTS
  #ifdef _MSC_VER
    #define JUTZE3D_API __declspec(dllexport)
  #else
    #define JUTZE3D_API __stdcall __declspec(dllexport)
  #endif
#else
  #ifdef _MSC_VER
    #define JUTZE3D_API __declspec(dllimport)
  #else
    #define JUTZE3D_API __stdcall __declspec(dllimport)
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------

JUTZE3D_API void view3d_init (void);
JUTZE3D_API void view3d_finish (void);

// if 'val' is non-zero, the 3dviews that will be created
// will use the uint16 image/texture format for the internal heightmap
// representation.
// the default is to use float32 as the internal heightmap representation.
// this function can be called at any time.  the next image that will be
// created/resized will use the new setting.
JUTZE3D_API void view3d_use_uin16_heightmap (int val);

// --------------------------------------------------------------------------
// create a new 3D view window
// use standard win32 functions to
//  - show
//  - hide
//  - resize
//  - change position
//  - close/destroy
//
// ... the 3D view window.
// note: when window is resized need to re-create the framebuffers ....
//
// there can be only max. 1 window.  if a window already exists, it will
// be re-used.

// this function returns a HWND but we don't include windows.h in this file,
// so use void* instead.
//HWND JUTZE3D_API
JUTZE3D_API void*
view3d_new_window (unsigned int desktop_pos_x, unsigned int desktop_pos_y,
		   unsigned int width, unsigned int height, const char* title);

// center the current image at the specified image coordinate.
// x rotate and y rotate are angles in degree.
// to get top-down 2D view set x rotate = y rotate = 0.
JUTZE3D_API void
view3d_center_image (unsigned int x, unsigned int y,
		     double x_rotate, double y_rotate);

// enable rendering of the view.
JUTZE3D_API void
view3d_enable_render (void);

// disable rendering of the view.
JUTZE3D_API void
view3d_disable_render (void);

// set the z value scale coefficient for the z image.  the values from the
// z image are multiplied with this coefficient before display.  the default
// value is 1.0.
JUTZE3D_API void
view3d_set_z_scale (float val);

//set val1 is the tile_angle,and set val2 is rotate_trv
JUTZE3D_API void
view3d_set_angle(float val1, float val2);

// --------------------------------------------------------------------------

// resize the current image.  initially the image is empty (width = height = 0).
// when the image size is changed, the image is cleared and old image data
// is discarded.  the pixel values (rgb and z) are initialized to 0.
JUTZE3D_API void
view3d_resize_image (unsigned int width_pixels, unsigned int height_pixels);

// fill the whole image with the specified constant value.
// the value range of the fill values is clamped to [0..1].
JUTZE3D_API void
view3d_fill_image (float r, float g, float b, float z);

// fill the specified image region with the specified constant value.
JUTZE3D_API void
view3d_fill_image_area (unsigned int x, unsigned int y,
			unsigned int width, unsigned int height,
			float r, float g, float b, float z);

// update the image area at the specified region.  because the specified
// region in the image is overwritten, there is no problem with overlapping
// region updates.  update functions can be used while the image view is
// being displayed.

// this overload loads the image data from the files.
// after loading the file, it is cropped to the specified sub-region.
// this can be used to display a small area around a single part, if the
// FOV image of the part is known.
// to use the full image (no crop), set src_x = src_y = 0 and
// src_width = src_height = std::numeric_limits<unsigned int>::max ()
JUTZE3D_API void
view3d_update_image_area_1 (unsigned int x, unsigned int y,
			    unsigned int width, unsigned int height,
			    const char* rgb_bmp_file,
			    const char* height_bmp_file,
			    unsigned int src_x, unsigned int src_y,
			    unsigned int src_width, unsigned int src_height);

// this overload copies the image data from the specified memory.
// the RGB input format can have the following values:
//   0 - unsigned 8 bit per component, 24 bit per pixel, RGB order
//   1 - unsigned 8 bit per component, 24 bit per pixel, BGR order
//   2 - unsigned 8 bit per component, 32 bit per pixel, RGBA order
//
// the height format can have the following values:
//   0 - unsigned 8 bit
//   2 - unsigned 16 bit
//   3 - float 32 bit
JUTZE3D_API void
view3d_update_image_area_2 (unsigned int x, unsigned int y,
			    unsigned int width, unsigned int height,
			    const void* rgb_data,  unsigned int rgb_data_stride_bytes,
			    unsigned int rgb_format,
			    const void* height_data, unsigned int height_data_stride_bytes,
			    unsigned int height_format);

// set the heightmap color palette of the current image for heightmap
// visualization.
// if the image is resized or re-initialized the heightmap palette data
// will be reset to the internal default value.
typedef struct
{
  unsigned int height;
  float color_r;
  float color_g;
  float color_b;
  float color_a;
} view3d_heightmap_palette_entry;

JUTZE3D_API void
view3d_set_heightmap_palette (const view3d_heightmap_palette_entry* entries,
			      unsigned int entry_count);

// turn heightmap visualization rendering mode on/off.
// if 'value' is zero, heightmap rendering is off (normal color rendering)
// if 'value' is non-zero, heightmap rendering is on.
JUTZE3D_API void
view3d_set_heightmap_rendering (int value);

// adds a new 3D box to the board.
// board_pos_x, board_pos_y is the top-left corner of the box in board image
// coordinates.  board_pos_z is the bottom z coordinate of the box.
// returns the ID for the box object.  the object ID can be used to remove the
// object.
JUTZE3D_API unsigned int
view3d_add_box (unsigned int board_pos_x, unsigned int board_pos_y,
		unsigned int board_pos_z,
		unsigned int box_size_x, unsigned int box_size_y,
		unsigned int box_size_z,
		float fill_r, float fill_g, float fill_b, float fill_a,
		float edge_r, float edge_g, float edge_b, float edge_a);

JUTZE3D_API void
view3d_remove_box (unsigned int obj_id);

JUTZE3D_API void
view3d_remove_all_boxes (void);

#ifdef __cplusplus
}
#endif

#endif // includeguard_jutze3d_hpp_includeguard
