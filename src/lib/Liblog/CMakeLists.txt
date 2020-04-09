add_library(log STATIC
	chk_file_sec.c
	log_event.c
	pbs_log.c
	pbs_messages.c
	setup_env.c
)
target_include_directories(log PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)

add_library(pbs::lib::log ALIAS log)
