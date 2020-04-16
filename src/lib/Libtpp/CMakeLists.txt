add_library(tpp STATIC
	tpp_client.c
	tpp_em.c
	tpp_platform.c
	tpp_router.c
	tpp_transport.c
	tpp_util.c
)
target_include_directories(tpp PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
add_library(pbs::lib::tpp ALIAS tpp)
