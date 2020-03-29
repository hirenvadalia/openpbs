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
#include "pbs_imr.h"

static int
__wire_im_read_reply(ns(InterMoM_table_t) req, im_reply_t *pimrs)
{
	int rc = PBSE_NONE;
	int type = ns(InterMoM_reply_type(req));

	switch (type) {

		case ns(IMReply_IMRErr):
			ns(IMRErr_table_t) B = (ns(IMRErr_table_t)) ns(InterMoM_reply(req));

			pimrs->imr_err.imr_errcode = (int) ns(IMRErr_errcode(B));
			if (ns(IMRErr_errmsg_is_present(B)))
				COPYSTR_S(pimrs->imr_err.imr_errmsg, ns(IMRErr_errmsg(B)));

			break;

		case ns(IMReply_IMRKill):
			ns(IMRKill_table_t) B = (ns(IMRKill_table_t)) ns(InterMoM_reply(req));

			pimrs->imr_ok.imr_kill.imr_cput = (long) ns(IMRKill_cput(B));
			pimrs->imr_ok.imr_kill.imr_mem = (long) ns(IMRKill_mem(B));
			pimrs->imr_ok.imr_kill.imr_cpupercent = (long) ns(IMRKill_cpupercent(B));
			rc = wire_decode_svrattrl(ns(IMRKill_attrs(B)), &(pimrs->imr_ok.imr_kill.imr_used));

			break;

		case ns(IMReply_IMRSpawn):
			ns(IMRSpawn_table_t) B = (ns(IMRSpawn_table_t)) ns(InterMoM_reply(req));

			pimrs->imr_ok.imr_spawn.imr_taskid = (unsigned int) ns(IMRSpawn_taskId(B));

			break;

		case ns(IMReply_IMRTasks):
			ns(IMRTasks_table_t) B = (ns(IMRTasks_table_t)) ns(InterMoM_reply(req));
			flatbuffers_uint16_vec_t ids = ns(IMRTasks_taskIds(B));
			int i = 0;

			pimrs->imr_ok.imr_tasks.imr_count = flatbuffers_uint16_vec_len(ids);
			pimrs->imr_ok.imr_tasks.imr_taskids = malloc(sizeof(tm_task_id) * pimrs->imr_ok.imr_tasks.imr_count);
			if (pimrs->imr_ok.imr_tasks.imr_taskids == NULL)
				return PBSE_SYSTEM;

			for (i = 0; i < pimrs->imr_ok.imr_tasks.imr_count; i++) {
				*(pimrs->imr_ok.imr_tasks.imr_taskids + i) = flatbuffers_uint16_vec_at(ids, i);
			}

			break;

		case ns(IMReply_IMRObit):
			ns(IMRObit_table_t) B = (ns(IMRObit_table_t)) ns(InterMoM_reply(req));

			pimrs->imr_ok.imr_obit.imr_exitval = (int) ns(IMRObit_exitval(B));

			break;

		case ns(IMReply_IMRInfo):

			// FIXME: do this

			break;

		case ns(IMReply_IMRResc):
			ns(IMRResc_table_t) B = (ns(IMRResc_table_t)) ns(InterMoM_reply(req));

			COPYSTR_S(pimrs->imr_ok.imr_resc.imr_info, ns(IMRResc_info(B)));

			break;

		case ns(IMReply_IMRPoll):
			ns(IMRPoll_table_t) B = (ns(IMRPoll_table_t)) ns(InterMoM_reply(req));

			pimrs->imr_ok.imr_poll.imr_exitval = (long) ns(IMRPoll_exitval(B));
			pimrs->imr_ok.imr_poll.imr_cput = (long) ns(IMRPoll_cput(B));
			pimrs->imr_ok.imr_poll.imr_mem = (long) ns(IMRPoll_mem(B));
			pimrs->imr_ok.imr_poll.imr_cpupercent = (long) ns(IMRPoll_cpupercent(B));
			rc = wire_decode_svrattrl(ns(IMRPoll_attrs(B)), &(pimrs->imr_ok.imr_poll.imr_used));

			break;

		default:
			log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_REQUEST, LOG_ERR, __func__, "IM Reply Body bad, type %d", (int) type);
			rc = PBSE_PROTOCOL;
			break;
	}

	return rc;
}

