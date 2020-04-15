function(gen_attrdef_file name type)
	add_custom_command(OUTPUT ${name}.c
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_CURRENT_SOURCE_DIR}/attr_parser.py
			-m ${CMAKE_CURRENT_SOURCE_DIR}/master_${name}.xml
			-e /dev/null -s ${name}.c -a ${type}
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/master_${name}.xml
		COMMENT "Generating ${name}.c"
		USES_TERMINAL
	)
endfunction()

gen_attrdef_file(job_attr_def job)
gen_attrdef_file(node_attr_def node)
gen_attrdef_file(queue_attr_def queue)
gen_attrdef_file(resc_def_all resc)
gen_attrdef_file(resv_attr_def resv)
gen_attrdef_file(sched_attr_def sched)
gen_attrdef_file(svr_attr_def server)

add_library(attr STATIC
	attr_atomic.c
	attr_fn_acl.c
	attr_fn_arst.c
	attr_fn_b.c
	attr_fn_c.c
	attr_fn_entlim.c
	attr_fn_f.c
	attr_fn_hold.c
	attr_fn_intr.c
	attr_fn_l.c
	attr_fn_ll.c
	attr_fn_resc.c
	attr_fn_size.c
	attr_fn_str.c
	attr_fn_time.c
	attr_fn_unkn.c
	attr_func.c
	attr_node_func.c
	attr_resc_func.c
	job_attr_def.c
	Long_.c
	LTostr.c
	node_attr_def.c
	queue_attr_def.c
	resc_def_all.c
	resc_map.c
	resv_attr_def.c
	sched_attr_def.c
	strToL.c
	strTouL.c
	svr_attr_def.c
	uLTostr.c
)
target_include_directories(attr PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)

add_library(pbs::lib::attr ALIAS attr)
