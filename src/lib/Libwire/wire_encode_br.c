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

#include <pbs_config.h>   /* the master config generated by configure */
#include <fcntl.h>
#ifndef WIN32
#include <stdint.h>
#endif
#include "pbs_ifl.h"
#include "libpbs.h"

#define START_REQ(B, prot, msgid, type) do { \
	ns(Header_ref_t) hdr = PBSE_FLATCC_ERROR; \
	if (wire_encode_batch_start(B, prot, msgid) == PBSE_FLATCC_ERROR) \
		return (pbs_errno = PBSE_PROTOCOL); \
	hdr = wire_encode_hdr(B, type, pbs_current_user, prot, msgid); \
	if (hdr == PBSE_FLATCC_ERROR) \
		return (pbs_errno = PBSE_PROTOCOL); \
	ns(Req_hdr_add(B, hdr)); \
} while(0)

#define END_REQ(name, prot, B, body, extend) do { \
	if (body != -1) { \
		if (body == PBSE_FLATCC_ERROR) \
			return (pbs_errno = PBSE_PROTOCOL); \
		ns(Req_body_add(B, ns(ReqBody_as_ ## name(body)))); \
	} \
	if (extend != NULL) { \
		flatbuffers_string_ref_t s = 0; \
		FB_STR(s, B, extend); \
		ns(Req_extend_add(B, s)); \
	} \
	if (prot == PROT_TPP) { \
		ns(InterSvr_breq_add(B, ns(Req_end(B)))); \
		ns(InterSvr_end_as_root(B)); \
	} else \
		ns(Req_end_as_root(B)); \
} while(0)


/**
 * @brief - Start a standard inter-server message.
 *
 * @param[in] stream  - The TPP stream on which to send message
 * @param[in] command - The message type (cmd) to encode
 *
 * @return void
 */
void
is_compose(flatcc_builder_t *B, int command)
{
	int ret;

	ns(InterSvr_start_as_root(B));
	ns(InterSvr_cmd_add(B, command));
}

/**
 * @brief - Get a unique id each time this function is called
 *
 * @par NOTE:
 *	This id is used as a message id in every command sent out from
 * 	this daemon. This is done to match replies to asynchronous
 * 	command sends to the replies that we receive later
 *
 * @param[out] id - The return msgid created
 *
 * @return int
 * @retval PBSE_NONE  - Success
 * @retval !PBSE_NONE - Failure
 */
int
get_msgid(char **id)
{
	char msgid[MAXNAMLEN];

	static time_t last_time = -1;
	static int counter = 0;
	time_t now = time(NULL);

	if (now != last_time) {
		counter = 0;
		last_time = now;
	} else {
		counter++;
	}
#ifdef WIN32
	sprintf(msgid, "%ld:%d", now, counter);
#else
	sprintf(msgid, "%ju:%d", (uintmax_t)now, counter);
#endif
	if ((*id = strdup(msgid)) == NULL)
		return PBSE_SYSTEM;

	return PBSE_NONE;
}

/**
 * @brief - Compose a command to be sent over TPP stream
 *
 * @par Functionality:
 *	calls im_compose to create the message header, get_msgid to
 * 	add a msg id to the header (unless one is passed)
 *
 * @param[in] stream - Tpp stream to write to
 * @param[in] command - The command to encode
 * @param[in,out] ret_msgid - The msgid, if passed to this function, is
 *                            the msgid to be used for this message.
 *                            If msgid is not passed, then create a unique
 *                            msgid and set for the message, also return it
 *                            back to caller.
 *
 * @return error code
 * @retval  DIS_SUCCESS - Success
 * @retval !DIS_SUCCESS - Failure
 */
int
is_compose_cmd(flatcc_builder_t *B, int command, char **ret_msgid)
{
	flatbuffers_string_ref_t s;

	is_compose(B, command);

	if (ret_msgid == NULL || *ret_msgid == NULL || *ret_msgid[0] == '\0') /* NULL or empty id provided */
		if (get_msgid(ret_msgid) != PBSE_NONE)
			return PBSE_SYSTEM;
	FB_STR(s, B, *ret_msgid);
	ns(InterSvr_msgId_add(B, s));
	return PBSE_SYSTEM;
}

int
PBSD_jobid_put(int c, int reqtype, char *jobid, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(JobId_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_JobId(B, jobid);
	END_REQ(JobId, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_movejob_put(int c, int reqtype, char *jobid, char *destin, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Move_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_MoveJob(B, jobid, destin);
	END_REQ(Move, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_rescquery_put(int c, int reqtype, char **rescl, int ct, pbs_resource_t rh, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(RescQuery_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_RescQuery(B, rescl, ct, rh);
	END_REQ(RescQuery, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int PBSD_dmncmd_put(int c, int reqtype, int cmd, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(DmnCmd_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_DmnCmd(B, cmd);
	END_REQ(DmnCmd, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_empty_put(int c, int reqtype, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;

	START_REQ(B, prot, msgid, reqtype);
	/* Not used but still use DmnCmd in END_REQ to make compiler happy */
	END_REQ(DmnCmd, prot, B, -1, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_select_put(int c, int reqtype, struct attropl *aoplp, struct attrl *alp, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Select_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_Select(B, aoplp, alp);
	END_REQ(Select, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_auth_put(int c, char *auth_method, char *encrypt_method, int encrypt_mode, int port, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Auth_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_Authenticate);
	body = wire_encode_Auth(B, auth_method, encrypt_method, encrypt_mode, port);
	END_REQ(Auth, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_register_put(int c, char *owner, char *parent, char *child, int type, int op, int cost, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Register_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_RegistDep);
	body = wire_encode_Register(B, owner, parent, child, type, op, cost);
	END_REQ(Register, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_trackjob_put(int c, char *jobid, int hops, char *location, char state, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Track_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_TrackJob);
	body = wire_encode_TrackJob(B, jobid, hops, location, state);
	END_REQ(Track, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_files_put(int c, int reqtype, struct rq_cpyfile *pcf, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(CopyFile_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_CopyFiles(B, pcf);
	END_REQ(CopyFile, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_files_cred_put(int c, int reqtype, struct rq_cpyfile_cred *pcf, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(CopyFileCred_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_CopyFilesCred(B, pcf);
	END_REQ(CopyFile, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

int
PBSD_run(int c, int reqtype, char *jobid, char *location, unsigned long resch, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Run_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_Run(B, jobid, location, resch);
	END_REQ(Run, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		struct batch_reply *reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

int
PBSD_defschreply(int c, int cmd, char *id, int err, char *txt, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(SchedDefRep_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_DefSchReply);
	body = wire_encode_SchedDefRep(B, cmd, id, err, txt);
	END_REQ(SchedDefRep, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		struct batch_reply *reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *	-PBSD_rdytocmt This function does the Ready To Commit sub-function of
 *	the Queue Job request.
 *
 * @param[in] c - socket fd
 * @param[in] jobid - job identifier
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return	int
 * @retval	0		success
 * @retval	!0(pbs_errno)	failure
 *
 */
int
PBSD_rdytocmt(int c, char *jobid, int prot, char **msgid)
{
	int rc;

	rc = PBSD_jobid(c, jobid, prot, msgid, PBS_BATCH_RdytoCommit);
	if (rc != PBSE_NONE)
		return rc;

	if (prot == PROT_TCP) {
		struct batch_reply *reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *	-PBS_commit.c This function does the Commit sub-function of
 *	the Queue Job request.
 *
 * @param[in] c - socket fd
 * @param[in] jobid - job identifier
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               success
 * @retval      !0(pbs_errno)   failure
 *
 */
int
PBSD_commit(int c, char *jobid, int prot, char **msgid)
{
	int rc;

	rc = PBSD_jobid(c, jobid, prot, msgid, PBS_BATCH_Commit);
	if (rc != PBSE_NONE)
		return rc;

	if (prot == PROT_TCP) {
		struct batch_reply *reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *	-PBS_scbuf.c Send a chunk of a of the job script to the server.
 *	Called by pbs_submit.  The buffer length could be
 *	zero; the server should handle that case...
 *
 * @param[in] c - connection handle
 * @param[in] reqtype - request type
 * @param[in] seq - file chunk sequence number
 * @param[in] buf - file chunk
 * @param[in] len - length of chunk
 * @param[in] jobid - ob id (for types 1 and 2 only)
 * @param[in] which - standard file type (enum)
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               success
 * @retval      !0(pbs_errno)   failure
 *
 */
int
PBSD_scbuf(int c, int reqtype, int seq, char *buf, int len, char *jobid, enum job_file which, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(JobFile_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_JobFile(B, seq, buf, len, jobid ? jobid : "", which);
	END_REQ(JobFile, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *	-The Job File function used to move files related to
 *	a job between servers.
 *	-- the function PBS_scbuf is called repeatedly to
 *	transfer chunks of the script to the server.
 *
 * @param[in] c - connection handle
 * @param[in] script_file - job file
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return	int
 * @retval	0	success
 * @retval	-1	failure
 *
 */

int
PBSD_jscript(int c, char *script_file, int prot, char **msgid)
{
	int i;
	int fd;
	int cc;
	char s_buf[SCRIPT_CHUNK_Z];
	int rc = 0;

	if ((fd = open(script_file, O_RDONLY, 0)) < 0) {
		return (-1);
	}
	i = 0;
	cc = read(fd, s_buf, SCRIPT_CHUNK_Z);
	while ((cc > 0) &&
		((rc = PBSD_scbuf(c, PBS_BATCH_jobscript, i, s_buf, cc, NULL, JScript, prot, msgid)) == PBSE_NONE)) {
		i++;
		cc = read(fd, s_buf, SCRIPT_CHUNK_Z);
	}

	close(fd);
	if (cc < 0)
		return (-1);

	return rc;
}

/**
 * @brief
 *	job file function for moving file between server/mom
 *
 * @param[in] c - connection handle
 * @param[in] script_file - job file
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0       success
 * @retval      -1      failure
 *
 */
int
PBSD_jscript_direct(int c, char *script, int prot, char **msgid)
{
	int rc;
	int tosend;
	int i = 0;
	char *p = script;
	int len;

	if (script == NULL) {
		pbs_errno = PBSE_INTERNAL;
		return -1;
	}

	len = strlen(script);
	do {
		tosend = (len > SCRIPT_CHUNK_Z) ? SCRIPT_CHUNK_Z : len;
		rc = PBSD_scbuf(c, PBS_BATCH_jobscript, i, p, tosend, NULL, JScript, prot, msgid);
		i++;
		p += tosend;
		len -= tosend;
	} while ((rc == PBSE_NONE) && (len > 0));

	return rc;
}


/**
 * @brief
 *	-PBS_jobfile.c
 *	The Job File function used to move files related to
 *	a job between servers.
 *	-- the function PBS_scbuf is called repeatedly to
 *	transfer chunks of the script to the server.
 *
 * @param[in] c - connection handle
 * @param[in] reqtype - request type
 * @param[in] path - file path
 * @param[in] jobid - job id
 * @param[in] which - standard file type (enum)
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0       success
 * @retval      -1      failure
 *
 */
int
PBSD_jobfile(int c, int req_type, char *path, char *jobid, enum job_file which, int prot, char **msgid)
{
	int   i;
	int   cc;
	int   fd;
	char  s_buf[SCRIPT_CHUNK_Z];
	int rc = 0;

	if ((fd = open(path, O_RDONLY, 0)) < 0) {
		return (-1);
	}
	i = 0;
	cc = read(fd, s_buf, SCRIPT_CHUNK_Z);
	while ((cc > 0) &&
		((rc = PBSD_scbuf(c, req_type, i, s_buf, cc, jobid, which, prot, msgid)) == PBSE_NONE)) {
		i++;
		cc = read(fd, s_buf, SCRIPT_CHUNK_Z);
	}

	close(fd);
	if (cc < 0)
		return (-1);
	return rc;
}

/**
 * @brief
 *	-PBS_queuejob.c
 *	This function sends the first part of the Queue Job request
 *
 * @param[in] c - socket descriptor
 * @param[in] jobid - job identifier
 * @param[in] destin - destination name
 * @param[in] attrib - pointer to attribute list
 * @param[in] extend - extention string for req encode
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               Success
 * @retval      pbs_error(!0)   error
 */
char *
PBSD_queuejob(int c, char *jobid, char *destin, struct attropl *attrib, char *extend, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Qjob_ref_t) body = PBSE_FLATCC_ERROR;
	char *return_jobid = NULL;

	START_REQ(B, prot, msgid, PBS_BATCH_QueueJob);
	body = wire_encode_QueueJob(B, jobid, destin, attrib);
	END_REQ(Qjob, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TPP) {
		pbs_errno = PBSE_NONE; /* return something NON-NULL for tpp */
		return ("");
	}

	/* read reply from stream into presentation element */
	reply = PBSD_rdrpy(c);
	if (reply == NULL) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (reply->brp_choice &&
			reply->brp_choice != BATCH_REPLY_CHOICE_Text &&
			reply->brp_choice != BATCH_REPLY_CHOICE_Queue) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (get_conn_errno(c) == 0) {
		return_jobid = strdup(reply->brp_un.brp_jid);
		if (return_jobid == NULL) {
			pbs_errno = PBSE_SYSTEM;
		}
	}

	PBSD_FreeReply(reply);
	return return_jobid;
}

/**
 * @brief
 *	 encode a Job Credential Batch Request
 *
 * @param[in] c - socket descriptor
 * @param[in] credid - credential id (e.g. principal)
 * @param[in] jobid - job id
 * @param[in] data - credentials
 * @param[in] validity - credential validity
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - msg id
 *
 * @return	int
 * @retval	0		success
 * @retval	!0(pbse error)	error
 *
 */
int
PBSD_cred(int c, char *credid, char *jobid, int cred_type, char *data, long validity, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Cred_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_Cred);
	body = wire_encode_Cred(B, jobid, credid, cred_type, data, strlen(data), validity);
	END_REQ(Cred, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);
	return PBSE_NONE;
}

/**
 *
 * @brief
 *	Send a chunk of data (buf) of size 'len', sequence 'seq'  associated
 *	with the 'hook_filename', over the connection handle 'c'.
 *
 * @param[in]	c - connection channel
 * @param[in]   reqtype - request type
 * @param[in] 	seq - sequence of a block of data (0,1,...)
 * @param[in] 	buf - a block of data
 * @param[in] 	len - size of buf
 * @param[in]	hook_filename - hook filename
 * @param[in]   prot - PROT_TCP or PROT_TPP
 * @param[in]   msgid - msg
 *
 * @return 	int
 * @retval	0 for success
 * @retval	non-zero otherwise.
 */
static int
PBSD_hookbuf(int c, int reqtype, int seq, char *buf, int len, char *hook_filename, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(CopyHook_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, reqtype);
	body = wire_encode_CopyHook(B, seq, buf, len, hook_filename);
	END_REQ(CopyHook, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 *
 * @brief
 *	Copy the contents of 'hook_filepath' over the network connection
 *	handle 'c'.
 *
 * @param[in]	c - connection channel
 * @param[in]	hook_filepath - local full file pathname
 * @param[in]   prot - PROT_TCP or PROT_TPP
 * @param[in]   msgid - msg
 *
 * @return int
 * @retval	0 for success
 * @retval	-2 for success, no hookfile or empty hookfile
 * @retval	non-zero otherwise.
 */
int
PBSD_copyhookfile(int c, char *hook_filepath, int prot, char **msgid)
{
	int i;
	int fd;
	int cc;
	int rc = -2;
	char s_buf[SCRIPT_CHUNK_Z];
	char *p;
	char hook_file[MAXPATHLEN+1];

	if ((fd = open(hook_filepath, O_RDONLY, 0)) < 0) {
		if (prot == PROT_TPP)
			return (-2);  /* ok, if nothing to copy */
		else
			return 0;
	}

	/* set hook_file to the relative path of 'hook_filepath' */
	strncpy(hook_file, hook_filepath, MAXPATHLEN);
	if ((p = strrchr(hook_filepath, '/')) != NULL) {
		strncpy(hook_file, p + 1, MAXPATHLEN);
	}

	i = 0;
	cc = read(fd, s_buf, SCRIPT_CHUNK_Z);

	while ((cc > 0) &&
		((rc = PBSD_hookbuf(c, PBS_BATCH_CopyHookFile, i, s_buf, cc, hook_file, prot, msgid)) == PBSE_NONE)) {
		i++;
		cc = read(fd, s_buf, SCRIPT_CHUNK_Z);
	}

	close(fd);
	if (cc < 0)
		return (-1);

	return rc;
}

/**
 *
 * @brief
 *	Send a Delete Hook file request of 'hook_filename' over the network
 *	channel 'c'.
 *
 * @param[in]	c - connection channel
 * @param[in]	hook_filename - hook filename
 * @param[in] 	prot - PROT_TCP or PROT_TPP
 * @param[in] 	msgid - msg
 *
 * @return 	int
 * @retval	0 for success
 * @retval	non-zero otherwise.
 */
int
PBSD_delhookfile(int c, char *hook_filename, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(CopyHook_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_DelHookFile);
	body = wire_encode_DelHook(B, hook_filename);
	END_REQ(CopyHook, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *	 encode a Job Credential Batch Request
 *
 * @param[in] c - socket descriptor
 * @param[in] type - credential type
 * @param[in] buf - credentials
 * @param[in] len - credential length
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - msg id
 *
 * @return	int
 * @retval	0		success
 * @retval	!0(pbse error)	error
 *
 */
int
PBSD_jcred(int c, int type, char *buf, int len, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Cred_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_JobCred);
	body = wire_encode_JobCred(B, type, buf, len);
	END_REQ(Cred, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	if (prot == PROT_TCP) {
		reply = PBSD_rdrpy(c);
		PBSD_FreeReply(reply);
		return get_conn_errno(c);
	}

	return PBSE_NONE;
}

/**
 * @brief
 *      -encode a Manager Batch Request
 *
 * @par Functionality:
 *              This request is used for most operations where an object is being
 *              created, deleted, or altered.
 *
 * @param[in] c - socket descriptor
 * @param[in] command - command type
 * @param[in] objtype - object type
 * @param[in] objname - object name
 * @param[in] aoplp - pointer to attropl structure(list)
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      DIS_SUCCESS(0)  success
 * @retval      error code      error
 *
 */
int
PBSD_mgr_put(int c, int function, int command, int objtype, char *objname, struct attropl *aoplp, char *extend, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Manage_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, function);
	body = wire_encode_Manage(B, command, objtype, objname, aoplp);
	END_REQ(Manage, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/**
 * @brief Sends the Modify Reservation request
 *
 * @param[in] connect - socket descriptor for the connection.
 * @param[in] resv-Id - Reservation Identifier
 * @param[in] attrib  - list of attributes to be modified.
 * @param[in] extend  - extended options
 *
 * @return - reply from server on no error.
 * @return - NULL on error.
 */
char *
PBSD_modify_resv(int connect, char *resv_id, struct attropl *attrib, char *extend)
{
	int rc = 0;
	struct batch_reply *reply = NULL;
	char *ret = NULL;

	rc = PBSD_mgr_put(connect, PBS_BATCH_ModifyResv, 0, MGR_OBJ_RESV, resv_id ? resv_id : "", attrib, extend, PROT_TCP, NULL);
	if (rc != PBSE_NONE)
		return rc;

	reply = PBSD_rdrpy(connect);
	if (reply == NULL)
		pbs_errno = PBSE_PROTOCOL;
	else {
		if (reply->brp_code == PBSE_NONE && reply->brp_un.brp_txt.brp_str != NULL) {
			if ((ret = strdup(reply->brp_un.brp_txt.brp_str)) == NULL)
				pbs_errno = PBSE_SYSTEM;
		}
		PBSD_FreeReply(reply);
	}
	return ret;
}

/**
 * @brief
 *	-PBS_msg_put Send the MessageJob request, does not read the reply.
 *
 * @param[in] c - socket descriptor
 * @param[in] jobid - job identifier
 * @param[in] fileopt - file type
 * @param[in] msg - msg to be sent
 * @param[in] extend - extention string for req encode
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               Success
 * @retval      pbs_error(!0)   error
 */
int
PBSD_msg_put(int c, char *jobid, int fileopt, char *msg, char *extend, int prot, char **msgid)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Msg_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_MessJob);
	body = wire_encode_Message(B, jobid, fileopt, msg);
	END_REQ(Msg, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/**
 * @brief
 *	-Send the PySpawn request, does not read the reply.
 *
 * @param[in] c - socket descriptor
 * @param[in] jobid - job identifier
 * @param[in] argv - pointer to arguments
 * @param[in] envp - pointer to environment vars
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return	int
 * @retval	0		Success
 * @retval	pbs_error(!0)	error
 */
int
PBSD_py_spawn_put(int c, char *jobid, char **argv, char **envp, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Spawn_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_PySpawn);
	body = wire_encode_PySpawn(B, jobid, argv, envp);
	END_REQ(Spawn, prot, B, body, NULL);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/*
 *	PBS_relnodes_put.c
 *
 *	Send the RelnodesJob request, does not read the reply.
 */
int
PBSD_relnodes_put(int c, char *jobid, char *node_list, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(RelNodes_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_RelnodesJob);
	body = wire_encode_RelNodes(B, jobid, node_list);
	END_REQ(RelNodes, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/**
 * @brief
 *	-Pass-through call to send preempt jobs batch request
 *
 * @param[in] connect - connection handler
 * @param[in] preempt_jobs_list - list of jobs to be preempted
 *
 * @return      preempt_job_info *
 * @retval      list of jobs and their preempt_method
 *
 */
preempt_job_info*
PBSD_preempt_jobs(int connect, char **preempt_jobs_list)
{
	struct batch_reply *reply;
	preempt_job_info *ppj_reply = NULL;
	flatcc_builder_t builder, *B = &builder;
	ns(Preempt_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, PROT_TCP, NULL, PBS_BATCH_PreemptJobs);
	body = wire_encode_PreemptJobs(B, preempt_jobs_list);
	END_REQ(Preempt, PROT_TCP, B, body, NULL);

	if (dis_flush(connect, B))
		return (pbs_errno = PBSE_PROTOCOL);

	reply = PBSD_rdrpy(connect);
	if (reply == NULL)
		pbs_errno = PBSE_PROTOCOL;
	else {
		ppj_reply = reply->brp_un.brp_preempt_jobs.ppj_list;
		reply->brp_un.brp_preempt_jobs.ppj_list = NULL;
		reply->brp_un.brp_preempt_jobs.count = 0;
		PBSD_FreeReply(reply);
	}
	return ppj_reply;
}

/**
 * @brief
 *	-PBS_sig_put.c Send the Signal Job Batch Request
 *
 * @param[in] c - socket descriptor
 * @param[in] jobid - job identifier
 * @param[in] signal - signal
 * @param[in] msg - msg to be sent
 * @param[in] extend - extention string for req encode
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               Success
 * @retval      pbs_error(!0)   error
 */
int
PBSD_sig_put(int c, char *jobid, char *signal, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Signal_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, PBS_BATCH_SignalJob);
	body = wire_encode_Signal(B, jobid, signal);
	END_REQ(Signal, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/**
 * @brief
 *	send status job batch request
 *
 * @param[in] c - socket descriptor
 * @param[in] function - request type
 * @param[in] id - object id
 * @param[in] attrib - pointer to attribute list
 * @param[in] extend - extention string for req encode
 * @param[in] prot - PROT_TCP or PROT_TPP
 * @param[in] msgid - message id
 *
 * @return      int
 * @retval      0               Success
 * @retval      pbs_error(!0)   error
 */
int
PBSD_status_put(int c, int function, char *id, struct attrl *attrib, char *extend, int prot, char **msgid)
{
	flatcc_builder_t builder, *B = &builder;
	ns(Stat_ref_t) body = PBSE_FLATCC_ERROR;

	START_REQ(B, prot, msgid, function);
	body = wire_encode_Status(B, id, attrib);
	END_REQ(Stat, prot, B, body, extend);

	if (dis_flush(c, B))
		return (pbs_errno = PBSE_PROTOCOL);

	return PBSE_NONE;
}

/**
 * @brief
 *	This function sends the Submit Reservation request
 *
 * @param[in] c - socket descriptor
 * @param[in] resv_id - reservation identifier
 * @param[in] attrib - pointer to attribute list
 * @param[in] extend - extention string for req encode
 *
 * @return      string
 * @retval      resvn id	Success
 * @retval      NULL		error
 *
 */

char *
PBSD_submit_resv(int connect, char *resv_id, struct attropl *attrib, char *extend)
{
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Qjob_ref_t) body = PBSE_FLATCC_ERROR;
	char *return_resv_id = NULL;

	START_REQ(B, PROT_TCP, NULL, PBS_BATCH_SubmitResv);
	body = wire_encode_QueueJob(B, resv_id, NULL, attrib);
	END_REQ(Qjob, PROT_TCP, B, body, extend);

	if (dis_flush(connect, B))
		return (pbs_errno = PBSE_PROTOCOL);

	/* read reply from stream into presentation element */
	reply = PBSD_rdrpy(connect);
	if (reply == NULL) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (!pbs_errno && reply->brp_choice &&
		reply->brp_choice != BATCH_REPLY_CHOICE_Text) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (get_conn_errno(connect) == 0 && reply->brp_code == 0) {
		if (reply->brp_choice == BATCH_REPLY_CHOICE_Text) {
			return_resv_id = strdup(reply->brp_un.brp_txt.brp_str);
			if (return_resv_id == NULL) {
				pbs_errno = PBSE_SYSTEM;
			}
		}
	}

	PBSD_FreeReply(reply);
	return return_resv_id;
}

/**
 * @brief
 *	send user credentials to the server.
 *
 * @param[in] c - connection handler
 * @param[in] user - username
 * @param[in] type - cred type
 * @param[in] buf - credentials
 * @param[in] len - length of buffer
 *
 * @return      int
 * @retval      0  		success
 * @retval      error code      error
 *
 */

int
PBSD_ucred(int c, char *user, int type, char *buf, int len)
{
	int rc;
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Header_ref_t) hdr = PBSE_FLATCC_ERROR;
	ns(Cred_ref_t) body = PBSE_FLATCC_ERROR;

	/* initialize the thread context data, if not already initialized */
	if (pbs_client_thread_init_thread_context() != 0)
		return pbs_errno;

	/* lock pthread mutex here for this connection */
	/* blocking call, waits for mutex release */
	if (pbs_client_thread_lock_connection(c) != 0)
		return pbs_errno;

	if (wire_encode_batch_start(B, PROT_TCP, NULL) == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	hdr = wire_encode_hdr(B, PBS_BATCH_UserCred, pbs_current_user, PROT_TCP, NULL);
	if (hdr == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	ns(Req_hdr_add(B, hdr));
	body = wire_encode_UserCred(B, user, type, buf, len);
	if (body == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	ns(Req_body_add(B, ns(ReqBody_as_Cred(body))));
	ns(Req_end_as_root(B));

	if (dis_flush(c, B)) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}

	reply = PBSD_rdrpy(c);
	PBSD_FreeReply(reply);
	rc = get_conn_errno(c);

	/* unlock the thread lock and update the thread context data */
	if (pbs_client_thread_unlock_connection(c) != 0)
		return pbs_errno;

	return rc;
}

/**
 * @brief
 *	-migrate users info on the current server to a destination server.
 *
 * @param[in] c - socket descriptor
 * @param[in] tohost - the destination host to migrate users cred to
 *
 * @return      int
 * @retval      0		success
 * @retval      error code      error
 *
 */
int
PBSD_user_migrate(int c, char *tohost)
{
	int rc;
	struct batch_reply *reply;
	flatcc_builder_t builder, *B = &builder;
	ns(Header_ref_t) hdr = PBSE_FLATCC_ERROR;
	ns(UserMigrate_ref_t) body = PBSE_FLATCC_ERROR;

	/* initialize the thread context data, if not already initialized */
	if (pbs_client_thread_init_thread_context() != 0)
		return pbs_errno;

	/* lock pthread mutex here for this connection */
	/* blocking call, waits for mutex release */
	if (pbs_client_thread_lock_connection(c) != 0)
		return pbs_errno;

	if (wire_encode_batch_start(B, PROT_TCP, NULL) == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	hdr = wire_encode_hdr(B, PBS_BATCH_UserMigrate, pbs_current_user, PROT_TCP, NULL);
	if (hdr == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	ns(Req_hdr_add(B, hdr));
	body = wire_encode_UserMigrate(B, tohost);
	if (body == PBSE_FLATCC_ERROR) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}
	ns(Req_body_add(B, ns(ReqBody_as_UserMigrate(body))));
	ns(Req_end_as_root(B));

	if (dis_flush(c, B)) {
		pbs_errno = PBSE_PROTOCOL;
		(void)pbs_client_thread_unlock_connection(c);
		return pbs_errno;
	}

	reply = PBSD_rdrpy(c);
	PBSD_FreeReply(reply);
	rc = get_conn_errno(c);

	/* unlock the thread lock and update the thread context data */
	if (pbs_client_thread_unlock_connection(c) != 0)
		return pbs_errno;

	return rc;
}

/**
 * @brief
 *	-wrapper function for PBSD_status_put which sends
 *	status batch request
 *
 * @param[in] c - socket descriptor
 * @param[in] function - request type
 * @param[in] id - object id
 * @param[in] attrib - pointer to attribute list
 * @param[in] extend - extention string for req encode
 *
 * @return	structure handle
 * @retval 	pointer to batch status on SUCCESS
 * @retval 	NULL on failure
 *
 */
struct batch_status *
PBSD_status(int c, int function, char *objid, struct attrl *attrib, char *extend)
{
	int rc;

	/* send the status request */
	if (objid == NULL)
		objid = "";	/* set to null string for encoding */

	rc = PBSD_status_put(c, function, objid, attrib, extend, PROT_TCP, NULL);
	if (rc) {
		return NULL;
	}

	/* get the status reply */

	return (PBSD_status_get(c));
}

/**
 * @brief
 *	Returns pointer to status record
 *
 * @param[in]   c - index into connection table
 *
 * @return returns a pointer to a batch_status structure
 * @retval pointer to batch status on SUCCESS
 * @retval NULL on failure
 */
struct batch_status *PBSD_status_get(int c)
{
	struct batch_status *rbsp = NULL;
	struct batch_reply  *reply;

	/* read reply from stream into presentation element */

	reply = PBSD_rdrpy(c);
	if (reply == NULL) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (reply->brp_choice != BATCH_REPLY_CHOICE_NULL  &&
		reply->brp_choice != BATCH_REPLY_CHOICE_Text &&
		reply->brp_choice != BATCH_REPLY_CHOICE_Status) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (get_conn_errno(c) == 0) {
		int i = 0;
		struct batch_status *bsp  = NULL;
		struct brp_cmdstat *stp = reply->brp_un.brp_statc;

		pbs_errno = 0;
		while (stp != NULL) {
			if (i++ == 0) {
				rbsp = bsp = calloc(1, sizeof(struct batch_status));
			} else {
				bsp->next = calloc(1, sizeof(struct batch_status));
				bsp = bsp->next;
			}
			if (bsp == NULL) {
				pbs_errno = PBSE_SYSTEM;
				break;
			}
			if ((bsp->name = strdup(stp->brp_objname)) == NULL) {
				pbs_errno = PBSE_SYSTEM;
				break;
			}
			bsp->attribs = stp->brp_attrl;
			stp->brp_attrl = 0;
			bsp->next = NULL;
			stp = stp->brp_stlink;
		}
		if (pbs_errno) {
			pbs_statfree(rbsp);
			rbsp = NULL;
		}
	}
	PBSD_FreeReply(reply);
	return rbsp;
}

/**
 * @brief
 *	-send manager request and read reply.
 *
 * @param[in] c - communication handle
 * @param[in] function - req type
 * @param[in] command - command
 * @param[in] objtype - object type
 * @param[in] objname - object name
 * @param[in] aoplp - attribute list
 * @param[in] extend - extend string for req
 *
 * @return	int
 * @retval	0	success
 * @retval	!0	error
 *
 */
int
PBSD_manager(int c, int function, int command, int objtype, char *objname, struct attropl *aoplp, char *extend)
{
	int i;
	struct batch_reply *reply;
	int rc;

	/* initialize the thread context data, if not initialized */
	if (pbs_client_thread_init_thread_context() != 0)
		return pbs_errno;

	/* verify the object name if creating a new one */
	if (command == MGR_CMD_CREATE)
		if (pbs_verify_object_name(objtype, objname) != 0)
			return pbs_errno;

	/* now verify the attributes, if verification is enabled */
	if ((pbs_verify_attributes(c, function, objtype, command, aoplp)) != 0)
		return pbs_errno;

	/* lock pthread mutex here for this connection */
	/* blocking call, waits for mutex release */
	if (pbs_client_thread_lock_connection(c) != 0)
		return pbs_errno;

	/* send the manage request */
	i = PBSD_mgr_put(c, function, command, objtype, objname, aoplp, extend, PROT_TCP, NULL);
	if (i != PBSE_NONE) {
		(void)pbs_client_thread_unlock_connection(c);
		return i;
	}

	/* read reply from stream into presentation element */
	reply = PBSD_rdrpy(c);
	PBSD_FreeReply(reply);
	rc = get_conn_errno(c);

	/* unlock the thread lock and update the thread context data */
	if (pbs_client_thread_unlock_connection(c) != 0)
		return pbs_errno;

	return rc;
}
