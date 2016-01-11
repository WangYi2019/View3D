
#if defined (WIN32) && defined (JUTZE3D_EXPORTS)

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "mingw.thread.h"
#include <mutex>
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#include <atomic>

#include <chrono>
#include <limits>
#include <iostream>
#include <algorithm>

#include "tiled_image.hpp"
#include "test_scene1.hpp"

#include "jutze3d.hpp"


void JUTZE3D_API
view3d_init (void)
{

}

void JUTZE3D_API
view3d_finish (void)
{

}

void* JUTZE3D_API
view3d_new_window (unsigned int desktop_pos_x, unsigned int desktop_pos_y,
		   unsigned int width, unsigned int height, const char* title)
{
  return nullptr;
}


void JUTZE3D_API
view3d_center_image (unsigned int x, unsigned int y,
		     double x_rotate, double y_rotate)
{

}

void JUTZE3D_API
view3d_enable_render (void)
{

}

void JUTZE3D_API
view3d_disable_render (void)
{

}


void JUTZE3D_API
resize_image (unsigned int width_pixels, unsigned int height_pixels)
{

}

void JUTZE3D_API
fill_image (float r, float g, float b, float z)
{

}

void JUTZE3D_API
fill_image_area (unsigned int x, unsigned int y,
		 unsigned int width, unsigned int height,
		 float r, float g, float b, float z)
{
}

void JUTZE3D_API
update_image_area_1 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const char* rgb_bmp_file,
		     const char* height_bmp_file,
		     unsigned int src_x, unsigned int src_y,
		     unsigned int src_width, unsigned int src_height)
{
}


void JUTZE3D_API
update_image_area_2 (unsigned int x, unsigned int y,
		     unsigned int width, unsigned int height,
		     const void* rgb_data,  unsigned int rgb_data_stride_bytes,
		     const void* height_data, unsigned int height_data_stride_bytes)
{
}




#endif // #if defined (WIN32) && defined (JUTZE3D_EXPORTS)
