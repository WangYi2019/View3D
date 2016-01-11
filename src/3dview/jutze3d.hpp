#ifndef includeguard_jutze3d_hpp_includeguard
#define includeguard_jutze3d_hpp_includeguard

#ifdef JUTZE3D_EXPORTS
  #define JUTZE3D_API __stdcall __declspec(dllexport)
#else
  #define JUTZE3D_API __stdcall __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------

void JUTZE3D_API view3d_init (void);
void JUTZE3D_API view3d_finish (void);

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
void* JUTZE3D_API
view3d_new_window (unsigned int desktop_pos_x, unsigned int desktop_pos_y,
		   unsigned int width, unsigned int height, const char* title);

// center the current image at the specified image coordinate.
// x rotate and y rotate are angles in degree.
// to get top-down 2D view set x rotate = y rotate = 0.
void JUTZE3D_API
view3d_center_image (unsigned int x, unsigned int y,
		     double x_rotate, double y_rotate);

// enable rendering of the view.
void JUTZE3D_API
view3d_enable_render (void);

// disable rendering of the view.
void JUTZE3D_API
view3d_disable_render (void);


// --------------------------------------------------------------------------

// resize the current image.  initially the image is empty (width = height = 0).
// when the image size is changed, the image is cleared and old image data
// is discarded.  the pixel values (rgb and z) are initialized to 0.
void JUTZE3D_API
resize_image (unsigned int width_pixels, unsigned int height_pixels);

// fill the whole image with the specified constant value.
// the value range of the fill values is clamped to [0..1].
void JUTZE3D_API
fill_image (float r, float g, float b, float z);

// fill the specified image region with the specified constant value.
void JUTZE3D_API
fill_image_area (unsigned int x, unsigned int y,
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
void JUTZE3D_API
update_image_area_1 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const char* rgb_bmp_file,
		     const char* height_bmp_file,
		     unsigned int src_x, unsigned int src_y,
		     unsigned int src_width, unsigned int src_height);

// this overload copies the image data from the specified memory.
// the expected RGB data is unsigned 8 bit per component, 24 bit per pixel,
// ordered as R,G,B.
// the expected height data is unsigned 8 bit.
void JUTZE3D_API
update_image_area_2 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const void* rgb_data,  unsigned int rgb_data_stride_bytes,
		     const void* height_data, unsigned int height_data_stride_bytes);
 
#ifdef __cplusplus
}
#endif

#endif // includeguard_jutze3d_hpp_includeguard