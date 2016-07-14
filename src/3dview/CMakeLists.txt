cmake_minimum_required(VERSION 3.0)

project (3dview)

set (CMAKE_CXX_STANDARD 14)

include_directories (
  ${CMAKE_SOURCE_DIR}/..
)

if (WIN32)

add_definitions (
-DUSE_GL
-DWIN32
-DNOMINMAX
-D_CRT_SECURE_NO_WARNINGS
)

else ()

add_definitions (
-DLINUX
-DUSE_GL
-DUSE_X11
)

endif ()

if (MSVC)

else ()
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++ -s")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=corei7 -mtune=intel")

# haswell / broadwell
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=core-avx2 -mtune=intel")

# ivy bridge
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=core-avx-i -mtune=intel")

endif ()

add_subdirectory (${CMAKE_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")
add_subdirectory (${CMAKE_SOURCE_DIR}/../gl "${CMAKE_CURRENT_BINARY_DIR}/gl")
add_subdirectory (${CMAKE_SOURCE_DIR}/../img "${CMAKE_CURRENT_BINARY_DIR}/img")
add_subdirectory (${CMAKE_SOURCE_DIR}/../s_expr "${CMAKE_CURRENT_BINARY_DIR}/s_expr")

if (MSVC)
else ()
SET_SOURCE_FILES_PROPERTIES (image.cpp PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
endif ()

add_executable (3dview
  main.cpp
  test_scene.cpp
  test_scene1.cpp
  tiled_image.cpp
  simple_3dbox.cpp
)

if (WIN32)

target_link_libraries (3dview
  utils
  kernel32
  gdi32
  gl
  img
  s_expr
  opengl32
)

if (MSVC)
else ()
target_link_libraries (3dview
  pthread
)
endif ()

add_definitions (
-DJUTZE3D_EXPORTS
)

# remove the "lib" prefix from the final dll file
set (CMAKE_SHARED_LIBRARY_PREFIX "")

add_library (jutze3d_dll SHARED
  jutze3d.cpp
  test_scene1.cpp
  tiled_image.cpp
  simple_3dbox.cpp
)

set_target_properties (jutze3d_dll PROPERTIES
  OUTPUT_NAME "jutze3d"
  IMPORT_PREFIX ""
)

target_link_libraries (jutze3d_dll
  utils
  kernel32
  gdi32
  gl
  img
  s_expr
  opengl32
)

if (MSVC)
else ()
target_link_libraries (jutze3d_dll
  pthread
)
endif ()


else ()

target_link_libraries (3dview
  utils
  gl
  img
  s_expr
  pthread
  X11
  GL
)

endif ()
