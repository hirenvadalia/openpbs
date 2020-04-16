add_library(sec STATIC
	cs_standard.c
)
target_include_directories(sec PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
add_library(pbs::lib::sec ALIAS sec)
