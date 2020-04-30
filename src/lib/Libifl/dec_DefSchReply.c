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
 * 	decode Deffered sched reply batch request
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
wire_decode_defschreply(void *buf, breq *request)
{
	ns(SchedDefRep_table_t) B = (ns(SchedDefRep_table_t))buf;

	request->rq_ind.rq_defrpy.rq_cmd = (int) ns(SchedDefRep_cmd(B));
	request->rq_ind.rq_defrpy.rq_err = (int) ns(SchedDefRep_err(B));
	COPYSTR_S(request->rq_ind.rq_defrpy.rq_id, ns(SchedDefRep_id(B)));
	request->rq_ind.rq_defrpy.rq_txt = NULL;
	if (SchedDefRep_text_is_present(B))
		COPYSTR_S(request->rq_ind.rq_defrpy.rq_txt, ns(SchedDefResp_text(B)));

	return PBSE_NONE;
}
