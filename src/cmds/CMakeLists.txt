set(common_libs
	pbs::lib::pbs_static
	pbs::lib::net
	pbs::lib::sec
	pbs::lib::util
	pthread
	dl
	nsl #FIXME find socket lib
)

add_library(cmnsrc OBJECT ${CMAKE_SOURCE_DIR}/src/lib/Libcmds/cmds_common.c)
target_include_directories(cmnsrc PRIVATE
		${CMAKE_SOURCE_DIR}/src/include
		${CMAKE_BINARY_DIR}/src/include)

function(add_cmd name)
	set(options NEED_CMNSRC)
	set(args)
	set(list_args SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 cmd "${options}" "${args}" "${list_args}")
	set(_sources ${cmd_SOURCES})
	if (cmd_NEED_CMNSRC)
		set(_sources ${_sources} $<TARGET_OBJECTS:cmnsrc>)
	endif()
	add_executable(${name} ${_sources})
	target_include_directories(${name} PRIVATE
		${CMAKE_SOURCE_DIR}/src/include
		${CMAKE_BINARY_DIR}/src/include)
	target_link_libraries(${name} PRIVATE ${common_libs})
endfunction()

add_cmd(pbsdsh SOURCES pbsdsh.c)
add_cmd(pbs_attach SOURCES pbs_attach.c NEED_CMNSRC)
add_cmd(pbs_demux SOURCES pbs_demux.c)
#FIXME pbs_ds_password
add_cmd(pbs_password SOURCES pbs_password.c NEED_CMNSRC)
add_cmd(pbs_tmrsh SOURCES pbs_tmrsh.c)
add_cmd(pbs_ralter SOURCES pbs_ralter.c NEED_CMNSRC)
add_cmd(pbs_rdel SOURCES pbs_rdel.c NEED_CMNSRC)
add_cmd(pbs_rstat SOURCES pbs_rstat.c NEED_CMNSRC)
add_cmd(pbs_rsub SOURCES pbs_rsub.c NEED_CMNSRC)
add_cmd(pbs_migrate_users SOURCES pbs_migrate_users.c NEED_CMNSRC)
add_cmd(pbs_release_nodes SOURCES pbs_release_nodes.c NEED_CMNSRC)
add_cmd(qalter SOURCES qalter.c NEED_CMNSRC)
add_cmd(qdel SOURCES qdel.c NEED_CMNSRC)
add_cmd(qdisable SOURCES qdisable.c NEED_CMNSRC)
add_cmd(qenable SOURCES qenable.c NEED_CMNSRC)
add_cmd(qhold SOURCES qhold.c NEED_CMNSRC)
#FIXME qmgr
add_cmd(qmove SOURCES qmove.c NEED_CMNSRC)
add_cmd(qorder SOURCES qorder.c NEED_CMNSRC)
add_cmd(qmsg SOURCES qmsg.c NEED_CMNSRC)
add_cmd(qrerun SOURCES qrerun.c NEED_CMNSRC)
add_cmd(qrls SOURCES qrls.c NEED_CMNSRC)
add_cmd(qrun SOURCES qrun.c NEED_CMNSRC)
add_cmd(qselect SOURCES qselect.c NEED_CMNSRC)
add_cmd(qsig SOURCES qsig.c NEED_CMNSRC)
add_cmd(qstat SOURCES qstat.c NEED_CMNSRC)
add_cmd(qstart SOURCES qstart.c NEED_CMNSRC)
add_cmd(qstop SOURCES qstop.c NEED_CMNSRC)
add_cmd(qsub SOURCES qsub.c NEED_CMNSRC)
add_cmd(qterm SOURCES qterm.c NEED_CMNSRC)
