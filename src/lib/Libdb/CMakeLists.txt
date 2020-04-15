add_library(db STATIC
	db_postgres_impl.c
	db_postgres_common.c
	db_postgres_attr.c
	db_postgres_job.c
	db_postgres_resv.c
	db_postgres_svr.c
	db_postgres_que.c
	db_postgres_node.c
	db_postgres_sched.c
)
target_include_directories(db PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include
	${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(db PRIVATE ${PostgreSQL_LIBRARIES})

add_library(pbs::lib::db ALIAS db)