static int
__wire_im_read_req(ns(InterMoM_table_t) req, im_req_t *pimrs)
{
	int rc = PBSE_NONE;
	int type = ns(InterMoM_req_type(req));

	switch (type) {

		case ns(IMReq_IMJoin):
			ns(IMJoin_table_t) B = (ns(IMJoin_table_t)) ns(InterMoM_reply(req));

			pimrs->im_join.im_nodenum = (int) ns(IMJoin_nodeNum(B));
			pimrs->im_join.im_stdout = (int) ns(IMJoin_stdOut(B));
			pimrs->im_join.im_stderr = (int) ns(IMJoin_stdErr(B));
			pimrs->im_join.im_credtype = (int) ns(IMJoin_credType(B));
			pimrs->im_join.im_credlen = (size_t) ns(IMJoin_credLen(B));
			COPYSTR_M(pimrs->im_join.im_cred, ns(IMJoin_cred(B)), pimrs->im_join.im_credlen);
			rc = wire_decode_svrattrl(ns(IMJoin_attrs(B)), &(pimrs->im_join.im_jattrs));

			break;

		case ns(IMReq_IMSpawn):
			ns(IMSpawn_table_t) B = (ns(IMSpawn_table_t)) ns(InterMoM_reply(req));
			int i = 0;
			size_t len = 0;
			flatbuffers_string_vec_t strs;

			pimrs->im_spawn.im_pvnodeid = (tm_node_id) ns(IMSpawn_pvnodeId(B));
			pimrs->im_spawn.im_tvnodeid = (tm_node_id) ns(IMSpawn_tvnodeId(B));
			pimrs->im_spawn.im_taskid = (tm_task_id) ns(IMSpawn_taskId(B));
			pimrs->im_spawn.im_argv = NULL;
			pimrs->im_spawn.im_envp = NULL;

			strs = ns(IMSpawn_argv(B));
			len = flatbuffers_string_vec_len(strs);
			pimrs->im_spawn.im_argv = (char **)calloc(sizeof(char **), len + 1);
			if (pimrs->im_spawn.im_argv == NULL)
				return PBSE_SYSTEM;
			for (i = 0; i < len; i++) {
				COPYSTR_S(pimrs->im_spawn.im_argv[i], flatbuffers_string_vec_at(strs, i));
			}

			strs = ns(IMSpawn_envp(B));
			len = flatbuffers_string_vec_len(strs);
			pimrs->im_spawn.im_envp = (char **)calloc(sizeof(char **), len + 1);
			if (pimrs->im_spawn.im_envp == NULL)
				return PBSE_SYSTEM;
			for (i = 0; i < len; i++) {
				COPYSTR_S(pimrs->im_spawn.im_envp[i], flatbuffers_string_vec_at(strs, i));
			}

			break;

		case ns(IMReq_IMTasks):
			ns(IMTasks_table_t) B = (ns(IMTasks_table_t)) ns(InterMoM_reply(req));

			pimrs->im_tasks.im_pvnodeid = (tm_node_id) ns(IMTasks_pvnodeId(B));
			pimrs->im_tasks.im_tvnodeid = (tm_node_id) ns(IMTasks_tvnodeId(B));

			break;

		case ns(IMReq_IMSignal):
			ns(IMSignal_table_t) B = (ns(IMSignal_table_t)) ns(InterMoM_reply(req));

			pimrs->im_sig.im_pvnodeid = (tm_node_id) ns(IMSignal_pvnodeId(B));
			pimrs->im_sig.im_taskid = (tm_task_id) ns(IMSignal_taskId(B));
			pimrs->im_sig.im_signum = (int) ns(IMSignal_sigNum(B));

			break;

		case ns(IMReq_IMObit):
			ns(IMObit_table_t) B = (ns(IMObit_table_t)) ns(InterMoM_reply(req));

			pimrs->im_obit.im_pvnodeid = (tm_node_id) ns(IMObit_pvnodeId(B));
			pimrs->im_obit.im_taskid = (tm_task_id) ns(IMObit_taskId(B));

			break;

		case ns(IMReq_IMInfo):
			ns(IMInfo_table_t) B = (ns(IMInfo_table_t)) ns(InterMoM_reply(req));

			pimrs->im_info.im_pvnodeid = (tm_node_id) ns(IMInfo_pvnodeId(B));
			pimrs->im_info.im_taskid = (tm_task_id) ns(IMInfo_taskId(B));
			COPYSTR_S(pimrs->im_info.im_name, ns(IMInfo_name(B)));

			break;

		case ns(IMReq_IMGResc):
			ns(IMGResc_table_t) B = (ns(IMGResc_table_t)) ns(InterMoM_reply(req));

			pimrs->im_gresc.im_pvnodeid = (tm_node_id) ns(IMGResc_pvnodeId(B));

			break;

		case ns(IMReq_IMSResc):
			ns(IMSResc_table_t) B = (ns(IMSResc_table_t)) ns(InterMoM_reply(req));

			pimrs->im_sresc.im_cput = (long) ns(IMSResc_cput(B));
			pimrs->im_sresc.im_mem = (long) ns(IMSResc_mem(B));
			pimrs->im_sresc.im_cpupercent = (long) ns(IMSResc_cpupercent(B));
			COPYSTR_S(pimrs->im_sresc.im_node, ns(IMSResc_node(B)));

			break;

		case ns(IMReq_IMUpdate):
			ns(IMUpdate_table_t) B = (ns(IMUpdate_table_t)) ns(InterMoM_reply(req));

			rc = wire_decode_svrattrl(ns(IMUpdate_attrs(B)), &(pimrs->im_update.im_jattrs));

			break;

		case ns(IMReq_IMCred):
			ns(IMCred_table_t) B = (ns(IMCred_table_t)) ns(InterMoM_reply(req));

			pimrs->im_cred.im_type = (int) ns(IMCred_type(B));
			pimrs->im_cred.validity = (long) ns(IMCred_validity(B));
			COPYSTR_S(pimrs->im_cred.im_cred, ns(IMCred_cred(B)));

			break;

		default:
			log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_REQUEST, LOG_ERR, __func__, "IM Req Body bad, type %d", (int) type);
			rc = PBSE_PROTOCOL;
			break;
	}

	return rc;
}

