cmake_minimum_required(VERSION 3.0)

project (gl)

if (WIN32)

add_library (gl STATIC
  gl.cpp
  display_win32.cpp
  gldev_win32.cpp
  pixel_format.cpp
)

else ()

add_library (gl STATIC
  gl.cpp
  display_x11.cpp
  gldev_glx.cpp
  pixel_format.cpp
)

endif ()
