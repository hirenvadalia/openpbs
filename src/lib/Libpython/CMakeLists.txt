add_custom_command(OUTPUT pbs_ifl_wrap.c
	COMMAND ${SWIG_EXECUTABLE}
		-I${CMAKE_SOURCE_DIR}/src/include
		-I${CMAKE_BINARY_DIR}/src/include
		-I${SWIG_DIR} -I${SWIG_DIR}/python
		-python -outcurrentdir
		${CMAKE_CURRENT_SOURCE_DIR}/pbs_ifl.i
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/pbs_ifl.i
	COMMENT "Generating pbs_ifl_wrap.c"
	USES_TERMINAL
)

add_library(pbspython STATIC
	shared_python_utils.c
	common_python_utils.c
	pbs_ifl_wrap.c
	pbs_python_external.c
	pbs_python_svr_external.c
	module_pbs_v1.c
	pbs_python_svr_internal.c
	pbs_python_svr_size_type.c
	pbs_python_import_types.c
)
target_include_directories(pbspython PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include
	${Python3_INCLUDE_DIRS})
target_link_libraries(pbspython PRIVATE ${Python3_LIBRARIES})
set_target_properties(pbspython PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/pbs_ifl.py)
add_library(pbs::lib::py ALIAS pbspython)

add_library(pbspython_svr STATIC
	shared_python_utils.c
	common_python_utils.c
	pbs_ifl_wrap.c
	pbs_python_external.c
	pbs_python_svr_external.c
	module_pbs_v1.c
	pbs_python_svr_internal.c
	pbs_python_svr_size_type.c
	pbs_python_import_types.c
)
target_compile_definitions(pbspython_svr PRIVATE LIBPYTHONSVR)
target_include_directories(pbspython_svr PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include
	${Python3_INCLUDE_DIRS})
target_link_libraries(pbspython_svr PRIVATE ${Python3_LIBRARIES})
set_target_properties(pbspython_svr PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/pbs_ifl.py)
add_library(pbs::lib::pysvr ALIAS pbspython_svr)
