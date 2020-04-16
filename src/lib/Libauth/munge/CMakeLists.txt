add_library(auth_munge SHARED
	munge_supp.c
)
target_include_directories(auth_munge PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
