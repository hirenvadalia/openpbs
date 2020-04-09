cmake_minimum_required(VERSION 3.15)
if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
	set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()
set(PBS_VERSION 20.0.0 CACHE STRING "PBS version")
project(PBSPro VERSION ${PBS_VERSION})

# include(GNUInstallDirs)

set(PBS_SERVER_HOME /var/spool/pbs CACHE STRING "PBS home directory")
# FIXME: subst database_user
set(PBS_DATA_SERVICE_USER postgres CACHE STRING "PBS database user")
# FIXME: Do we need PBS_SERVER_FILE_NAME?
set(PBS_DATA_SERVICE_PORT 15007 CACHE STRING "PBS database port")
set(PBS_CONF_FILE /etc/pbs.conf CACHE STRING "PBS configuration file location")
set(TMP_DIR /var/tmp CACHE STRING "PBS temporary file location")
set(PBS_CORE_LIMIT unlimited CACHE STRING "PBS daemon coredump limit")
set(TCL_ATRSEP "." CACHE STRING "Tcl attribute separator")
set(SENDMAIL_CMD /usr/sbin/sendmail CACHE STRING "Full path to the sendmail executable")
set(XAUTH_BINARY xauth CACHE STRING "Name of the xauth executable")

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(EXPAT REQUIRED)
find_package(SWIG REQUIRED)
find_package(TCL REQUIRED)

add_subdirectory(src)
