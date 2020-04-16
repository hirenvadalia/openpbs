set(common_libs
	pbs::lib::pbs_static
	pbs::lib::net
	pbs::lib::sec
	pbs::lib::util
	pthread
	nsl
)

add_executable(pbsdsh pbsdsh.c)
target_include_directories(pbsdsh PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
target_link_libraries(pbsdsh PRIVATE ${common_libs})
