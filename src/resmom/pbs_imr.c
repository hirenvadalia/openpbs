/*
 * Copyright (C) 1994-2020 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * For a copy of the commercial license terms and conditions,
 * go to: (http://www.pbspro.com/UserArea/agreement.html)
 * or contact the Altair Legal Department.
 *
 * Altair’s dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of PBS Pro and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair’s trademarks, including but not limited to "PBS™",
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
 * trademark licensing policies.
 *
 */
//#include <pbs_config.h>
#define PBS_MOM
#include "job.h"
#include "hook.h"
#include "mom_hook_func.h"
#include "placementsets.h"
#include "pbs_imr.h"

void
log_errf(int errnum, const char *routine, const char *fmt, ...)
{
	va_list args;
	int len;
	char logbuf[LOG_BUF_SIZE];
	char *buf;

	va_start(args, fmt);
	len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
	if (len >= sizeof(logbuf)) {
		buf = pbs_asprintf_format(len, fmt, args);
		if (buf == NULL) {
			va_end(args);
			return;
		}
	} else
		buf = logbuf;

	log_err(errnum, routine, buf);
	if (len >= sizeof(logbuf))
		free(buf);
	va_end(args);
}

void
log_joberrf(int errnum, const char *routine, char *jobid, const char *fmt, ...)
{
	va_list args;
	int len;
	char logbuf[LOG_BUF_SIZE];
	char *buf;

	va_start(args, fmt);
	len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
	if (len >= sizeof(logbuf)) {
		buf = pbs_asprintf_format(len, fmt, args);
		if (buf == NULL) {
			va_end(args);
			return;
		}
	} else
		buf = logbuf;

	log_joberr(errnum, routine, buf, jobid);
	if (len >= sizeof(logbuf))
		free(buf);
	va_end(args);
}

void
free_im(pbs_im_t *pims)
{
}

void
im_send_error(int stream, im_common_t *pimcs, int errcode, char *errmsg)
{
	// FIXME: do this
	tpp_eom(stream);
	tpp_close(stream);
}

