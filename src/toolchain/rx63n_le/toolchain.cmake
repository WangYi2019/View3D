# adding options
# http://stackoverflow.com/questions/6787371/how-do-i-specify-build-options-for-cmake-based-projects

# toolchain files
# http://www.vtk.org/Wiki/CMake_Cross_Compiling


set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_VERSION 1)
set (CMAKE_SYSTEM_PROCESSOR RX)
set (CMAKE_CROSSCOMPILING TRUE)

set (CMAKE_C_COMPILER /usr/local/bin/rx-elf-gcc CACHE FILEPATH "target_c" FORCE)
set (CMAKE_CXX_COMPILER /usr/local/bin/rx-elf-g++ CACHE FILEPATH "target_cxx" FORCE)

# have to override ar,nm and ranlib to use the gcc wrapper for LTO support.
# force it into the cmake cache or else it will try lookup them up later and
# get it wrong.
set (CMAKE_AR /usr/local/bin/rx-elf-gcc-ar CACHE FILEPATH "target_ar" FORCE)
set (CMAKE_NM /usr/local/bin/rx-elf-gcc-nm CACHE FILEPATH "target_nm" FORCE)
set (CMAKE_RANLIB /usr/local/bin/rx-elf-gcc-ranlib CACHE FILEPATH "target_ranlib" FORCE)

# for some reason, can't enable the ASM language here.  it makes cmake eat
# all memory and eventually crash.  looks like an infinite loop somewhere.
#enable_language(ASM)

# search for programs in the build host directories
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


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

get_filename_component (TOOLCHAIN_DIR ${CMAKE_TOOLCHAIN_FILE} DIRECTORY CACHE)
mark_as_internal (TOOLCHAIN_DIR)


# if LTO is used, we can't use "add_compile_options" because these will
# not be added to the final linker options, which is needed for the whole
# thing to work.
# https://cmake.org/pipermail/cmake-developers/2014-June/010623.html

option (TARGET_LTO "Link time optimization (LTO)" ON)
option (TARGET_GC_SECTIONS "Linker GC sections" ON)
option (TARGET_DATA_SECTIONS "Separate data sections" ON)
option (TARGET_FUNCTION_SECTIONS "Separate function sections" OFF)
option (TARGET_SAVE_TEMPS "Save intermediate compiler output files" OFF)
option (TARGET_VERBOSE_LINKER "Verbose linking" OFF)
option (TARGET_CXX_EXCEPTIONS "C++ Exceptions" ON)
option (TARGET_C_EXCEPTIONS "C Exceptions" OFF)
option (TARGET_DISABLE_GLOBAL_DTORS "Disable global destructor code" ON)
set (TARGET_OPTIMIZE "-O2" CACHE STRING "Optimization flags")
option (TARGET_STRIP_SYMBOLS "Strip symbols" OFF)

set (TARGET_ROM_END "0xFFFFFFFF" CACHE STRING "ROM end address")
set (TARGET_ROM_SIZE "1024*256" CACHE STRING "ROM size in bytes")
set (TARGET_RAM_SIZE "1024*128" CACHE STRING "RAM size in bytes")
set (TARGET_ISTACK_SIZE "1024" CACHE STRING "istack size in bytes")
set (TARGET_USTACK_SIZE "1024*2" CACHE STRING "ustack size in bytes")

set (TARGET_ENDIAN "little" CACHE STRING "Endian")
set_property (CACHE TARGET_ENDIAN PROPERTY STRINGS little big)


# if LTO is used it's better to disable function-sections as it 
# results in smaller code.

if (TARGET_LTO)
  set (OPT_LTO "-flto -flto-compression-level=0")
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

if (TARGET_DISABLE_GLOBAL_DTORS)
  set (OPT_DISABLE_GLOBAL_DTORS "-DDISABLE_GLOBAL_DTORS")
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/global_dtors.ld "")

else ()
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/global_dtors.ld "\
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array ))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  .dtors :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }")

endif ()

file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/memory_config.ld "\
  _rx_rom_end = ${TARGET_ROM_END};
  _rx_rom_size = ${TARGET_ROM_SIZE};
  _rx_ram_size = ${TARGET_RAM_SIZE};
  _rx_istack_size = ${TARGET_ISTACK_SIZE};
  _rx_ustack_size = ${TARGET_USTACK_SIZE};
")


set (TARGET_LINKER_SCRIPT ${TOOLCHAIN_DIR}/linker_script.ld)
mark_as_internal (TARGET_LINKER_SCRIPT)

set (TARGET_LINKER_SCRIPT_DEPS
  ${CMAKE_CURRENT_BINARY_DIR}/global_dtors.ld
  ${CMAKE_CURRENT_BINARY_DIR}/memory_config.ld
  ${TARGET_LINKER_SCRIPT}
)
mark_as_internal (TARGET_LINKER_SCRIPT_DEPS)


set (LINKER_STATIC "-Wl,-Bstatic")

