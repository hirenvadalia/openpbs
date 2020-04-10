# Tries to find editline headers and libraries
#
# Usage of this module as follows:
#
#     find_package(Editline)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  Editline_ROOT_DIR  Set this variable to the root installation of
#                     editline if the module has problems finding
#                     the proper installation path.
#
# Variables defined by this module:
#
#  Editline_FOUND              System has Editline libs/headers
#  Editline_LIBRARIES          The Editline libraries
#  Editline_INCLUDE_DIR        The location of Editline headers
#  Editline_VERSION            The full version of Editline
#  Editline_VERSION_MAJOR      The version major of Editline
#  Editline_VERSION_MINOR      The version minor of Editline

find_path(Editline_INCLUDE_DIR
	NAMES histedit.h
	HINTS ${Editline_ROOT_DIR}/include
)

if(Editline_INCLUDE_DIR)
	file(STRINGS ${Editline_INCLUDE_DIR}/histedit.h Editline_header REGEX "^#define.LIBEDIT_[A-Z]+.*$")
	string(REGEX REPLACE ".*#define.LIBEDIT_MAJOR[ \t]+([0-9]+).*" "\\1" Editline_VERSION_MAJOR "${Editline_header}")
	string(REGEX REPLACE ".*#define.LIBEDIT_MINOR[ \t]+([0-9]+).*" "\\1" Editline_VERSION_MINOR "${Editline_header}")
	set(Editline_VERSION_MAJOR ${Editline_VERSION_MAJOR} CACHE STRING "" FORCE)
	set(Editline_VERSION_MINOR ${Editline_VERSION_MINOR} CACHE STRING "" FORCE)
	set(Editline_VERSION ${Editline_VERSION_MAJOR}.${Editline_VERSION_MINOR} CACHE STRING "" FORCE)
endif()

find_library(Editline_LIBRARIES
	NAMES edit
	HINTS ${Editline_ROOT_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	Editline
	DEFAULT_MSG
	Editline_LIBRARIES
	Editline_INCLUDE_DIR
)

mark_as_advanced(
	Editline_ROOT_DIR
	Editline_LIBRARIES
	Editline_INCLUDE_DIR
	Editline_VERSION
	Editline_VERSION_MAJOR
	Editline_VERSION_MINOR
)
