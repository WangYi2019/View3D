
# on linux we can't do a full static link
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set (TARGET_FULL_STATIC OFF CACHE BOOL "")
endif ()


# when cross compiling a DLL for windows, we have to do a full static link
if (CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set (TARGET_FULL_STATIC ON CACHE BOOL "")
endif ()

