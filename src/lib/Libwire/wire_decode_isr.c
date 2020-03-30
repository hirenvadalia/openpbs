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
#include "pbs_isr.h"

#define COPYSTR_S_VNL(dest, src) \
do { \
	dest = strdup((char *)src); \
	if (dest == NULL) { \
		free_vnlp(vnlp); \
		return PBSE_SYSTEM; \
	} \
} while(0)

/**
 * @brief
 *	free_vnlp - free a vnl_t data structure
 *
 * @par Functionality:
 *	Note that this function is nearly identical to vnl_free(),
 *	with the exception of using the *_cur values to free partially
 *
 * @param[in]	vnlp - pointer to structure to free
 *
 * @return vnl_t *
 * @retval void
 *
 */
void
free_vnlp(vnl_t *vnlp)
{
	unsigned long i = 0;
	unsigned long j = 0;

	for (i = 0; i <= vnlp->vnl_cur; i++) {
		vnal_t *vnalp = VNL_NODENUM(vnlp, i);

		for (j = 0; j <= vnalp->vnal_cur; j++) {
			vna_t *vnap = VNAL_NODENUM(vnalp, j);
			free(vnap->vna_name);
			free(vnap->vna_val);
		}
		free(vnalp->vnal_list);
		free(vnalp->vnal_id);
	}
	free(vnlp->vnl_list);
	free(vnlp);
}

/**
 * @brief
 *	vn_decode_DIS_V4 - decode version 4 vnode information from Mom
 *
 * @par Functionality:
 *	See vn_decode_DIS() above, This is called from there to decode
 *	V4 information.
 *
 * @param[in]	fd  -     socket descriptor from which to read
 * @param[out]	rcp -     pointer to place to return error code if error.
 *
 * @return	vnl_t *
 * @retval	pointer to decoded vnode information which has been malloc-ed.
 * @retval	NULL on error, see rcp value
 *
 * @par Side Effects: None
 *
 * @par MT-safe: yes
 *
 */
static int
wire_decode_vnodes(ns(Vnodes_table_t) B, vnl_t *vnlp)
{
	unsigned long i = 0;
	ns(Vnode_vec_t) vnodes;

	if ((vnlp = calloc(1, sizeof(vnl_t))) == NULL) {
		return PBSE_SYSTEM;
	}

	vnlp->vnl_modtime = (time_t) ns(Vnodes_modetime(B));
	vnodes = ns(Vnodes_vnodes(B));
	vnlp->vnl_nelem = vnlp->vnl_used = (unsigned long) ns(Vnode_vec_len(vnodes));

	if ((vnlp->vnl_list = calloc(vnlp->vnl_used, sizeof(vnal_t))) == NULL) {
		free(vnlp);
		return PBSE_SYSTEM;
	}

	for (i = 0; i < vnlp->vnl_used; i++) {
		ns(Vnode_table_t) vnode = ns(Vnode_vec_at(vnodes, i));
		vnal_t *cur_vnal = VNL_NODENUM(vnlp, i);
		ns(VnodeAttr_vec_t) attrs = ns(Vnode_attrs(vnode));
		unsigned long j = 0;

		vnlp->vnl_cur = i;

		COPYSTR_S_VNL(cur_vnal->vnal_id, ns(Vnode_vnodeId(vnode)));
		cur_vnal->vnal_nelem = cur_vnal->vnal_used = (unsigned long) ns(VnodeAttr_vec_len(attrs));

		if ((cur_vnal->vnal_list = calloc(cur_vnal->vnal_used, sizeof(vna_t))) == NULL) {
			free_vnlp(vnlp);
			return PBSE_SYSTEM;
		}

		for (j = 0; j < cur_vnal->vnal_used; j++) {
			ns(VnodeAttr_table_t) vattr = ns(VnodeAttr_vec_at(attrs, i));
			vna_t *cur_vna = VNAL_NODENUM(cur_vnal, j);

			cur_vnal->vnal_cur = j;

			cur_vna->vna_type = (int) ns(VnodeAttr_type(vattr));
			cur_vna->vna_flag = (int) ns(VnodeAttr_flag(vattr));
			COPYSTR_S_VNL(cur_vna->vna_name, ns(VnodeAttr_name(vattr)));
			COPYSTR_S_VNL(cur_vna->vna_val, ns(VnodeAttr_value(vattr)));
		}
	}

	return PBSE_NONE;
}

