# Tries to find Libical headers and libraries
#
# Usage of this module as follows:
#
#     find_package(Ical)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  Ical_ROOT_DIR  Set this variable to the root installation of
#                 Libical if the module has problems finding
#                 the proper installation path.
#
# Variables defined by this module:
#
#  Ical_FOUND              System has Libical libs/headers
#  Ical_LIBRARIES          The Libical libraries
#  Ical_INCLUDE_DIR        The location of Libical headers
#  Ical_VERSION            The full version of Libical
#  Ical_VERSION_MAJOR      The version major of Libical
#  Ical_VERSION_MINOR      The version minor of Libical

find_path(Ical_INCLUDE_DIR
	NAMES libical/ical.h
	HINTS ${Ical_ROOT_DIR}/include
)

if(Ical_INCLUDE_DIR)
	file(STRINGS ${Ical_INCLUDE_DIR}/libical/ical.h Ical_header REGEX "^#define[ \t]+ICAL_VERSION[ \t]+.*$")
	string(REGEX REPLACE ".*ICAL_VERSION[ \t]+\"([0-9]+)..*\"" "\\1" Ical_VERSION_MAJOR "${Ical_header}")
	string(REGEX REPLACE ".*ICAL_VERSION[ \t]+\"[0-9]+.([0-9]+).*\"" "\\1" Ical_VERSION_MINOR "${Ical_header}")
	set(Ical_VERSION_MAJOR ${Ical_VERSION_MAJOR} CACHE STRING "" FORCE)
	set(Ical_VERSION_MINOR ${Ical_VERSION_MINOR} CACHE STRING "" FORCE)
	set(Ical_VERSION ${Ical_VERSION_MAJOR}.${Ical_VERSION_MINOR} CACHE STRING "" FORCE)
endif()

find_library(Ical_LIBRARIES
	NAMES ical
	HINTS ${Ical_ROOT_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	LibIcal
	DEFAULT_MSG
	Ical_LIBRARIES
	Ical_INCLUDE_DIR
)

mark_as_advanced(
	Ical_ROOT_DIR
	Ical_LIBRARIES
	Ical_INCLUDE_DIR
	Ical_VERSION
	Ical_VERSION_MAJOR
	Ical_VERSION_MINOR
)
