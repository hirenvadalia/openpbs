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

// FIXME: Move this to libwire and rename to wire_decode_batch_reply.c

#include "pbs_wire.h"

/**
 * @brief
 * 	Read encoded jobid reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_jobid(void *buf, breply *reply)
{
	ns(TextResp_table_t) B = (ns(TextResp_table_t))buf;

	COPYSTR_B(reply->brp_un.brp_jid, ns(TextResp_txt(B)));

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded select reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_select(void *buf, breply *reply)
{
	int i;
	size_t len;
	ns(SelectResp_table_t) B = (ns(SelectResp_table_t))buf;
	flatbuffers_string_vec_t ids = ns(SelectResp_ids(B));
	struct brp_select **pselx = &(reply->brp_un.brp_select);

	*pselx = NULL;
	len = flatbuffers_string_vec_len(ids);
	for (i = 0; i < len; i++) {
		struct brp_select *psel = (struct brp_select *)malloc(sizeof(struct brp_select));
		if (psel == NULL)
			return PBSE_SYSTEM;

		psel->brp_next = NULL;
		COPYSTR_B(psel->brp_jobid, flatbuffers_string_vec_at(ids, i));

		*pselx = psel;
		pselx = &(psel->brp_next);
	}

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded status reply from the given buffer and decodes it
 * 	into the reply structure (Server version)
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_status_svr(void *buf, breply *reply)
{
	int i;
	size_t len;
	ns(StatResp_table_t) B = (ns(StatResp_table_t))buf;
	ns(StatRespStat_vec_t) stats = ns(StatResp_stats(B));

	CLEAR_HEAD(reply->brp_un.brp_status);

	len = ns(StatRespStat_vec_len(stats));
	for (i = 0; i < len; i++) {
		int rc;
		ns(StatRespStat_table_t) stat = ns(StatRespStat_vec_at(stats, i));
		struct brp_status *pstsvr = (struct brp_status *)malloc(sizeof(struct brp_status));

		if (pstsvr == NULL)
			return PBSE_SYSTEM;

		CLEAR_LINK(pstsvr->brp_stlink);
		pstsvr->brp_objtype = ns(StatRespStat_type(stat));
		COPYSTR_B(pstsvr->brp_objname, ns(StatRespStat_name(stat)));
		rc = wire_decode_svrattrl((void *)ns(StatRespStat_attrs(stat)), &(pstsvr->brp_attr));
		if (rc != PBSE_NONE)
			return rc;
		append_link(&(reply->brp_un.brp_status), &(pstsvr->brp_stlink), pstsvr);
	}

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded status reply from the given buffer and decodes it
 * 	into the reply structure (Command version)
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_status_cmd(void *buf, breply *reply)
{
	int i;
	size_t len;
	ns(StatResp_table_t) B = (ns(StatResp_table_t))buf;
	ns(StatRespStat_vec_t) stats = ns(StatResp_stats(B));
	struct brp_cmdstat **pstcx = &(reply->brp_un.brp_statc);

	*pstcx = NULL;
	len = ns(StatRespStat_vec_len(stats));
	for (i = 0; i < len; i++) {
		int rc;
		ns(StatRespStat_table_t) stat = ns(StatRespStat_vec_at(stats, i));
		struct brp_cmdstat *pstcmd = (struct brp_cmdstat *)malloc(sizeof(struct brp_cmdstat));

		if (pstcmd == NULL)
			return PBSE_SYSTEM;

		pstcmd->brp_stlink = NULL;
		pstcmd->brp_objtype = ns(StatRespStat_type(stat));
		COPYSTR_B(pstcmd->brp_objname, ns(StatRespStat_name(stat)));
		rc = wire_decode_attrl((void *)ns(StatRespStat_attrs(stat)), &(pstcmd->brp_attrl));
		if (rc != PBSE_NONE)
			return rc;
		*pstcx = pstcmd;
		pstcx  = &(pstcmd->brp_stlink);
	}

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded text reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_text(void *buf, breply *reply)
{
	ns(TextResp_table_t) B = (ns(TextResp_table_t))buf;

	COPYSTR_S(reply->brp_un.brp_txt.brp_str, ns(TextResp_txt(B)));
	reply->brp_un.brp_txt.brp_txtlen = strlen(reply->brp_un.brp_txt.brp_str);

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded location reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_locate(void *buf, breply *reply)
{
	ns(TextResp_table_t) B = (ns(TextResp_table_t))buf;

	COPYSTR_B(reply->brp_un.brp_locate, ns(TextResp_txt(B)));

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded resc query reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_rescquery(void *buf, breply *reply)
{
	int i;
	size_t len;
	ns(RescQueryResp_table_t) B = (ns (RescQueryResp_table_t))buf;
	ns(RescQueryRespInfo_vec_t) infos = ns(RescQueryResp_infos(B));

	reply->brp_un.brp_rescq.brq_avail = NULL;
	reply->brp_un.brp_rescq.brq_alloc = NULL;
	reply->brp_un.brp_rescq.brq_resvd = NULL;
	reply->brp_un.brp_rescq.brq_down  = NULL;

	len = ns(RescQueryRespInfo_vec_len(infos));
	reply->brp_un.brp_rescq.brq_number = len;

	reply->brp_un.brp_rescq.brq_avail = (int *)malloc(sizeof(int) * len);
	reply->brp_un.brp_rescq.brq_alloc = (int *)malloc(sizeof(int) * len);
	reply->brp_un.brp_rescq.brq_resvd = (int *)malloc(sizeof(int) * len);
	reply->brp_un.brp_rescq.brq_down = (int *)malloc(sizeof(int) * len);
	if (reply->brp_un.brp_rescq.brq_avail == NULL ||
		reply->brp_un.brp_rescq.brq_alloc == NULL ||
		reply->brp_un.brp_rescq.brq_resvd == NULL ||
		reply->brp_un.brp_rescq.brq_down == NULL) {
		if (reply->brp_un.brp_rescq.brq_alloc)
			free(reply->brp_un.brp_rescq.brq_alloc);
		if (reply->brp_un.brp_rescq.brq_avail)
			free(reply->brp_un.brp_rescq.brq_avail);
		if (reply->brp_un.brp_rescq.brq_resvd)
			free(reply->brp_un.brp_rescq.brq_resvd);
		if (reply->brp_un.brp_rescq.brq_down)
			free(reply->brp_un.brp_rescq.brq_down);
		reply->brp_un.brp_rescq.brq_alloc = NULL;
		reply->brp_un.brp_rescq.brq_avail = NULL;
		reply->brp_un.brp_rescq.brq_resvd = NULL;
		return PBSE_SYSTEM;
	}

	for (i = 0; i < len; i++) {
		ns(RescQueryRespInfo_table_t) info = ns(RescQueryRespInfo_vec_at(infos, i));

		*(reply->brp_un.brp_rescq.brq_avail + i) = (int) ns(RescQueryRespInfo_avail(info));
		*(reply->brp_un.brp_rescq.brq_alloc + i) = (int) ns(RescQueryRespInfo_alloc(info));
		*(reply->brp_un.brp_rescq.brq_resvd + i) = (int) ns(RescQueryRespInfo_resvd(info));
		*(reply->brp_un.brp_rescq.brq_down + i) = (int) ns(RescQueryRespInfo_down(info));
	}

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded preemt jobs reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply body
 * @param[in] reply - pointer to reply structure
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
static int
wire_decode_batch_reply_preemptjobs(void *buf, breply *reply)
{
	int i;
	size_t len;
	ns(Preempt_table_t) B = (ns(Preempt_table_t))buf;
	ns(PreemptJob_vec_t) infos = ns(Preempt_infos(B));

	len = ns(PreemptJob_vec_len(infos));
	reply->brp_un.brp_preempt_jobs.count = (int) len;

	reply->brp_un.brp_preempt_jobs.ppj_list = NULL;
	reply->brp_un.brp_preempt_jobs.ppj_list = (preempt_job_info *)calloc(sizeof(struct preempt_job_info), len);
	if (reply->brp_un.brp_preempt_jobs.ppj_list == NULL)
		return PBSE_SYSTEM;

	for (i = 0; i < len; i++) {
		ns(PreemptJob_table_t) info = ns(PreemptJob_vec_at(infos, i));

		COPYSTR_B(reply->brp_un.brp_preempt_jobs.ppj_list[i].job_id, ns(PreemptJob_jid(info)));
		COPYSTR_B(reply->brp_un.brp_preempt_jobs.ppj_list[i].order, ns(PreemptJob_order(info)));
	}

	return PBSE_NONE;
}

/**
 * @brief
 * 	Read encoded reply from the given buffer and decodes it
 * 	into the reply structure
 *
 * @param[in] buf - buffer which holds encoded reply
 * @param[in] reply - pointer to reply structure
 * @param[in] forsvr - decode reply for server or client command
 *
 * @return int
 * @retval PBSE_NONE  - success
 * @retval !PBSE_NONE - failure
 */
