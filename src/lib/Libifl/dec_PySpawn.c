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
 * 	decode python spawn batch request
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
wire_decode_batch_pyspawn(void *buf, breq *request)
{
	int i;
	size_t len;
	flatbuffers_string_vec_t strs;
	ns(Spawn_table_t) B = (ns(Spawn_table_t))buf;

	COPYSTR_B(request->rq_ind.rq_py_spawn.rq_jid, ns(Spawn_jobId(B)));

	request->rq_ind.rq_py_spawn.rq_argv = NULL;
	request->rq_ind.rq_py_spawn.rq_envp = NULL;

	strs = ns(Spawn_argv(B));
	len = flatbuffers_string_vec_len(strs);
	request->rq_ind.rq_py_spawn.rq_argv = (char **)calloc(sizeof(char **), len + 1);
	if (request->rq_ind.rq_py_spawn.rq_argv == NULL)
		return PBSE_SYSTEM;
	for (i = 0; i < len; i++) {
		COPYSTR_S(request->rq_ind.rq_py_spawn.rq_argv[i], flatbuffers_string_vec_at(strs, i));
	}

	strs = ns(Spawn_envp(B));
	len = flatbuffers_string_vec_len(strs);
	request->rq_ind.rq_py_spawn.rq_envp = (char **)calloc(sizeof(char **), len + 1);
	if (request->rq_ind.rq_py_spawn.rq_envp == NULL)
		return PBSE_SYSTEM;
	for (i = 0; i < len; i++) {
		COPYSTR_S(request->rq_ind.rq_py_spawn.rq_envp[i], flatbuffers_string_vec_at(strs, i));
	}
}
