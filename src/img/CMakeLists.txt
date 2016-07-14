cmake_minimum_required(VERSION 3.0)

project (img)

add_library (img STATIC
  image.cpp
  bmp_loader.cpp
  pixel_format.cpp
)
