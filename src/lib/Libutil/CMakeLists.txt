add_library(util STATIC
	get_hostname.c
	execvnode_seq_util.c
	pbs_ical.c
	misc_utils.c
	avltree.c
	hook.c
	work_task.c
	entlim.c
	daemon_protect.c
	pbs_array_list.c
	pbs_secrets.c
	pbs_aes_encrypt.c
	undolr.c
)
target_include_directories(util PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include
	${Ical_INCLUDES}
	${UndoLR_INCLUDES})
add_library(pbs::lib::util ALIAS util)
