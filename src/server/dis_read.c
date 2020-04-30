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

// FIXME: Move this to libwire and rename to wire_decode_batch_request.c

#include "pbs_wire.h"
#include "log.h"

extern char	*msg_nosupport;

/**
 * @brief
 * 	Read encoded request from the given buffer and decodes it
 * 	into the request structure
 *
 * @param[in] buf - buffer which holds encoded request
 * @param[in] request - pointer to request structure
 *
 * @return int
 * @retval PBSE_NONE - success
 * @retval PBSE_EOF  - on EOF (no request but no error)
 * @retval >PBSE_    - failure
 */
int
wire_decode_batch_request(void *buf, breq *request)
{
	int proto_type = 0;
	int proto_ver = 0;
	int rc = PBSE_NONE;
	void *body = NULL;

	/* Decode the Request Header, that will tell the request type */
	rc = wire_decode_batch_req_hdr(buf, request, &proto_type, &proto_ver);
	if (rc != PBSE_NONE || proto_type != ns(ProtType_Batch) || proto_ver > PBS_BATCH_PROT_VER) {
		log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
				"Req Header bad, errno %d, wire error %d", errno, rc);
		return PBSE_PROTOCOL;
	}

	body = (void *)ns(Req_body((ns(Req_table_t))buf));

	/* Decode the Request Body based on the type */
	switch (request->rq_type) {
		case PBS_BATCH_Connect:
			break;

		case PBS_BATCH_Disconnect:
			return PBSE_EOF;

		case PBS_BATCH_QueueJob:
		case PBS_BATCH_SubmitResv:
			rc = wire_decode_batch_req_queuejob(body, request);
			break;

		case PBS_BATCH_JobCred:
			rc = wire_decode_batch_req_jobcred(body, request);
			break;

		case PBS_BATCH_UserCred:
			rc = wire_decode_batch_req_usercred(body, request);
			break;

		case PBS_BATCH_jobscript:
		case PBS_BATCH_MvJobFile:
			rc = wire_decode_batch_req_jobfile(body, request);
			break;

		case PBS_BATCH_RdytoCommit:
		case PBS_BATCH_Commit:
		case PBS_BATCH_Rerun:
			rc = wire_decode_batch_req_jobid(body, request);
			break;

		case PBS_BATCH_DeleteJob:
		case PBS_BATCH_DeleteResv:
		case PBS_BATCH_ResvOccurEnd:
		case PBS_BATCH_HoldJob:
		case PBS_BATCH_ModifyJob:
		case PBS_BATCH_ModifyJob_Async:
			rc = wire_decode_batch_req_manage(body, request);
			break;

		case PBS_BATCH_MessJob:
			rc = wire_decode_batch_req_messagejob(body, request);
			break;

		case PBS_BATCH_Shutdown:
		case PBS_BATCH_FailOver:
			rc = wire_decode_batch_req_shutdown(body, request);
			break;

		case PBS_BATCH_SignalJob:
			rc = wire_decode_batch_req_signaljob(body, request);
			break;

		case PBS_BATCH_StatusJob:
			rc = wire_decode_batch_req_status(body, request);
			break;

		case PBS_BATCH_PySpawn:
			rc = wire_decode_batch_req_pyspawn(body, request);
			break;

		case PBS_BATCH_Authenticate:
			rc = wire_decode_batch_req_authenticate(body, request);
			break;

#ifndef PBS_MOM
		case PBS_BATCH_RelnodesJob:
			rc = wire_decode_batch_req_relnodesjob(body, request);
			break;

		case PBS_BATCH_LocateJob:
			rc = wire_decode_batch_req_jobid(body, request);
			break;

		case PBS_BATCH_Manager:
		case PBS_BATCH_ReleaseJob:
		case PBS_BATCH_ModifyResv:
			rc = wire_decode_batch_req_manage(body, request);
			break;

		case PBS_BATCH_MoveJob:
		case PBS_BATCH_OrderJob:
			rc = wire_decode_batch_req_movejob(body, request);
			break;

		case PBS_BATCH_RunJob:
		case PBS_BATCH_AsyrunJob:
		case PBS_BATCH_StageIn:
		case PBS_BATCH_ConfirmResv:
			rc = wire_decode_batch_req_run(body, request);
			break;

		case PBS_BATCH_DefSchReply:
			rc = wire_decode_batch_req_defschreply(body, request);
			break;

		case PBS_BATCH_SelectJobs:
		case PBS_BATCH_SelStat:
			rc = wire_decode_batch_req_selectjob(body, request);
			break;

		case PBS_BATCH_StatusNode:
		case PBS_BATCH_StatusResv:
		case PBS_BATCH_StatusQue:
		case PBS_BATCH_StatusSvr:
		case PBS_BATCH_StatusSched:
		case PBS_BATCH_StatusRsc:
		case PBS_BATCH_StatusHook:
			rc = wire_decode_batch_req_status(body, request);
			break;

		case PBS_BATCH_TrackJob:
			rc = wire_decode_batch_req_trackjob(body, request);
			break;

		case PBS_BATCH_Rescq:
		case PBS_BATCH_ReserveResc:
		case PBS_BATCH_ReleaseResc:
			rc = wire_decode_batch_req_rescq(body, request);
			break;

		case PBS_BATCH_RegistDep:
			rc = wire_decode_batch_req_register(body, request);
			break;

		case PBS_BATCH_PreemptJobs:
			rc = wire_decode_batch_req_preemptjobs(body, request);
			break;

#else	/* yes PBS_MOM */

		case PBS_BATCH_CopyHookFile:
			rc = wire_decode_batch_req_copyhookfile(body, request);
			break;

		case PBS_BATCH_DelHookFile:
			rc = wire_decode_batch_req_delhookfile(body, request);
			break;

		case PBS_BATCH_CopyFiles:
		case PBS_BATCH_DelFiles:
			rc = wire_decode_batch_req_copyfiles(body, request);
			break;

		case PBS_BATCH_CopyFiles_Cred:
		case PBS_BATCH_DelFiles_Cred:
			rc = wire_decode_batch_req_copyfiles_cred(body, request);
			break;

		case PBS_BATCH_Cred:
			rc = wire_decode_batch_req_cred(body, request);
			break;

#endif	/* PBS_MOM */

		default:
			log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
					"%s: %d from %s", msg_nosupport, request->rq_type, request->rq_user);
			rc = PBSE_UNKREQ;
			break;
	}

	if (rc == PBSE_NONE) {
		/* Decode the Request Extension, if present */
		rc = wire_decode_batch_req_extend(buf, request);
		if (rc != PBSE_NONE) {
			log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
					"Request type: %d Req Extension bad, error %d", request->rq_type, rc);
			rc = PBSE_PROTOCOL;
		}
	} else if (rc != PBSE_UNKREQ) {
		log_eventf(PBSEVENT_DEBUG, PBS_EVENTCLASS_REQUEST, LOG_DEBUG, __func__,
				"Req Body bad, type %d", request->rq_type);
		rc = PBSE_PROTOCOL;
	}

	return (rc);
}
