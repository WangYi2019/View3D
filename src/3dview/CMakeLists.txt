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

add_subdirectory (${CMAKE_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")
add_subdirectory (${CMAKE_SOURCE_DIR}/../gl "${CMAKE_CURRENT_BINARY_DIR}/gl")

add_executable (3dview
  main.cpp
  test_scene.cpp
  bmp_loader.cpp
  image.cpp
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
