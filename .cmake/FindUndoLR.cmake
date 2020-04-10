# Tries to find UndoLR headers and libraries
#
# Usage of this module as follows:
#
#     find_package(UndoLR)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  UndoLR_ROOT_DIR  Set this variable to the root installation of
#                   undolr if the module has problems finding
#                   the proper installation path.
#
# Variables defined by this module:
#
#  UndoLR_FOUND              System has UndoLR libs/headers
#  UndoLR_LIBRARIES          The UndoLR libraries
#  UndoLR_INCLUDE_DIR        The location of UndoLR headers
#  UndoLR_VERSION            The full version of UndoLR
#  UndoLR_VERSION_MAJOR      The version major of UndoLR
#  UndoLR_VERSION_MINOR      The version minor of UndoLR

find_path(UndoLR_INCLUDE_DIR
	NAMES undolr.h
	HINTS ${UndoLR_ROOT_DIR}/include
)

if(UndoLR_INCLUDE_DIR)
	file(STRINGS ${UndoLR_INCLUDE_DIR}/undolr.h UndoLR_header REGEX "^#define.LIBEDIT_[A-Z]+.*$")
	string(REGEX REPLACE ".*#define.LIBEDIT_MAJOR[ \t]+([0-9]+).*" "\\1" UndoLR_VERSION_MAJOR "${UndoLR_header}")
	string(REGEX REPLACE ".*#define.LIBEDIT_MINOR[ \t]+([0-9]+).*" "\\1" UndoLR_VERSION_MINOR "${UndoLR_header}")
	set(UndoLR_VERSION_MAJOR ${UndoLR_VERSION_MAJOR} CACHE STRING "" FORCE)
	set(UndoLR_VERSION_MINOR ${UndoLR_VERSION_MINOR} CACHE STRING "" FORCE)
	set(UndoLR_VERSION ${UndoLR_VERSION_MAJOR}.${UndoLR_VERSION_MINOR} CACHE STRING "" FORCE)
endif()

find_library(UndoLR_LIBRARIES
	NAMES undolr_pic_x64
	HINTS ${UndoLR_ROOT_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	UndoLR
	DEFAULT_MSG
	UndoLR_LIBRARIES
	UndoLR_INCLUDE_DIR
)

mark_as_advanced(
	UndoLR_ROOT_DIR
	UndoLR_LIBRARIES
	UndoLR_INCLUDE_DIR
	UndoLR_VERSION
	UndoLR_VERSION_MAJOR
	UndoLR_VERSION_MINOR
)