int
wire_decode_batch_reply(void *buf, breply *reply, int forsvr)
{
	int rc = PBSE_NONE;
	ns(Resp_table_t) resp = (ns(Resp_table_t))buf;
	void *body = NULL;

	reply->brp_code = ns(Resp_code(resp));
	reply->brp_auxcode = ns(Resp_auxCode(resp));
	reply->brp_choice = ns(Resp_choice(resp));
	if (ns(Resp_body_is_present(resp)))
		body = (void *)ns(Resp_body(resp));

	switch (reply->brp_choice) {

		case BATCH_REPLY_CHOICE_NULL:
			break;

		case BATCH_REPLY_CHOICE_Queue:
		case BATCH_REPLY_CHOICE_RdytoCom:
		case BATCH_REPLY_CHOICE_Commit:
			rc = wire_decode_batch_reply_jobid(body, reply);
			break;

		case BATCH_REPLY_CHOICE_Select:
			rc = wire_decode_batch_reply_select(body, reply);
			break;

		case BATCH_REPLY_CHOICE_Status:
			if (forsvr)
				rc = wire_decode_batch_reply_status_svr(body, reply);
			else
				rc = wire_decode_batch_reply_status_cmd(body, reply);
			break;

		case BATCH_REPLY_CHOICE_Text:
			rc = wire_decode_batch_reply_text(body, reply);
			break;

		case BATCH_REPLY_CHOICE_Locate:
			rc = wire_decode_batch_reply_locate(body, reply);
			break;

		case BATCH_REPLY_CHOICE_RescQuery:
			rc = wire_decode_batch_reply_rescquery(body, reply);
			break;

		case BATCH_REPLY_CHOICE_PreemptJobs:
			rc = wire_decode_batch_reply_preemptjobs(body, reply);
			break;

		default:
			rc = PBSE_PROTOCOL;
			break;
	}

	return rc;
}
