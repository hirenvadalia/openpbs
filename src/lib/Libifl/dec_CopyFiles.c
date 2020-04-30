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

#include "pbs_wire.h"

/**
 * @brief
 * 	decode a Copy Files Batch Request
 *
 *	This request is used by the server ONLY.
 *	The rq_cpyfile structure pointed to by pcf must already exist.
 *
 * @param[in] B - copy file table ref
 * @param[in] pcf - pointer to rq_cpyfile structure
 *
 * @return int
 * @retval PBS_NONE  - success
 * @retval !PBS_NONE - failure
 */
static int
__wire_decode_copyfiles_helper(ns(CopyFile_table_t) B, struct rq_cpyfile *pcf)
{
	int pair_ct = 0;
	struct rqfpair *ppair = NULL;
	ns(FilePair_vec_t) pairs;
	int i = 0;

	CLEAR_HEAD(pcf->rq_pair);

	COPYSTR_B(pcf->rq_jobid, ns(CopyFile_jobId(B)));
	COPYSTR_B(pcf->rq_owner, ns(CopyFile_owner(B)));
	COPYSTR_B(pcf->rq_user, ns(CopyFile_user(B)));
	COPYSTR(pcf->rq_group, ns(CopyFile_group(B)), sizeof(pcf->rq_group));

	pcf->rq_dir = (int) ns(CopyFile_flags(B));

	pairs = ns(CopyFile_pairs(B));
	pair_ct = (int) ns(FilePair_vec_len(pairs));

	for (i = 0; i < pair_ct; i++) {
		ns(FilePair_table_t) pair = ns(FilePair_vec_at(pairs, i));

		ppair = (struct rqfpair *) calloc(sizeof(struct rqfpair), 1);
		if (ppair == NULL)
			return PBSE_SYSTEM;

		CLEAR_LINK(ppair->fp_link);

		ppair->fp_flag = (int) ns(FilePair_flag(pair));

		ppair->fp_rmt = strdup((char *) ns(FilePair_remote(pair)));
		if (ppair->fp_rmt == NULL) {
			free(ppair);
			return PBSE_SYSTEM;
		}

		ppair->fp_local = strdup((char *) ns(FilePair_local(pair)));
		if (ppair->fp_local == NULL) {
			free(ppair->fp_rmt);
			free(ppair);
			return PBSE_SYSTEM;
		}

		append_link(&(pcf->rq_pair), &(ppair->fp_link), ppair);
	}
	return PBSE_NONE;
}

/**
 * @brief
 * 	decode a copy files batch request
 *
 * @param[in] buf - encoded request
 * @param[in] request - pointer to request structure
 *
 * @return int
 * @retval PBS_NONE  - success
 * @retval !PBS_NONE - failure
 *
 */
int
wire_decode_copyfiles(void *buf, breq *request)
{
	ns(CopyHook_table_t) B = (ns(CopyHook_table_t))buf;

	return __wire_decode_copyfiles_helper(B, &(request->rq_ind.rq_cpyfile));
}

/**
 * @brief
 * 	decode a copy files with cred batch request
 *
 * @param[in] buf - encoded request
 * @param[in] request - pointer to request structure
 *
 * @return int
 * @retval PBS_NONE  - success
 * @retval !PBS_NONE - failure
 *
 */
int
wire_decode_copyfiles_cred(void *buf, breq *request)
{
	flatbuffers_string_t cred;
	ns(CopyFileCred_table_t) B = (ns(CopyFileCred_table_t))buf;

	request->rq_ind.rq_cpyfile_cred.rq_credtype = (int) ns(CopyFileCred_type(B));
	cred = ns(CopyFileCred_cred(B));
	request->rq_ind.rq_cpyfile_cred.rq_credlen = flatbuffers_string_len(cred);
	COPYSTR_M(request->rq_ind.rq_cpyfile_cred.rq_pcred, cred, request->rq_ind.rq_cpyfile_cred.rq_credlen);
	return __wire_decode_copyfiles_helper(ns(CopyFileCred_files(B)), &(request->rq_ind.rq_cpyfile_cred.rq_copyfile));
}
