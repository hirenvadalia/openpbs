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

#include <pbs_config.h> /* the master config generated by configure */
#include <errno.h>
#include "pbs_ifl.h"
#include "libpbs.h"
#include "log.h"
#include "tm_.h"

typedef struct im_join_job {
	int im_nodenum;
	int im_stdout;
	int im_stderr;
	pbs_list_head im_jattrs;
	int im_credtype;
	char *im_cred;
	size_t im_credlen;
} im_join_job_t;

typedef struct im_spawn_task {
	tm_node_id im_pvnodeid;
	tm_node_id im_tvnodeid;
	tm_task_id im_taskid;
	char **im_argv;
	char **im_envp;
} im_spawn_task_t;

typedef struct im_get_tasks {
	tm_node_id im_pvnodeid;
	tm_node_id im_tvnodeid;
} im_get_tasks_t;

typedef struct im_sig_task {
	tm_node_id im_pvnodeid;
	tm_task_id im_taskid;
	int im_signum;
} im_sig_task_t;

typedef struct im_obit_task {
	tm_node_id im_pvnodeid;
	tm_task_id im_taskid;
} im_obit_task_t;

typedef struct im_get_info {
	tm_node_id im_pvnodeid;
	tm_task_id im_taskid;
	char *im_name;
} im_get_info_t;

typedef struct im_get_resc {
	tm_node_id im_pvnodeid;
} im_get_resc_t;

typedef struct im_cred {
	int im_type;
	char *im_cred;
	long validity;
} im_cred_t;

typedef struct im_send_resc {
	char *im_node;
	long im_cput;
	long im_mem;
	long im_cpupercent;
} im_send_resc_t;

typedef struct im_update_job {
	pbs_list_head im_jattrs;
} im_update_job_t;

typedef union im_req {
	im_join_job_t im_join; /* IM_JOIN_JOB */
	/* no info for IM_KILL_JOB */
	im_spawn_task_t im_spawn; /* IM_SPAWN_TASK */
	im_get_tasks_t im_tasks; /* IM_GET_TASKS */
	im_sig_task_t im_sig; /* IM_SIGNAL_TASK */
	im_obit_task_t im_obit; /* IM_OBIT_TASK */
	/* no info for IM_POLL_JOB */
	im_get_info_t im_info; /* IM_GET_INFO */
	im_get_resc_t im_gresc; /* IM_GET_RESC */
	/* no info for IM_ABORT_JOB */
	/* no info for IM_SUSPEND */
	/* no info for IM_RESUME */
	/* no info for IM_CHECKPOINT */
	/* no info for IM_CHECKPOINT_ABORT */
	/* no info for IM_RESTART */
	/* no info for IM_DELETE_JOB */
	/* no info for IM_REQUEUE */
	/* no info for IM_DELETE_JOB_REPLY */
	// FIXME: unknown IM_SETUP_JOB
	/* no info for IM_DELETE_JOB2 */
	im_send_resc_t im_sresc; /* IM_SEND_RESC */
	im_update_job_t im_update; /* IM_UPDATE_JOB */
	/* no info for IM_EXEC_PROLOGUE */
	// FIXME: add IM_PMIX
	im_cred_t im_cred; /* IM_CRED */
} im_req_t;

typedef struct imr_kill_job {
	long imr_cput;
	long imr_mem;
	long imr_cpupercent;
	pbs_list_head imr_used;
} imr_kill_job_t;

typedef struct imr_spawn_task {
	tm_task_id imr_taskid;
} imr_spawn_task_t;

typedef struct imr_get_tasks {
	int imr_count;
	tm_task_id *imr_taskid; /* list */
} imr_get_tasks_t;

typedef struct imr_obit_task {
	int imr_exitval;
} imr_obit_task_t;

typedef struct imr_get_info {
	int imr_len;
	void *imr_info; // FIXME: not sure about this
} imr_get_info_t;

typedef struct imr_get_resc {
	char *imr_info;
} imr_get_resc_t;

typedef struct imr_poll_job {
	int imr_exitval;
	long imr_cput;
	long imr_mem;
	long imr_cpupercent;
	pbs_list_head imr_used;
} imr_poll_job_t;

typedef union imr_ok {
	// FIXME: unknown IM_JOIN_JOB
	/* no info for IM_SETUP_JOB */
	/* no info for IM_SUSPEND */
	/* no info for IM_RESUME */
	/* no info for IM_RESTART */
	/* no info for IM_CHECKPOINT */
	/* no info for IM_CHECKPOINT_ABORT */
	imr_kill_job_t imr_kill; /* IM_KILL_JOB */
	/* no info for IM_DELETE_JOB_REPLY */
	imr_spawn_task_t imr_spawn; /* IM_SPAWN_TASK */
	/* no info for IM_SIGNAL_TASK */
	imr_obit_task_t imr_obit; /* IM_OBIT_TASK */
	imr_get_info_t imr_info; /* IM_GET_INFO */
	imr_get_resc_t imr_resc; /* IM_GET_RESC */
	imr_poll_job_t imr_poll; /* IM_POLL_JOB */
	// FIXME: add IM_PMIX
	/* no info for IM_UPDATE_JOB */
	/* no info for IM_EXEC_PROLOGUE */
} imr_ok_t;

typedef struct imr_err {
	int imr_errcode;
	char *imr_errmsg;
} imr_err_t;

typedef union im_reply {
	imr_ok_t imr_ok;
	imr_err_t imr_err;
} im_reply_t;

typedef struct pbs_im {
	char *im_jobid;
	char *im_cookie;
	int im_command;
	tm_event_t im_event;
	tm_task_id im_fromtask;
	im_req_t im_req;
	im_reply_t im_reply;
} pbs_im_t;