int
__wire_is_read(ns(InterSvr_table_t) req, is_req_t *isreq)
{
	int i = 0;
	int rc = PBSE_NONE;
	int type = ns(InterSvr_req_type(req));

	switch (type) {

		case ns(ISReq_ISNull):
			ns(ISNull_table_t) B = (ns(ISNull_table_t)) ns(InterSvr_req(req));

			isreq->is_null.is_retry = ns(ISNull_retry(B));
			isreq->is_null.is_highwater = ns(ISNull_highwater(B));

			break;

		case ns(ISReq_ISUpdate):
			ns(ISUpdate_table_t) B = (ns(ISUpdate_table_t)) ns(InterSvr_req(req));

			isreq->is_update.is_state = (int) ns(ISUpdate_state(B));
			isreq->is_update.is_phycpu = (int) ns(ISUpdate_phyCpu(B));
			isreq->is_update.is_availcpu = (int) ns(ISUpdate_availCpu(B));
			isreq->is_update.is_phymem = (unsigned long long) ns(ISUpdate_phyMem(B));
			COPYSTR_S(isreq->is_update.is_arch, ns(ISUpdate_arch(B)));
			COPYSTR_S(isreq->is_update.is_version, ns(ISUpdate_version(B)));
			rc = wire_decode_vnodes(ns(ISUpdate_vnodes(B)), isreq->is_update.is_vnlp);
			if (rc != PBSE_NONE)
				isreq->is_update.is_vnlp = NULL;

			break;

		case ns(ISReq_ISRescUsed):
			ns(ISRescUsed_table_t) B = (ns(ISRescUsed_table_t)) ns(InterSvr_req(req));
			ns(RUsed_vec_t) rescs = ns(ISRescUsed_rescs(B));
			rused_t *ru = isreq->is_rused;

			for (i = 0; i < ns(RUsed_vec_len(rescs)); i++) {
				ns(RUsed_table_t) resc = ns(RUsed_vec_at(rescs, i));

				ru = (rused_t *)calloc(1, sizeof(rused_t));
				ru->ru_status = (int) ns(RUsed_status(resc));
				ru->ru_hop = (int) ns(RUsed_hope(resc));
				COPYSTR_S(ru->ru_pjobid, ns(RUsed_jobId(resc)));
				if (ns(RUsed_comment_is_present(resc)))
					COPYSTR_S(ru->ru_comment, ns(RUsed_comment(resc)));
				rc = wire_decode_svrattrl(ns(RUsed_attrs(resc)), ru->ru_attr);
				if (rc != PBSE_NONE) {
					CLEAR_HEAD(ru->ru_attr);
					return rc;
				}
				ru = ru->ru_next;
			}

			break;

		case ns(ISReq_ISObit):
			ns(ISObit_table_t) B = (ns(ISObit_table_t)) ns(InterSvr_req(req));

			COPYSTR_S(isreq->is_jobid, ns(ISObit_jobId(B)));

			break;

		case ns(ISReq_ISRestart):
			ns(ISRestart_table_t) B = (ns(ISRestart_table_t)) ns(InterSvr_req(req));

			isreq->is_port = (int) ns(ISRestart_port(B));

			break;

		case ns(ISReq_ISIdle):
			ns(ISIdle_table_t) B = (ns(ISIdle_table_t)) ns(InterSvr_req(req));

			isreq->is_idle.is_which = (int) ns(ISIdle_which(B));
			COPYSTR_S(isreq->is_idle.is_jobid, ns(ISIdle_jobId(B)));

			break;

		case ns(ISReq_ISHello):
			ns(ISHello_table_t) B = (ns(ISHello_table_t)) ns(InterSvr_req(req));
			ns(JInfo_vec_t) jobs = ns(ISHello_jobs(B));
			jinfo_t *ji = isreq->is_hello.is_jinfo;

			isreq->is_hello.is_opts = (int) ns(ISHello_opts(B));
			for (i = 0; i < ns(JInfo_vec_len(jobs)); i++) {
				ns(JInfo_table_t) job = ns(JInfo_vec_at(jobs, i));

				ji = (jinfo_t *) calloc(1, sizeof(jinfo_t));
				ji->substate = (int) ns(JInfo_subState(job));
				ji->runver = (long) ns(JInfo_runVer(job));
				COPYSTR_S(ji->jid, ns(JInfo_jobId(job)));
				COPYSTR_S(ji->exec_vnode, ns(JInfo_ececVnode(job)));

				ji = ji->next;
			}

			break;

		case ns(ISReq_ISDiscard):
			ns(ISDiscard_table_t) B = (ns(ISDiscard_table_t)) ns(InterSvr_req(req));

			isreq->is_discard.is_runver = (int) ns(ISDiscard_runVer(B));
			COPYSTR_S(isreq->is_discard.is_jobid, ns(ISDiscard_jobId(B)));

			break;

		case ns(ISReq_ISAddrs):
			ns(ISAddrs_table_t) B = (ns(ISAddrs_table_t)) ns(InterSvr_req(req));
			ns(Addr_vec_t) addrs = ns(ISAddrs_addrs(B));
			is_addr_info_t *ai = isreq->is_addrs;

			for (i = 0; i < ns(Addr_vec_len(addrs)); i++) {
				ns(Addr_table_t) addr = ns(Addr_vec_at(addrs, i));

				ai = (is_addr_info_t *)calloc(1, sizeof(is_addr_info_t));
				ai->is_addr = (u_long) ns(Addr_addr(addr));
				ai->is_depth = (u_long) ns(Addr_depth(addr));
				ai = ai->is_next;
			}

			break;

		case ns(ISReq_ISUpdateHook):
			ns(ISUpdateHook_table_t) B = (ns(ISUpdateHook_table_t)) ns(InterSvr_req(req));

			isreq->is_update_hook.is_seq = (unsigned long) ns(ISUpdateHook_seq(B));
			COPYSTR_S(isreq->is_update_hook.is_user, ns(ISUpdateHook_user(B)));
			rc = wire_decode_vnodes(ns(ISUpdate_vnodes(B)), isreq->is_update_hook.is_vnlp);
			if (rc != PBSE_NONE)
				isreq->is_update_hook.is_vnlp = NULL;

			break;

		case ns(ISReq_ISActions):
			ns(ISActions_table_t) B = (ns(ISActions_table_t)) ns(InterSvr_req(req));
			ns(Action_vec_t) actions = ns(ISActions_acations(B));
			is_hj_action_t *ha = isreq->is_actions;

			for (i = 0; i < ns(Action_vec_len(addrs)); i++) {
				ns(Action_table_t) action = ns(Action_vec_at(actions, i));

				ha = (is_hj_action_t *)calloc(1, sizeof(is_hj_action_t));
				ha->is_seq = (unsigned long) ns(Action_seq(action));
				ha->is_runver = (int) ns(Action_runVer(action));
				ha->is_action = (int) ns(Action_action(action));
				COPYSTR_S(ha->is_jid, ns(Action_jobId(action)));

				ha = ha->is_next;
			}

			break;

		case ns(ISReq_ISHAck):
			ns(ISHAck_table_t) B = (ns(ISHAck_table_t)) ns(InterSvr_req(req));

			isreq->is_hact_ack.is_act_seq = (unsigned long) ns(ISHAck_seq(B));
			isreq->is_hact_ack.is_act_type = (int) ns(ISHAck_type(B));

			break;

		case ns(ISReq_ISSchedRC):
			ns(ISSchedRC_table_t) B = (ns(ISSchedRC_table_t)) ns(InterSvr_req(req));

			COPYSTR_S(isreq->is_user, ns(ISSchedRC_user(B)));

			break;

		case ns(ISReq_ISChkSums):
			ns(ISChkSums_table_t) B = (ns(ISChkSums_table_t)) ns(InterSvr_req(req));
			ns(HookChkSum_vec_t) chksums = ns(ISChkSums_hChkSums(B));
			hook_chksums_t *hchk = isreq->is_hchksums.is_hchksums;

			isreq->is_hchksums.is_rescdef_chksum = (unsigned long) ns(ISChkSums_rdChkSum(B));

			for (i = 0; i < ns(HookChkSum_vec_len(chksums)); i++) {
				ns(HookChkSum_table_t) chksum = ns(HookChkSum_vec_at(chksums, i));

				hchk = (hook_chksums_t *)calloc(1, sizeof(hook_chksums_t));
				hchk->hk_chksum = (unsigned long) ns(HookChkSum_hkChkSum(chksum));
				hchk->py_chksum = (unsigned long) ns(HookChkSum_pyChkSum(chksum));
				hchk->cf_chksum = (unsigned long) ns(HookChkSum_cfChkSum(chksum));

				hchk = hchk->next;
			}

			break;

		default:
			log_eventf(PBSEVENT_ERROR, PBS_EVENTCLASS_REQUEST, LOG_ERR, __func__, "IS Req Body bad, type %d", (int) type);
			rc = PBSE_PROTOCOL;
			break;
	}

}