int
wire_im_read(int sock, pbs_im_t *pims)
{
	int rc = 0;
	void *buf = NULL;
	size_t bufsz = 0;
	ns(InterMoM_table_t) req;

	//FIXME: get loaded buf for read with size here
	rc = ns(InterMoM_verify_as_root(buf, bufsz));
	if (rc != 0) {
		log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
				"Bad IM, errno %d, wire error %d", errno, rc);
		return PBSE_PROTOCOL;
	}

	req = ns(InterMoM_as_root(buf));
	COPYSTR_S(pims->im_jobid, ns(InterMom_jobId(req)));
	COPYSTR_S(pims->im_cookie, ns(InterMom_cookie(req)));
	pims->im_command = (int) ns(InterMom_cmd(req));
	pims->im_event = (tm_event_t) ns(InterMom_event(req));
	pims->im_fromtask = (tm_task_id) ns(InterMom_fromTask(req));

	switch (pims->im_command) {
		case IM_ALL_OKAY:
		case IM_ERROR:
		case IM_ERROR2:
			/* this is reply */
			// FIXME: alloc im_reply here
			return __wire_im_read_reply(req, pims->im_reply);
			break;

		default:
			/* this is request */
			// FIXME: alloc im_req here
			return __wire_im_read_req(req, pims->im_req);
			break;
	}

	return PBSE_NONE;
}