# GAS and/or GCC (at least for rx-elf) produce things like
#   . byte 0xFFFFFFFFFFFFFFFF
# to store a -1 in an int8_t
# GAS then issues are warning.
set (ASM_OPTIONS "-Wa,--no-warn")


# default options

set (TARGET_OPTIONS "-msave-acc-in-interrupts")

if (TARGET_STRIP_SYMBOLS)
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -s")
endif ()

if(${TARGET_ENDIAN} STREQUAL "big")
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -mbig-endian-data -Wl,--oformat=elf32-rx-be -Wa,-mbig-endian")
#  set (TARGET_OBJCOPY_OPTIONS "--input-target=elf32-rx-be-ns")
elseif (${TARGET_ENDIAN} STREQUAL "little")
  set (TARGET_OPTIONS "${TARGET_OPTIONS} -mlittle-endian-data -Wl,--oformat=elf32-rx-le -Wa,-mlittle-endian")
  set (TARGET_OBJCOPY_OPTIONS "")
else ()
  message (FATAL_ERROR "undefined target endian setting")
endif ()

set (C_LANG_OPTIONS "-std=c99 -Werror=return-type")
set (CXX_LANG_OPTIONS "-std=c++14 -Werror=return-type")

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
${OPT_DISABLE_GLOBAL_DTORS} \
" CACHE STRING "target_c_flags" FORCE)

set (CMAKE_CXX_FLAGS "\
${TARGET_OPTIMIZE} \
${OPT_CXX_EXCEPTIONS} \
${CXX_LANG_OPTIONS} \
${TARGET_OPTIONS} \
${ASM_OPTIONS} \
${OPT_FUNCTION_SECTIONS} \
${OPT_DATA_SECTIONS} \
${OPT_SAVE_TEMPS} \
${OPT_LTO} \
${OPT_DISABLE_GLOBAL_DTORS} \
" CACHE STRING "target_cxx_flags" FORCE)

# note: cmake uses gcc as the assembler for .S files.
set (CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "target_asm_flags" FORCE)

set (CMAKE_EXE_LINKER_FLAGS "\
${OPT_GC_SECTIONS} -nostartfiles -nodefaultlibs \
${ASM_OPTIONS} ${LINKER_VERBOSE} ${LINKER_STATIC} \
-T '${TARGET_LINKER_SCRIPT}' \
" CACHE STRING "target_ld_flags" FORCE)


macro (add_additional_executable_targets _var)

get_filename_component (TARGET_EXE_FILENAME_WE ${_var} NAME_WE)

add_custom_command (TARGET ${_var}
  POST_BUILD COMMAND rx-elf-size ${_var})

add_custom_target ("${TARGET_EXE_FILENAME_WE}.mot"
  COMMAND ${CMAKE_OBJCOPY} ${TARGET_OBJCOPY_OPTIONS} --srec-forceS3 --srec-len 32 -O srec ${_var} ${TARGET_EXE_FILENAME_WE}.mot
  DEPENDS ${_var}
)

# http://stackoverflow.com/questions/19019199/raw-binary-file-generated-by-objcopy-is-too-big
add_custom_target ("${TARGET_EXE_FILENAME_WE}.bin"
  COMMAND ${CMAKE_OBJCOPY} ${TARGET_OBJCOPY_OPTIONS} -O binary ${_var} ${TARGET_EXE_FILENAME_WE}.bin
  DEPENDS ${_var}
)

# DJ's rxusb tool has problems when reading/uploading small ELFs (< 3KByte)
# but s-rec seems to be working fine.  Thus, always use s-rec.
add_custom_target ("${TARGET_EXE_FILENAME_WE}.upload"
  COMMAND ${CMAKE_SOURCE_DIR}/../upload.sh "${CMAKE_SOURCE_DIR}/../" "${TARGET_EXE_FILENAME_WE}.mot"
  DEPENDS ${TARGET_EXE_FILENAME_WE}.mot
)

add_custom_target ("${TARGET_EXE_FILENAME_WE}.upload_usb"
  COMMAND ${CMAKE_SOURCE_DIR}/../upload_usb.sh "${CMAKE_SOURCE_DIR}/../" "${TARGET_EXE_FILENAME_WE}.mot"
  DEPENDS ${TARGET_EXE_FILENAME_WE}.mot
)

add_custom_target ("${TARGET_EXE_FILENAME_WE}.upload_tftp"
  COMMAND ${CMAKE_SOURCE_DIR}/../upload_tftp.sh "${CMAKE_SOURCE_DIR}/../" "${TARGET_EXE_FILENAME_WE}.mot"
  DEPENDS ${TARGET_EXE_FILENAME_WE}.mot
)

add_custom_target ("${TARGET_EXE_FILENAME_WE}.upload_user_boot"
  COMMAND ${CMAKE_SOURCE_DIR}/../upload_user_boot.sh "${CMAKE_SOURCE_DIR}/../" "${TARGET_EXE_FILENAME_WE}.mot"
  DEPENDS ${TARGET_EXE_FILENAME_WE}.mot
)

endmacro()

endif ()
