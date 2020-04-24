function(gen_attrdef_file name type)
	add_custom_command(OUTPUT ecl_${name}.c
		COMMAND ${Python3_EXECUTABLE}
			${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_parser.py
			-m ${CMAKE_SOURCE_DIR}/src/lib/Libattr/master_${name}.xml
			-s /dev/null -e ecl_${name}.c -a ${type}
		MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/lib/Libattr/master_${name}.xml
		COMMENT "Generating ecl_${name}.c"
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

set(sources
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_arst.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_b.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_c.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_f.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_hold.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_intr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_l.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_ll.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_size.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_str.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_time.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_fn_unkn.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_func.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/attr_resc_func.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/Long_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/LTostr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/resc_map.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/uLTostr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/strToL.c
	${CMAKE_SOURCE_DIR}/src/lib/Libattr/strTouL.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/batch_status.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/check_job_script.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/chk_Jrange.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/ck_job_name.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/cnt2server.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/cvtdate.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/get_attr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/get_dataservice_usr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/get_server.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/isjobid.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/locate_job.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_at.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_depend.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_destid.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_equal.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_jobid.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/parse_stage.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/pbs_json.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/prepare_path.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/prt_job_err.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/set_attr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libcmds/set_resource.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/dis_helpers.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/dis.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/discui_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/discul_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disi10d_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disi10l_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disiui_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disp10d_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disp10l_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrcs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrd.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrf.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrfcs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrfst.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrl_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsi.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsi_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsl_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrss.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrst.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disruc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrui.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrul.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrus.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswcs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswf.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswl_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswsi.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswsl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswui.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswui_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswul.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/ps_dis.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/diswull.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrull.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/discull_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libdis/disrsll_.c
	${CMAKE_SOURCE_DIR}/src/lib/Libecl/ecl_verify.c
	${CMAKE_SOURCE_DIR}/src/lib/Libecl/ecl_verify_datatypes.c
	${CMAKE_SOURCE_DIR}/src/lib/Libecl/ecl_verify_values.c
	${CMAKE_SOURCE_DIR}/src/lib/Libecl/ecl_verify_object_name.c
	${CMAKE_SOURCE_DIR}/src/lib/Libecl/pbs_client_thread.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/advise.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/auth.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/conn_table.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Authen.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_CopyHookFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_DelHookFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_JobCred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_JobFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_JobId.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Manage.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_MsgJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_MoveJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_UserCred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_QueueJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Reg.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_ReqExt.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_ReqHdr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Resc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_RunJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Shut.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Sig.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Status.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Track.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_attrl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_attropl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_rpyc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_svrattrl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_ModifyResv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_PreemptJobs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_CopyHookFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_CpyFil.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_DelHookFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_JobCred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_JobFile.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_JobId.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_UserCred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Manage.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_MsgJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_MoveJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_QueueJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Reg.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_ReqExt.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_ReqHdr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_RunJob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Shut.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Sig.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Status.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Track.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_attrl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_attropl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_reply.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_SubmitResv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_ModifyResv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_PreemptJobs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_svrattrl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/entlim_parse.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/get_svrport.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/grunt_parse.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_hook.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_jcred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_manager.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_manage2.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_msg2.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_rdrpy.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_sig2.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_status2.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_submit.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_submit_resv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_status.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_ucred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_cred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/enc_Cred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/dec_Cred.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_modify_resv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/int_preempt_jobs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/list_link.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/PBS_attr.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_authors.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_get_attribute_errors.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_geterrmg.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_geterrno.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_loadconf.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_quote_parse.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbs_statfree.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_alterjo.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_asyrun.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_connect.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_deljob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_holdjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_locjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_manager.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_movejob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_msgjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_orderjo.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_rerunjo.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_resc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_rlsjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_runjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_selectj.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_sigjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_stagein.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_stathost.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statjob.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statnode.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statque.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statsrv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statsched.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_submit.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_termin.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_submit_resv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_stathook.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_delresv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statresv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_confirmresv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_defschreply.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_statrsc.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_modify_resv.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/pbsD_Preempt_Jobs.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/ifl_impl.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/ifl_pointers.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/rm.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/strsep.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/tcp_dis.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/tm.c
	${CMAKE_SOURCE_DIR}/src/lib/Libifl/xml_encode_decode.c
	${CMAKE_SOURCE_DIR}/src/lib/Liblog/pbs_messages.c
	${CMAKE_SOURCE_DIR}/src/lib/Libsec/cs_standard.c
	${CMAKE_SOURCE_DIR}/src/lib/Libutil/misc_utils.c
	${CMAKE_SOURCE_DIR}/src/lib/Libutil/pbs_secrets.c
	${CMAKE_SOURCE_DIR}/src/lib/Libutil/pbs_aes_encrypt.c
	${CMAKE_SOURCE_DIR}/src/lib/Libnet/hnls.c
	ecl_job_attr_def.c
	ecl_svr_attr_def.c
	ecl_sched_attr_def.c
	ecl_node_attr_def.c
	ecl_queue_attr_def.c
	ecl_resc_def_all.c
	ecl_resv_attr_def.c
)
add_library(_pbsobj MODULE ${sources})
target_link_libraries(_pbsobj PRIVATE crypto pthread)
target_include_directories(_pbsobj PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
add_library(pbs SHARED $<TARGET_OBJECTS:_pbsobj>)
target_link_libraries(pbs PRIVATE crypto pthread)
target_include_directories(pbs PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
add_library(pbs::lib::pbs_shared ALIAS pbs)
add_library(pbs_static STATIC $<TARGET_OBJECTS:_pbsobj>)
target_link_libraries(pbs_static PRIVATE crypto pthread)
target_include_directories(pbs_static PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_BINARY_DIR}/src/include)
set_target_properties(pbs_static PROPERTIES OUTPUT_NAME pbs)
add_library(pbs::lib::pbs_static ALIAS pbs_static)

configure_file(pbs.pc.in pbs.pc @ONLY)
