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
 * 	Decode the request header gields common to all requests
 *
 * @param[in] buf - buffer which holds encoded request header
 * @param[in] request - pointer to request structure
 * @param[out] proto_type - protocol type
 * @param[out] proto_ver - protocol version
 *
 * @return int
 * @retval 0  success
 * @retval >0 failure (a PBSE_ number)
 *
 */
int
wire_decode_batch_reqhdr(void *buf, breq *request, int *proto_type, int *proto_ver)
{
	ns(Header_table_t) hdr = ns(Req_hdr((ns(Req_table_t))buf));

	*proto_type = (int)ns(Header_protType(hdr));
	*proto_ver = (int)ns(Header_version(hdr));
	request->rq_type = (int)ns(Header_reqId(hdr));
	COPYSTR_B(request->rq_user, ns(Header_user(hdr)));

	return PBSE_NONE;
}