int wire_is_read(int sock, pbs_is_t *piss)
{
	int rc = 0;
	void *buf = NULL;
	size_t bufsz = 0;
	ns(InterSvr_table_t) req;

	//FIXME: get loaded buf for read with size here
	rc = ns(InterSvr_verify_as_root(buf, bufsz));
	if (rc != 0) {
		log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
				"Bad IS, errno %d, wire error %d", errno, rc);
		return PBSE_PROTOCOL;
	}

	req = ns(InterSvr_as_root(buf));
	piss->is_cmd = ns(InterSvr_cmd(req));

	switch (piss->is_cmd) {
		case IS_CMD:
			piss->is_breq = alloc_br(0);
			if (piss->is_breq == NULL)
				return PBSE_SYSTEM;
			piss->is_breq->prot = PROT_TPP;
			COPYSTR_S(piss->is_breq->tppcmd_msgid, ns(InterSvr_msgId(req)));
			rc = __wire_request_read(ns(InterSvr_req(req)), piss->is_breq);
			break;

		case IS_CMD_REPLY:
			piss->is_breply = (breply_t *) calloc(1, sizeof(breply_t));
			if (piss->is_breply == NULL)
				return PBSE_SYSTEM;
			rc = __wire_reply_read(ns(InterSvr_reply(req)), piss->is_breply, 1);

		default:
			piss->is_req = (is_req_t *) calloc(1, sizeof(is_req_t));
			if (piss->is_req == NULL)
				return PBSE_SYSTEM;
			rc = __wire_is_read(ns(InterSvr_req(req)), piss->is_req);
			break;
	}

	return rc;
}