void
im_req_join_job(int stream, im_common_t *pimcs, im_join_job_t *pimjs)
{
	int rc = 0;
	job *pjob = NULL;
	svrattrl *psatl = NULL;
	hnodent *np = NULL;
	int hook_errcode = 0;
	int hook_rc = 0;
	hook *last_phook = NULL;
	unsigned int hook_fail_action = 0;
	char hook_msg[HOOK_MSG_SIZE + 1];
	mom_hook_input_t hook_input;
	mom_hook_output_t hook_output;
	char namebuf[MAXPATHLEN + 1];

	if (check_ms(stream, NULL))
		return;

	/* Does job already exist? */
	pjob = find_job(pimcs->im_jobid);
	if (pjob) {
		/* Yes, throw away existing job */
		kill_job(pjob, SIGKILL);
		mom_deljob(pjob);
	}

	if ((pjob = job_alloc()) == NULL) {
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}
	strncpy(pjob->ji_qs.ji_jobid, pimcs->im_jobid, sizeof(pjob->ji_qs.ji_jobid) - 1);
	pjob->ji_stdout = pimjs->im_stdout;
	pjob->ji_stderr = pimjs->im_stderr;
	pjob->ji_extended.ji_ext.ji_credtype = pimjs->im_credtype;
	pjob->ji_numnodes = pimjs->im_nodenum;
	pjob->ji_modified = 1;
	pjob->ji_nodeid = -1;
	pjob->ji_qs.ji_svrflags = 0;
	pjob->ji_qs.ji_un_type = JOB_UNION_TYPE_MOM;
	pjob->ji_qs.ji_fileprefix[0] = '\0';
	/* decode attributes from request into job structure */
	resc_access_perm = READ_WRITE;
	psatl = (svrattrl *)GET_NEXT(pimjs->im_jattrs);
	for (; psatl != NULL; psatl = (svrattrl *)GET_NEXT(psatl->al_link)) {
		attribute_def *pdef = NULL;
		rc = 0;
		/* identify the attribute by name */
		index = find_attr(job_attr_def, psatl->al_name, JOB_ATR_LAST);
		if (index < 0) {
			/* didn't recognize the name */
			job_purge_mom(pjob);
			im_send_error(stream, pimcs, PBSE_NOATTR, NULL);
			return;
		}
		if (index == JOB_ATR_hashname && strlen(psatl->al_value) <= PBS_JOBBASE) {
			strncpy(pjob->ji_qs.ji_fileprefix, psatl->al_value, sizeof(pjob->ji_qs.ji_fileprefix) - 1);
		}
		pdef = &(job_attr_def[index]);
		/* decode attribute */
		rc = pdef->at_decode(&(pjob->ji_wattr[index]),
					psatl->al_name,
					psatl->al_resc,
					psatl->al_value);
		/*
		 * Unknown resources still get decoded
		 * under "unknown" resource def
		 */
		if (rc != 0 && rc != PBSE_UNKRESC) {
			job_purge_mom(pjob);
			im_send_error(stream, pimcs, rc, NULL);
			return;
		}
		if (psatl->al_op == DFLT) {
			pjob->ji_wattr[index].at_flags |= ATR_VFLAG_DEFLT;
		}
	}

	pjob->ji_nodeid = TM_ERROR_NODE;
	if ((rc = job_nodes_inner(pjob, &np)) != 0) {
		log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_JOB, LOG_NOTICE, pjob->ji_qs.ji_jobid, "job_nodes_inner failed with error %d", rc);
		nodes_free(pjob);
		job_purge_mom(pjob); // FIXME: confirm this
		im_send_error(stream, pimcs, rc, NULL);
		return;
	}
	/*
	 * Check to make sure we found ourself
	 */
	if (pjob->ji_nodeid == TM_ERROR_NODE) {
		log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_JOB, LOG_CRIT, pjob->ji_qs.ji_jobid,
				"no match for my hostname '%s' was found in exec_host2, "
				"possible network misconfiguration", mom_host);
		if (pjob->ji_wattr[(int) JOB_ATR_exec_host2].at_val.at_str != NULL) {
			log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_JOB, LOG_CRIT,
					pjob->ji_qs.ji_jobid, "exec_host2 = %s",
					pjob->ji_wattr[(int) JOB_ATR_exec_host2].at_val.at_str);
		}
		nodes_free(pjob);
		job_purge_mom(pjob); // FIXME: confirm this
		im_send_error(stream, pimcs, PBSE_INTERNAL, NULL);
		return;
	}

	DBPRT(("%s: JOIN_JOB %s node %d\n", __func__, pimcs->im_jobid, pjob->ji_nodeid))

	/* set remaining job structure elements */
	pjob->ji_qs.ji_state = JOB_STATE_RUNNING;
	pjob->ji_qs.ji_substate = JOB_SUBSTATE_PRERUN;
	pjob->ji_qs.ji_stime = time_now;
	pjob->ji_polltime = time_now;
	pjob->ji_wattr[(int)JOB_ATR_mtime].at_val.at_long = (long)time_now;
	pjob->ji_wattr[(int)JOB_ATR_mtime].at_flags |= ATR_VFLAG_SET;

	/*
	 * NULL value passed to hook_input.vnl
	 * means to assign vnode list using pjob->ji_host[]
	 */
	mom_hook_input_init(&hook_input);
	hook_input.pjob = pjob;

	mom_hook_output_init(&hook_output);
	hook_output.reject_errcode = &hook_errcode;
	hook_output.last_phook = &last_phook;
	hook_output.fail_action = &hook_fail_action;

	hook_rc = mom_process_hooks(HOOK_EVENT_EXECJOB_BEGIN, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
	switch (hook_rc) {
		case 1:
			/* explicit accept */
			break;
		case 2:
			/* no hook script executed - go ahead and accept event*/
			break;
		default:
			/* a value of '0' means explicit reject encountered. */
			if (hook_rc != 0) {
				/*
				 * we've hit an internal error (malloc error, full disk, etc...), so
				 * treat this now like a  hook error so hook fail_action
				 * will be consulted.
				 * Before, behavior of an internal error was to ignore it!
				 */
				hook_errcode = PBSE_HOOKERROR;
			}
			if (hook_errcode == PBSE_HOOKERROR && last_phook != NULL && (last_phook->fail_action & HOOK_FAIL_ACTION_OFFLINE_VNODES) != 0) {
				vnl_t *tvnl = NULL;
				char hook_buf[HOOK_BUF_SIZE];
				int vret = 0;

				snprintf(hook_buf, HOOK_BUF_SIZE, "1,%s", last_phook->hook_name);
				if (vnl_alloc(&tvnl) != NULL) {
					vret = vn_addvnr(tvnl, mom_short_name, VNATTR_HOOK_OFFLINE_VNODES, hook_buf, 0, 0, NULL);
				} else {
					vret = 1;
				}

				if (vret != 0) {
					log_eventf(PBSEVENT_DEBUG2, PBS_EVENTCLASS_HOOK, LOG_INFO, last_phook->hook_name,
							"Failed to add to vnlp: %s=%s", VNATTR_HOOK_OFFLINE_VNODES, hook_buf);
				} else {
					/*
					 * this saves 'tvnl' in svr_vnl_action,
					 * and later freed upon server acking action
					 */
					(void)send_hook_vnl(tvnl);
					tvnl = NULL;
				}
				if (tvnl != NULL)
					vnl_free(tvnl);
			}
			mom_deljob(pjob);
			im_send_error(stream, pimcs, hook_errcode, hook_msg);
			return;
	}

	mom_hook_input_init(&hook_input);
	hook_input.pjob = pjob;

	mom_hook_output_init(&hook_output);
	hook_output.reject_errcode = &hook_errcode;
	hook_output.last_phook = &last_phook;
	hook_output.fail_action = &hook_fail_action;

	/*
	 * Call job_join_extra to do setup.
	 */
	if (job_join_extra != NULL) {
		rc = job_join_extra(pjob, np);
		if (rc != 0) {
			(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
			mom_deljob(pjob);
			im_send_error(stream, pimcs, rc, NULL);
			return;
		}
	}
#if defined(MOM_CPUSET) && !defined(IRIX6_CPUSET)
	if (new_cpuset(pjob) < 0) {
		(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
		mom_deljob(pjob);
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}
#endif /* MOM_CPUSET && !IRIX6_CPUSET */

	(void)job_save(pjob, SAVEJOB_FULL);
	(void)strcpy(namebuf, path_jobs);
	if (pjob->ji_qs.ji_fileprefix[0] != '\0')
		(void)strcat(namebuf, pjob->ji_qs.ji_fileprefix);
	else
		(void)strcat(namebuf, pjob->ji_qs.ji_jobid);
	(void)strcat(namebuf, JOB_TASKDIR_SUFFIX);

	if (mkdir(namebuf, 0700) == -1) {
		(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
		mom_deljob(pjob);
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}

	/*
	 * the following must appear before check_pwd() since the
	 * latter tries to read cred info
	 */
	if (mom_create_cred(pjob, pimjs->im_cred, pimjs->im_credlen, FALSE, stream) == -1) {
		(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
		mom_deljob(pjob);
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}

	if (check_pwd(pjob) == NULL) {
		// FIXME: move below log event in check_pwd
		log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_NOTICE, pjob->ji_qs.ji_jobid, log_buffer);
		(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
		mom_deljob(pjob);
		im_send_error(stream, pimcs, PBSE_BADUSER, NULL);
		return;
	}
	pjob->ji_qs.ji_un.ji_momt.ji_exuid = pjob->ji_grpcache->gc_uid;
	pjob->ji_qs.ji_un.ji_momt.ji_exgid = pjob->ji_grpcache->gc_gid;

	/*
	 * create staging and execution dir if sandbox=PRIVATE mode is enabled
	 * this code should appear after check_pwd() since
	 * mkjobdir() depends on job uid and gid being set correctly
	 */
	if (pjob->ji_wattr[(int)JOB_ATR_sandbox].at_flags & ATR_VFLAG_SET && strcasecmp(pjob->ji_wattr[JOB_ATR_sandbox].at_val.at_str, "PRIVATE") == 0) {
		char *jdir = jobdirname(pjob->ji_qs.ji_jobid, pjob->ji_grpcache->gc_homedir);
		int e = 0;
#ifdef WIN32
		e = mkjobdir(pjob->ji_qs.ji_jobid, jdir,
				pjob->ji_user != NULL ? pjob->ji_user->pw_name : NULL,
				pjob->ji_user != NULL ? pjob->ji_user->pw_userlogin : INVALID_HANDLE_VALUE);
#else /* Unix/Linux */
		mode_t myumask = 0;

		if (pjob->ji_wattr[(int)JOB_ATR_umask].at_flags & ATR_VFLAG_SET) {
			char maskbuf[22];
			mode_t j;
			sprintf(maskbuf, "%ld", pjob->ji_wattr[(int)JOB_ATR_umask].at_val.at_long);
			sscanf(maskbuf, "%o", &j);
			myumask = umask(j);
		} else {
			myumask = umask(077);
		}

		e = mkjobdir(pjob->ji_qs.ji_jobid, jdir, pjob->ji_qs.ji_un.ji_momt.ji_exuid, pjob->ji_qs.ji_un.ji_momt.ji_exgid);
		if (myumask != 0)
			(void)umask(myumask);
#endif
		if (e != 0) {
			log_errf(errno, __func__, "unable to create the job directory %s", jdir);
			(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
			mom_deljob(pjob);
			im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
			return;
		}
	}

	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "JOIN_JOB as node %d", pjob->ji_nodeid);

	/*
	 * if certain resource limits require that the job usage be
	 * polled, we link the job to mom_polljobs.
	 *
	 * NOTE: we overload the job field ji_jobque for this as it
	 * is not used otherwise by MOM
	 */
	if (mom_do_poll(pjob))
		append_link(&mom_polljobs, &pjob->ji_jobque, pjob);
	append_link(&svr_alljobs, &pjob->ji_alljobs, pjob);

	/*
	 * At this point, we have done all the job setup.
	 * Any error from now on is a problem sending the
	 * reply to MS.  We don't need to call SEND_ERR.
	 */
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE)
		goto join_err;
	/*
	 * Here we need to call job_join_ack to send any extra
	 * information with the reply to the JOIN request.
	 * The format of the data sent by job_join_ack will
	 * always have a version number as the first item
	 * sent as an int.  The rest depends on job_join_ack
	 * and will be defined in the function it points to.
	 */
	if (job_join_ack != NULL) {
		ret = job_join_ack(pjob, np, stream);
		if (ret != PBSE_NONE)
			goto join_err;
	}

	if (tpp_eom(stream) == -1)
		goto join_err;

	if (dis_flush(stream) == -1)
		goto join_err;

	return;

join_err:
	log_err(errno, __func__, "Failed to reply JOIN_JOB");
	(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
	tpp_close(stream);
	mom_deljob(pjob);
}

void
im_req_kill_job(int stream, job *pjob, im_common_t *pimcs)
{
	int hook_errcode = 0;
	int hook_rc = 0;
	hook *last_phook = NULL;
	unsigned int hook_fail_action = 0;
	char hook_msg[HOOK_MSG_SIZE + 1];
	mom_hook_input_t hook_input;
	mom_hook_output_t hook_output;

	if (check_ms(stream, pjob))
		return;

	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "KILL_JOB received");

	mom_hook_input_init(&hook_input);
	hook_input.pjob = pjob;

	mom_hook_output_init(&hook_output);
	hook_output.reject_errcode = &hook_errcode;
	hook_output.last_phook = &last_phook;
	hook_output.fail_action = &hook_fail_action;
	if (mom_process_hooks(HOOK_EVENT_EXECJOB_PRETERM, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1) == 0) {
		im_send_error(stream, pimcs, hook_errcode, hook_msg);
		return;
	}

	/*
	 * Send the jobs a signal but we have to wait to
	 * do a reply to mother superior until the procs
	 * die and are reaped
	 *
	 * see scan_for_exiting()
	 */
	DBPRT(("%s: KILL_JOB %s\n", __func__, pimcs->im_jobid))
	kill_job(pjob, SIGKILL);
	pjob->ji_qs.ji_substate = JOB_SUBSTATE_EXITING;
	pjob->ji_qs.ji_state = JOB_STATE_EXITING;
	pjob->ji_obit = pimcs->im_event;
	exiting_tasks = 1;

	mom_hook_input_init(&hook_input);
	hook_input.pjob = pjob;

	mom_hook_output_init(&hook_output);
	hook_output.reject_errcode = &hook_errcode;
	hook_output.last_phook = &last_phook;
	hook_output.fail_action = &hook_fail_action;

	(void)mom_process_hooks(HOOK_EVENT_EXECJOB_EPILOGUE, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
}

void
im_req_exec_prologue(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;
	int hook_errcode = 0;
	int hook_rc = 0;
	hook *last_phook = NULL;
	unsigned int hook_fail_action = 0;
	char hook_msg[HOOK_MSG_SIZE + 1];
	mom_hook_input_t hook_input;
	mom_hook_output_t hook_output;

	DBPRT(("%s: %s for %s\n", __func__, "IM_EXEC_PROLOGUE", pjob->ji_qs.ji_jobid));
	mom_hook_input_init(&hook_input);
	hook_input.pjob = pjob;

	mom_hook_output_init(&hook_output);
	hook_output.reject_errcode = &hook_errcode;
	hook_output.last_phook = &last_phook;
	hook_output.fail_action = &hook_fail_action;

	rc = mom_process_hooks(HOOK_EVENT_EXECJOB_PROLOGUE, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
	switch (rc) {

		case 1: /* explicit accept */
		case 2:	/* no hook script executed - go ahead and accept event */
			rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
			if (rc != PBSE_NONE) {
				im_eof(stream, rc);
				return;
			}
			break;
		default:
			/* a value of '0' means explicit reject encountered. */
			if (hook_rc != 0) {
				/*
				 * we've hit an internal error (malloc error, full disk, etc...), so
				 * treat this now like a hook error so hook fail_action
				 * will be consulted.
				 * Before, behavior of an internal error was to ignore it!
				 */
				hook_errcode = PBSE_HOOKERROR;
			}
			// FIXME: we should flush in send_error here
			im_send_error(stream, pimcs, hook_errcode, hook_msg);
			if (hook_errcode == PBSE_HOOKERROR) {
				send_hook_fail_action(last_phook);
			}
	}
}

void
im_req_spawn_task(int stream, job *pjob, im_common_t *pimcs, im_spawn_task_t *pimsts)
{
	int rc = 0;
	hnodent *np = NULL;
	pbs_task *ptask = NULL;

	DBPRT(("%s: SPAWN_TASK %s parent %d target %d taskid %u\n", __func__, pimcs->im_jobid, pimsts->im_pvnodeid, pimsts->im_tvnodeid, pimsts->im_taskid))

	if ((np = find_node(pjob, stream, pimsts->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}

	/* The target node must be here */
	if (pjob->ji_nodeid != TO_PHYNODE(pimsts->im_tvnodeid)) {
		im_send_error(stream, pimcs, PBSE_INTERNAL, NULL);
		return;
	}
#ifdef PMIX
	pbs_pmix_register_client(pjob, pimsts->im_tvnodeid, &(pimsts->im_envp));
#endif
	if ((ptask = momtask_create(pjob)) == NULL) {
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}
	strcpy(ptask->ti_qs.ti_parentjobid, pimcs->im_jobid);
	ptask->ti_qs.ti_parentnode = pimsts->im_pvnodeid;
	ptask->ti_qs.ti_myvnode = pimsts->im_tvnodeid;
	ptask->ti_qs.ti_parenttask = pimcs->im_fromtask;
	if (task_save(ptask) == -1) {
		im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
		return;
	}
	rc = start_process(ptask, pimsts->im_argv, pimsts->im_envp, false);
	if (rc != PBSE_NONE) {
		im_send_error(stream, pimcs, rc, NULL);
	} else {
		rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
		if (rc == PBSE_NONE) {
			rc = diswui(stream, ptask->ti_qs.ti_task);
		}
	}
}

void
im_req_get_tasks(int stream, job *pjob, im_common_t *pimcs, im_get_tasks_t *pimgts)
{
	int rc = 0;
	hnodent *np = NULL;
	pbs_task *ptask = NULL;

	DBPRT(("%s: GET_TASKS %s from node %d to node %d\n", __func__, pimcs->im_jobid, pimgts->im_pvnodeid, pimgts->im_tvnodeid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "GET_TASKS received");

	if ((np = find_node(pjob, stream, pimgts->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
	ptask = (pbs_task *)GET_NEXT(pjob->ji_tasks);
	for (; ptask != NULL; ptask = (pbs_task *)GET_NEXT(ptask->ti_jobtask)) {
		if (ptask->ti_qs.ti_myvnode == pimgts->im_tvnodeid) {
			diswui(stream, ptask->ti_qs.ti_task);
		}
	}
}

void
im_req_signal_task(int stream, job *pjob, im_common_t *pimcs, im_sig_task_t *pimsts)
{
	int rc = 0;
	hnodent *np = NULL;
	pbs_task *ptask = NULL;

	DBPRT(("%s: SIGNAL_TASK %s from node %d task %8.8X sig %d\n", __func__, pimcs->im_jobid, pimsts->im_pvnodeid, pimsts->im_tvnodeid, pimsts->im_signum))
	log_eventf(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "SIGNAL_TASK %8.8X sig %d", pimsts->im_taskid, pimsts->im_signum);
	if ((np = find_node(pjob, stream, pimgts->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}
	ptask = task_find(pjob, pimsts->im_taskid);
	if (ptask == NULL) {
		im_send_error(stream, pimcs, PBSE_JOBEXIST, NULL);
		return;
	}
	kill_task(ptask, pimsts->im_signum, 0);
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_req_obit_task(int stream, job *pjob, im_common_t *pimcs, im_obit_task_t *pimots)
{
	int rc = 0;
	hnodent *np = NULL;
	pbs_task *ptask = NULL;

	DBPRT(("%s: OBIT_TASK %s from node %d to task %8.8X\n", __func__, pimcs->im_jobid, pimots->im_pvnodeid, pimots->im_taskid))
	log_eventf(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "SIGNAL_TASK %8.8X sig %d", pimsts->im_taskid, pimsts->im_signum);
	if ((np = find_node(pjob, stream, pimots->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}
	ptask = task_find(pjob, pimots->im_taskid);
	if (ptask == NULL) {
		im_send_error(stream, pimcs, PBSE_JOBEXIST, NULL);
		return;
	}

	if (ptask->ti_qs.ti_status >= TI_STATE_EXITED) {
		rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
		if (rc != PBSE_NONE) // FIXME: what should we do here
			return;
		diswsi(stream, ptask->ti_qs.ti_exitstat);
	}
	else {
		/* save obit request with task */
		obitent *op = (obitent *)malloc(sizeof(obitent));
		if (op != NULL) {
			im_send_error(stream, pimcs, PBSE_SYSTEM, NULL);
			return;
		}
		CLEAR_LINK(op->oe_next);
		append_link(&ptask->ti_obits, &op->oe_next, op);
		op->oe_type = OBIT_TYPE_TMEVENT;
		op->oe_u.oe_tm.oe_fd = -1;
		op->oe_u.oe_tm.oe_node = pimots->im_pvnodeid;
		op->oe_u.oe_tm.oe_event = pimcs->im_event;
		op->oe_u.oe_tm.oe_taskid = pimcs->im_fromtask;
		/* No reply here, see scan_for_exiting() */
	}
}

void
im_req_get_info(int stream, job *pjob, im_common_t *pimcs, im_get_info_t *pimgis)
{
	int rc = 0;
	hnodent *np = NULL;
	pbs_task *ptask = NULL;
	infoent *ip = NULL;

	DBPRT(("%s: GET_INFO %s from node %d task %8.8X name %s\n", __func__, pimcs->im_jobid, pimgis->im_pvnodeid, pimgis->im_taskid, pimgis->im_name))
	log_eventf(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "GET_INFO task %8.8X name %s", pimgis->im_taskid, pimgis->im_name);
	if ((np = find_node(pjob, stream, pimgts->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}
	ptask = task_find(pjob, pimgis->im_taskid);
	if (ptask == NULL) {
		im_send_error(stream, pimcs, PBSE_JOBEXIST, NULL);
		return;
	}

	if ((ip = task_findinfo(ptask, pimgis->im_name)) == NULL) {
		im_send_error(stream, pimcs, PBSE_JOBEXIST, NULL);
		return;
	}
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
	diswcs(stream, ip->ie_info, ip->ie_len);
}

void
im_req_get_resc(int stream, job *pjob, im_common_t *pimcs, im_get_resc_t *pimgrs)
{
	int rc = 0;
	hnodent *np = NULL;
	char *info = NULL;

	DBPRT(("%s: GET_RESC %s from node %d\n", __func__, pmics->im_jobid, pimgrs->im_pvnodeid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pmics->im_jobid, "GET_RESC received");
	if ((np = find_node(pjob, stream, pimgrs->im_pvnodeid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_BADHOST, NULL);
		return;
	}
	info = resc_string(pjob);
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
	diswst(stream, info);
	free(info);
}

void
im_req_poll_job(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;
	int exitval = 0;

	if (QA_testing != 0) {
		/* for QA Testing only */
		if (QA_testing & PBSQA_POLLJOB_CRASH)
			exit(98);
		else if (QA_testing & PBSQA_POLLJOB_SLEEP)
			sleep(90);
	}
	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: POLL_JOB %s\n", __func__, pimcs->im_jobid))
	pjob->ji_polltime = time_now;
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
	/* Now comes a recomendation for killing the job */
	exitval = (pjob->ji_qs.ji_svrflags & (JOB_SVFLG_OVERLMT1 | JOB_SVFLG_OVERLMT2)) ? 1 : 0;
	diswsi(stream, exitval);
	/* Send the information tallyed for the job. */
	diswul(stream, resc_used(pjob, "cput", gettime));
	diswul(stream, resc_used(pjob, "mem", getsize));
	diswul(stream, resc_used(pjob, "cpupercent", gettime));
	// FIXME: change below func to pass pjob
	send_resc_used_to_ms(stream, pimcs->im_jobid);
}

void
im_req_suspend_resume(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;

	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: %s %s\n", __func__, (pimcs->im_command == IM_SUSPEND) ? "SUSPEND" : "RESUME", pimcs->im_jobid))
	log_eventf(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "%s received", (pimcs->im_command == IM_SUSPEND) ? "SUSPEND" : "RESUME");
	if ((rc = local_supres(pjob, (pimcs->im_command == IM_SUSPEND) ? 1 : 0, NULL)) != PBSE_NONE) {
		im_send_error(stream, pimcs, rc);
		return;
	}
	pjob->ji_mompost = (pimcs->im_command == IM_SUSPEND) ? post_suspend : post_resume;
	/*
	 * If a child was started to handle the operation,
	 * wait to reply until the kid returns.
	 */
	if (pjob->ji_momsubt) {
		pjob->ji_postevent = pimcs->im_event;
		pjob->ji_taskid = pimcs->im_fromtask;
		return;
	}

	pjob->ji_mompost(pjob, PBSE_NONE);
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_req_restart(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;
	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: RESTART %s\n", __func__, pimcs->im_jobid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "RESTART received");

	if ((rc = local_restart(pjob, NULL)) != PBSE_NONE) {
		im_send_error(stream, pimcs, rc);
		return;
	}

	/*
	 * If a child was started to handle the operation,
	 * wait to reply until the kid returns.
	 */
	if (pjob->ji_momsubt) {
		pjob->ji_postevent = pimcs->im_event;
		pjob->ji_taskid = pimcs->im_fromtask;
		return;
	}

	post_restart(pjob, PBSE_NONE);
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_req_checkpoint(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;

	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: %s %s\n", __func__, (pimcs->im_command == IM_CHECKPOINT) ? "CHECKPOINT" : "CHECKPOINT_ABORT", pimcs->im_jobid))
	log_eventf(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "%s received", (pimcs->im_command == IM_CHECKPOINT) ? "CHECKPOINT" : "CHECKPOINT_ABORT");
	if ((rc = local_checkpoint(pjob, (pimcs->im_command == IM_CHECKPOINT) ? 1 : 0, NULL)) != PBSE_NONE) {
		im_send_error(stream, pimcs, rc);
		return;
	}

	/*
	 * If a child was started to handle the operation,
	 * wait to reply until the kid returns.
	 */
	if (pjob->ji_momsubt) {
		pjob->ji_postevent = pimcs->im_event;
		pjob->ji_taskid = pimcs->im_fromtask;
		return;
	}

	post_chkpt(pjob, PBSE_NONE);
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_req_abort(int stream, job *pjob, im_common_t *pimcs)
{
	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: ABORT_JOB %s\n", __func__, pimcs->im_jobid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "ABORT_JOB received");
	if (pjob->ji_qs.ji_svrflags & (JOB_SVFLG_CHKPT | JOB_SVFLG_ChkptMig)) {
		kill_job(pjob, SIGKILL); /* is this right? */
	} else {
		int hook_errcode = 0;
		hook *last_phook = NULL;
		unsigned int hook_fail_action = 0;
		char hook_msg[HOOK_MSG_SIZE + 1];
		mom_hook_input_t hook_input;
		mom_hook_output_t hook_output;

		mom_hook_input_init(&hook_input);
		hook_input.pjob = pjob;

		mom_hook_output_init(&hook_output);
		hook_output.reject_errcode = &hook_errcode;
		hook_output.last_phook = &last_phook;
		hook_output.fail_action = &hook_fail_action;
		(void)mom_process_hooks(HOOK_EVENT_EXECJOB_ABORT, PBS_MOM_SERVICE_NAME, mom_host, &hook_input, &hook_output, hook_msg, sizeof(hook_msg), 1);
		mom_deljob(pjob);
	}
}

void
im_req_requeue(int stream, job *pjob, im_common_t *pimcs)
{
	DBPRT(("%s: IM_REQUEUE job %s\n", __func__, pimcs->im_jobid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "REQUEUE received");
	if ((pjob->ji_qs.ji_svrflags & JOB_SVFLG_HERE) != 0) {
		pjob->ji_qs.ji_un.ji_momt.ji_exitstat = JOB_EXEC_RERUN;
		(void)kill_job(pjob, SIGKILL);
		/* Server will decide if job is rerunnable or not */
	}
}

void
im_req_setup_job(int stream, job *pjob, im_common_t *pimcs)
{
	int rc = 0;

	if (check_ms(stream, pjob))
		return;
	DBPRT(("%s: SETUP_JOB %s\n", __func__, pimcs->im_jobid))
	log_event(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, LOG_DEBUG, pimcs->im_jobid, "SETUP_JOB received");

	/*
	 * If there is a job_setup_final, call it,
	 * otherwise, send the "not supported" error.
	 */
	if (job_setup_final == NULL) {
		im_send_error(stream, pimcs, PBSE_NOSUP);
		return;
	}

	rc = job_setup_final(pjob, stream);
	if (rc != PBSE_NONE) {
		im_send_error(stream, pimcs, rc);
		return;
	}
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_req_send_resc(int stream, job *pjob, im_common_t *pimcs, im_send_resc_t *pimsrs)
{
	int i = 0;
	int resc_idx = -1;
	char timebuf[TIMEBUF_SIZE] = {'\0'};

	if ((pjob->ji_qs.ji_svrflags & JOB_SVFLG_HERE) == 0) {
		log_joberr(-1, __func__, "got IM_SEND_RESC and I'm not MS", pimcs->im_jobid);
		im_eof(stream, 0);
		return;
	}

	for (i = 0; i < pjob->ji_numrescs; i++) {
		if (pjob->ji_resources[i].nodehost != NULL && compare_short_hostname(pjob->ji_resources[i].nodehost, pimsrs->im_node) == 0) {
			resc_idx = i;
			break;
		}
	}
	if (resc_idx == -1) {
		noderes *tmparr = NULL;
		/*
		 * add an entry to pjob->ji_resources
		 * for this incoming resource report
		 */

		tmparr = (noderes *)realloc(pjob->ji_resources, (pjob->ji_numrescs + 1) * sizeof(noderes));
		if (tmparr == NULL) {
			log_joberr(-1, __func__, "realloc failure for extending pjob->ji_resources", pimcs->im_jobid);
			im_eof(stream, 0);
			return;
		}
		pjob->ji_resources = tmparr;
		resc_idx = pjob->ji_numrescs;
		pjob->ji_resources[resc_idx].nodehost = strdup(pimsrs->im_node);
		if (pjob->ji_resources[resc_idx].nodehost == NULL) {
			log_joberr(-1, __func__, "strdup failure setting nodehost", pimcs->im_jobid);
			im_eof(stream, 0);
			return;
		}
		clear_attr(&(pjob->ji_resources[resc_idx].nr_used), &(job_attr_def[JOB_ATR_resc_used]));
		pjob->ji_numrescs++;

	}
	pjob->ji_resources[resc_idx].nr_cput = pimsrs->im_cput;
	convert_duration_to_str(pimsrs->im_cput, timebuf, sizeof(timebuf));
	pjob->ji_resources[resc_idx].nr_mem = pimsrs->im_mem;
	pjob->ji_resources[resc_idx].nr_cpupercent = pimsrs->im_cpupercent;
	pjob->ji_resources[resc_idx].nr_status = PBS_NODERES_DELETE;
	DBPRT(("%s: SEND_RESC %s OKAY rescidx %d cpu %lu mem %lu\n", __func__, pimcs->im_jobid, resc_idx, pimsrs->im_cput, pimsrs->im_mem))
	log_eventf(PBSEVENT_DEBUG2, PBS_EVENTCLASS_JOB, LOG_DEBUG, pjob->ji_qs.ji_jobid, "%s cput=%s mem=%lukb", pimsrs->im_node, timebuf, pimsrs->im_mem);
	update_ajob_status(pjob);
}

void
im_req_update_job(int stream, job *pjob, im_common_t *pimcs, im_update_job_t *pimujs)
{
	int rc = 0;

	if (check_ms(stream, NULL))
		return;
	// FIXME: move receive_job_update code here
	if (receive_job_update(stream, pjob) != 0) {
		log_event(PBSEVENT_DEBUG2, PBS_EVENTCLASS_JOB, LOG_DEBUG, pjob->ji_qs.ji_jobid, "receive_job_update failed");
		im_eof(stream, 0);
		return;
	}
	rc = im_compose(stream, pimcs, IM_ALL_OKAY, IM_OLD_PROTOCOL_VER);
	if (rc != PBSE_NONE) // FIXME: what should we do here
		return;
}

void
im_request(int stream, pbs_im_t *pims)
{
	struct sockaddr_in *addr = NULL;
	u_long ipaddr = 0;
	job *pjob = NULL;
	im_common_t *pimcs = &(pims->im_common);

	/* check that machine is known */
	addr = tpp_getaddr(stream);
	if (addr == NULL) {
		log_err(-1, __func__, "Sender unknown");
		tpp_close(stream);
		free_im(pims);
		return;
	}
	ipaddr = ntohl(addr->sin_addr.s_addr);
	DBPRT(("connect from %s\n", netaddr(addr)))
	if (!addrfind(ipaddr)) {
		log_errf(-1, __func__, "bad connect from %s", netaddr(addr));
		im_eof(stream, 0);
		free_im(pims);
		return;
	}

	if (pimcs->im_command == IM_JOIN_JOB) {
		im_req_join_job(stream, pimcs, &(pims->im_req->im_join));
		free_im(pims);
		return;
	} else if (pimcs->im_command == IM_ALL_OKAY ||
			pimcs->im_command == IM_ERROR ||
			pimcs->im_command == IM_ERROR2) {
		im_handle_reply(stream, pimcs, pims->im_reply);
		free_im(pims);
		return;
	}

	/* find job for which im request came */
	if ((pjob = find_job(pimcs->im_jobid)) == NULL) {
		im_send_error(stream, pimcs, PBSE_JOBEXIST, NULL);
		tpp_close(stream);
		free_im(pims);
		return;
	}

	/* check cookie */
	if (!(pjob->ji_wattr[(int)JOB_ATR_Cookie].at_flags & ATR_VFLAG_SET)) {
		DBPRT(("%s: job %s has no cookie\n", __func__, pimcs->im_jobid))
		im_send_error(stream, pimcs, PBSE_BADSTATE, NULL);
		tpp_close(stream);
		free_im(pims);
		return;
	}
	if (strcmp(pjob->ji_wattr[(int)JOB_ATR_Cookie].at_val.at_str, pimcs->im_cookie) != 0) {
		DBPRT(("%s: job %s cookie %s message %s\n", __func__, pimcs->im_jobid, pjob->ji_wattr[(int)JOB_ATR_Cookie].at_val.at_str, pimcs->im_cookie))
		im_send_error(stream, pimcs, PBSE_BADSTATE, NULL);
		tpp_close(stream);
		free_im(pims);
		return;
	}

	switch (pimcs->im_command) {
		case IM_KILL_JOB:
			im_req_kill_job(stream, pjob, pimcs);
			break;

		case IM_EXEC_PROLOGUE:
			im_req_exec_prologue(stream, pjob, pimcs);
			break;

		case IM_SPAWN_TASK:
			im_req_spawn_task(stream, pjob, pimcs, &(pims->im_req->im_spawn));
			break;

		case IM_GET_TASKS:
			im_req_get_tasks(stream, pjob, pimcs, &(pims->im_req->im_tasks));
			break;

		case IM_SIGNAL_TASK:
			im_req_signal_task(stream, pjob, pimcs, &(pims->im_req->im_sig));
			break;

		case IM_OBIT_TASK:
			im_req_obit_task(stream, pjob, pimcs, &(pims->im_req->im_obit));
			break;

		case IM_GET_INFO:
			im_req_get_info(stream, pjob, pimcs, &(pims->im_req->im_info));
			break;

		case IM_GET_RESC:
			im_req_get_resc(stream, pjob, pimcs, &(pims->im_req->im_gresc));
			break;

		case IM_POLL_JOB:
			im_req_poll_job(stream, pjob, pimcs);
			break;

		case IM_SUSPEND:
		case IM_RESUME:
			im_req_suspend_resume(stream, pjob, pimcs);
			break;

		case IM_RESTART:
			im_req_restart(stream, pjob, pimcs);
			break;

		case IM_CHECKPOINT:
		case IM_CHECKPOINT_ABORT:
			im_req_checkpoint(stream, pjob, pimcs);
			break;

		case IM_ABORT_JOB:
			im_req_abort(stream, pjob, pimcs);
			break;

		case IM_REQUEUE:
			im_req_requeue(stream, pjob, pimcs);
			break;

		case IM_SEND_RESC:
			im_req_send_resc(stream, pjob, pimcs, &(pims->im_req->im_sresc));
			break;

		case IM_UPDATE_JOB:
			im_req_update_job(stream, pjob, pimcs &(pims->im_req->im_update));
			break;

		// case IM_PMIX:
		// case IM_CRED:

		default:
			// FIXME: check all log_event in above calls
			log_joberrf(-1, __func__, pimcs->im_jobid, "unknown command %d sent", pimcs->im_command);
			im_eof(stream, 0);
			break;
	}

	free_im(pims);
}
