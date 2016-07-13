# ----------------------------------------------------------------------------
# http://www.vtk.org/Wiki/images/c/c2/Toolchain-cross-mingw32-linux.cmake

include(ProcessorCount)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Linux)

set(COMPILER_PREFIX "")

#find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
#find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH  /usr/${COMPILER_PREFIX} ${USER_ROOT_PATH})

set (CMAKE_C_COMPILER gcc CACHE FILEPATH "target_c" FORCE)
set (CMAKE_CXX_COMPILER g++ CACHE FILEPATH "target_cxx" FORCE)

# have to override ar,nm and ranlib to use the gcc wrapper for LTO support.
# force it into the cmake cache or else it will try lookup them up later and
# get it wrong.
set (CMAKE_AR gcc-ar CACHE FILEPATH "target_ar" FORCE)
set (CMAKE_NM gcc-nm CACHE FILEPATH "target_nm" FORCE)
set (CMAKE_RANLIB gcc-ranlib CACHE FILEPATH "target_ranlib" FORCE)


# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# ----------------------------------------------------------------------------

# the directory of the toolchain file is used to lookup other toolchain
# related files like linker script.

if (NOT CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")

# don't add anything funny if the toolchain file is not set.
# this happens when cmake runs the initial "is compiler compiler working"
# checks.

else ()

# mark some of the variables introduced here as "internal".
# could also use "mark_as_advanced" ...
macro (mark_as_internal _var)
  set ( ${_var} ${${_var}} CACHE INTERNAL "" FORCE )
endmacro ()

get_filename_component (TOOLCHAIN_DIR ${CMAKE_TOOLCHAIN_FILE} DIRECTORY)
get_filename_component (TOOLCHAIN_DIR ${TOOLCHAIN_DIR} ABSOLUTE CACHE)
#mark_as_internal (TOOLCHAIN_DIR)

if (NOT CMAKE_LIBRARIES_DIR OR CMAKE_LIBRARIES_DIR STREQUAL "")
else ()
  get_filename_component (LIBRARIES_DIR ${CMAKE_LIBRARIES_DIR} ABSOLUTE CACHE)
#  mark_as_internal (BOARD_DIR)
endif ()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/defaults.cmake)
  include (${CMAKE_CURRENT_SOURCE_DIR}/defaults.cmake)
else ()
  message ("does not exist " ${CMAKE_CURRENT_SOURCE_DIR}/defaults.cmake)
endif ()

# if LTO is used, we can't use "add_compile_options" because these will
# not be added to the final linker options, which is needed for the whole
# thing to work.
# https://cmake.org/pipermail/cmake-developers/2014-June/010623.html

option (TARGET_LTO "Link time optimization (LTO)" ON)
option (TARGET_FWEB "-fweb" OFF)
option (TARGET_FPEEL_LOOPS "-fpeel-loops" OFF)
option (TARGET_GC_SECTIONS "Linker GC sections" ON)
option (TARGET_DATA_SECTIONS "Separate data sections" ON)
option (TARGET_FUNCTION_SECTIONS "Separate function sections" OFF)
option (TARGET_SAVE_TEMPS "Save intermediate compiler output files" OFF)
option (TARGET_VERBOSE_LINKER "Verbose linking" OFF)
option (TARGET_CXX_EXCEPTIONS "C++ Exceptions" ON)
option (TARGET_C_EXCEPTIONS "C Exceptions" OFF)
option (TARGET_CXX_RTTI "C++ RTTI" ON)
#option (TARGET_DISABLE_GLOBAL_DTORS "Disable global destructor code" ON)
option (TARGET_ENABLE_ALL_WARNINGS "Enable all warnings" ON)
option (TARGET_FULL_STATIC "Full static linking" ON)
set (TARGET_OPTIMIZE "-O2" CACHE STRING "Optimization flags")
option (TARGET_STRIP_SYMBOLS "Strip symbols" OFF)
option (TARGET_DEBUG_INFO "Incude Debug Info" OFF)

set (BUILD_PROCESSOR_COUNT "" CACHE STRING "Build Processor Count")

if (NOT BUILD_PROCESSOR_COUNT OR BUILD_PROCESSOR_COUNT STREQUAL "")
  ProcessorCount (BUILD_PROCESSOR_COUNT)
  set (BUILD_PROCESSOR_COUNT "${BUILD_PROCESSOR_COUNT}" CACHE STRING "Build Processor Count" FORCE)
  message ("build processor count set to ${BUILD_PROCESSOR_COUNT}")
endif ()

# if LTO is used it's better to disable function-sections as it
# results in smaller code.

if (TARGET_ENABLE_ALL_WARNINGS)
  set (OPT_WALL "-Wall")
endif ()

if (TARGET_LTO)
  set (OPT_LTO "-flto=${BUILD_PROCESSOR_COUNT} -flto-compression-level=0")
endif ()

if (TARGET_FWEB)
  set (OPT_FWEB "-fweb")
endif ()

if (TARGET_FPEEL_LOOPS)
  set (OPT_FPEEL_LOOPS "-fpeel-loops")
endif ()

if (TARGET_GC_SECTIONS)
  set (OPT_GC_SECTIONS "-Wl,--gc-sections")
endif ()

