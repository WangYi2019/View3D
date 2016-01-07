cmake_minimum_required(VERSION 3.0)

project (3dview)

include_directories (
  ${CMAKE_SOURCE_DIR}/..
)

if (WIN32)

add_definitions (
-DUSE_GL
)

else ()

add_definitions (
-DLINUX
-DUSE_GL
-DUSE_X11
)

endif ()

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=corei7 -mtune=intel")

# haswell / broadwell
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=core-avx2 -mtune=intel")

add_subdirectory (${CMAKE_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")
add_subdirectory (${CMAKE_SOURCE_DIR}/../gl "${CMAKE_CURRENT_BINARY_DIR}/gl")

SET_SOURCE_FILES_PROPERTIES (image.cpp PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")

add_executable (3dview
  main.cpp
  test_scene.cpp
  test_scene1.cpp
  bmp_loader.cpp
  image.cpp
  tiled_image.cpp
)

if (WIN32)

target_link_libraries (3dview
  utils
  pthread
  kernel32
  gdi32
  gl
  opengl32
)

else ()

target_link_libraries (3dview
  utils
  gl
  pthread
  X11
  GL
)

endif ()
