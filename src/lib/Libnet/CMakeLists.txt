add_library(net STATIC
	get_hostaddr.c
	net_client.c
	net_server.c
	net_set_clse.c
	port_forwarding.c
	hnls.c
)
target_include_directories(net PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)

add_library(pbs::lib::net ALIAS net)