if (TARGET_DATA_SECTIONS)
  set (OPT_DATA_SECTIONS "-fdata-sections")
endif ()

if (TARGET_FUNCTION_SECTIONS)
  set (OPT_FUNCTION_SECTIONS "-ffunction-sections")
endif ()


if (TARGET_SAVE_TEMPS)
  set (OPT_SAVE_TEMPS "-save-temps")
else ()
  set (OPT_SAVE_TEMPS "-pipe")
endif ()

if (TARGET_VERBOSE_LINKER)
  set (LINKER_VERBOSE "-Wl,-verbose")
endif ()

if (TARGET_CXX_EXCEPTIONS)
  set (OPT_CXX_EXCEPTIONS "-fexceptions")
else ()
  set (OPT_CXX_EXCEPTIONS "-fno-exceptions")
endif ()

if (TARGET_C_EXCEPTIONS)
  set (OPT_C_EXCEPTIONS "-fexceptions")
else ()
  set (OPT_C_EXCEPTIONS "-fno-exceptions")
endif ()

if (TARGET_CXX_RTTI)
  set (OPT_CXX_NORTTI "")
else ()
  set (OPT_CXX_NORTTI "-fno-rtti")
endif ()

# default options

set (TARGET_OPTIONS "")
set (C_LANG_OPTIONS "-std=gnu99 -Werror=return-type ${OPT_WALL} -lpthread -pthread")
set (CXX_LANG_OPTIONS "-std=c++14 -Werror=return-type ${OPT_WALL} -lpthread -pthread")

if (TARGET_FULL_STATIC)
  set (LINKER_STATIC "-Wl,-Bstatic")
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -static")
endif ()

if (TARGET_STRIP_SYMBOLS)
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -s")
endif ()

if (TARGET_DEBUG_INFO)
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -g")
endif ()

set (CMAKE_C_FLAGS "\
${TARGET_OPTIMIZE} \
${OPT_C_EXCEPTIONS} \
${C_LANG_OPTIONS} \
${TARGET_OPTIONS} \
${ASM_OPTIONS} \
${OPT_FUNCTION_SECTIONS} \
${OPT_DATA_SECTIONS} \
${OPT_SAVE_TEMPS} \
${OPT_LTO} \
${OPT_FWEB} \
${OPT_FPEEL_LOOPS} \
${OPT_DISABLE_GLOBAL_DTORS} \
" CACHE STRING "target_c_flags" FORCE)

set (CMAKE_CXX_FLAGS "\
${TARGET_OPTIMIZE} \
${OPT_CXX_EXCEPTIONS} \
${OPT_CXX_NORTTI} \
${CXX_LANG_OPTIONS} \
${TARGET_OPTIONS} \
${ASM_OPTIONS} \
${OPT_FUNCTION_SECTIONS} \
${OPT_DATA_SECTIONS} \
${OPT_SAVE_TEMPS} \
${OPT_LTO} \
${OPT_FWEB} \
${OPT_FPEEL_LOOPS} \
${OPT_DISABLE_GLOBAL_DTORS} \
" CACHE STRING "target_cxx_flags" FORCE)

# note: cmake uses gcc as the assembler for .S files.
set (CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "target_asm_flags" FORCE)

set (CMAKE_EXE_LINKER_FLAGS "\
${OPT_GC_SECTIONS} ${ASM_OPTIONS} ${LINKER_VERBOSE} ${LINKER_STATIC}"
CACHE STRING "target_ld_flags" FORCE)

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_CXX_FLAGS}"
CACHE STRING "target_ld_flags_shared" FORCE)


# ----------------------------------------------------------------------------
macro (import_library _name)

if ((${_name} STREQUAL "board") AND (BOARD_DIR))

  add_subdirectory (${BOARD_DIR} "${CMAKE_CURRENT_BINARY_DIR}/board")

else ()

  if (LIBRARIES_DIR)

    add_subdirectory (${LIBRARIES_DIR}/${_name} "${CMAKE_CURRENT_BINARY_DIR}/${_name}")

    if (EXISTS ${LIBRARIES_DIR}/${_name}/${_name}.cmake)
      include (${LIBRARIES_DIR}/${_name}/${_name}.cmake)
    endif ()

    include_directories (${LIBRARIES_DIR}/${_name}/../)

  else ()

    message (FATAL_ERROR "LIBRARIES_DIR not set, can't use import_library")

  endif ()

endif ()

endmacro ()

# ----------------------------------------------------------------------------
# unfortunately this doesn't work for the top-level project.
# for some reason the overriding macro doesn't get invoked.
# thus we need to do the "include_directories" for the top-level project
# explicitly.

macro (project name)

  if (INSIDE_PROJECT_MACRO_OVERRIDE)

  else ()
    set (INSIDE_PROJECT_MACRO_OVERRIDE TRUE)

#message ("PROJECT ${name}")
    _project (${name} ${ARGN})

    if (LIBRARIES_DIR)
#essage ("ADDING INC DIR ${LIBRARIES_DIR}")
      include_directories (${LIBRARIES_DIR})
    endif ()

    unset (INSIDE_PROJECT_MACRO_OVERRIDE)
  endif ()

endmacro ()

# ----------------------------------------------------------------------------

endif ()
