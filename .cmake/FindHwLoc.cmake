# Tries to find HwLoc headers and libraries
#
# Usage of this module as follows:
#
#     find_package(HwLoc)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  HwLoc_ROOT_DIR  Set this variable to the root installation of
#                  HwLoc if the module has problems finding
#                  the proper installation path.
#
# Variables defined by this module:
#
#  HwLoc_FOUND              System has HwLoc libs/headers
#  HwLoc_LIBRARIES          The HwLoc libraries
#  HwLoc_INCLUDE_DIR        The location of HwLoc headers
#  HwLoc_VERSION            The full version of HwLoc
#  HwLoc_VERSION_MAJOR      The version major of HwLoc
#  HwLoc_VERSION_MINOR      The version minor of HwLoc

find_path(HwLoc_INCLUDE_DIR
	NAMES hwloc.h
	HINTS ${HwLoc_ROOT_DIR}/include
)

if(HwLoc_INCLUDE_DIR)
	file(STRINGS ${HwLoc_INCLUDE_DIR}/hwloc.h HwLoc_header REGEX "^#define.HWLOC_API_VERSION.*$")
	string(REGEX REPLACE ".*#define.HWLOC_API_VERSION[ \t]+(0x[0-9a-fA-F]+).*" "\\1" HwLoc_VERSION "${HwLoc_header}")
	math(EXPR HwLoc_VERSION_MAJOR "${HwLoc_VERSION} >> 16" OUTPUT_FORMAT DECIMAL)
	math(EXPR HwLoc_VERSION_MINOR "(${HwLoc_VERSION} - (${HwLoc_VERSION_MAJOR} << 16)) >> 8" OUTPUT_FORMAT DECIMAL)
	set(HwLoc_VERSION_MAJOR ${HwLoc_VERSION_MAJOR} CACHE STRING "" FORCE)
	set(HwLoc_VERSION_MINOR ${HwLoc_VERSION_MINOR} CACHE STRING "" FORCE)
	set(HwLoc_VERSION ${HwLoc_VERSION_MAJOR}.${HwLoc_VERSION_MINOR} CACHE STRING "" FORCE)
endif()

find_library(HwLoc_LIBRARIES
	NAMES hwloc hwloc_embedded
	HINTS ${HwLoc_ROOT_DIR}/lib
)

#include(CheckFunctionExists)
#find_library(_numa_libs numa)
#set(CMAKE_REQUIRED_LIBRARIES "${_numa_libs}")
#check_function_exists(mbind _NEED_NUMA)
#if(${_NEED_NUMA})
#	set(HwLoc_LIBRARIES "${HwLoc_LIBRARIES} ${_numa_libs}")
#endif()

#AS_CASE([x$target_os],
#    [xlinux*],
#      AC_CHECK_LIB([numa], [mbind], [hwloc_lib="$hwloc_lib -lnuma"])
#      AC_CHECK_LIB([udev], [udev_new], [hwloc_lib="$hwloc_lib -ludev"])
#      AC_CHECK_LIB([pciaccess], [pci_system_init], [hwloc_lib="$hwloc_lib -lpciaccess"])
#  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	HwLoc
	DEFAULT_MSG
	HwLoc_LIBRARIES
	HwLoc_INCLUDE_DIR
)

mark_as_advanced(
	HwLoc_ROOT_DIR
	HwLoc_LIBRARIES
	HwLoc_INCLUDE_DIR
	HwLoc_VERSION
	HwLoc_VERSION_MAJOR
	HwLoc_VERSION_MINOR
)
