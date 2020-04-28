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
 *	decode a list of svrattrl structures from given encoded buffer
 *	The space for the svrattrl structures is allocated as needed.
 *
 * @param[in] buf - encoded list of attributes
 * @param[in] phead - pointer to list of attributes to be filled
 *
 * @return int
 * @retval PBS_NONE  - success
 * @retval !PBS_NONE - failure
 */
int
wire_decode_svrattrl(void *buf, pbs_list_head *phead)
{
	ns(Attribute_vec_t) attrs = (ns(Attribute_vec_t))buf;
	unsigned int hasresc;
	size_t ls;
	unsigned int data_len;
	unsigned int numattr;
	int rc;
	size_t tsize;
	int i = 0;
	size_t attrs_len = ns(Attribute_vec_len(attrs));

	CLEAR_HEAD((*phead));

	for (i = 0; i < attrs_len; i++) {
		ns(Attribute_table_t) attr = ns(Attribute_vec_at(attrs, i));
		svrattrl *psvrat = NULL;
		char *name = (char *) ns(Attribute_name(attr));
		size_t nlen = strlen(name) + 1;
		char *value = (char *) ns(Attribute_value(attr));
		size_t vlen = strlen(value) + 1;
		char *resc = NULL;
		size_t rlen = 0;
		size_t tsize = sizeof(svrattrl) + nlen + vlen;

		if (ns(Attribute_resc_is_present(attr))) {
			resc = (char *) ns(Attribute_resc(attr));
			rlen = strlen(resc) + 1;
			tsize += rlen;
		}

		if ((psvrat = (svrattrl *)malloc(tsize)) == NULL)
			return PBSE_SYSTEM;

		CLEAR_LINK(psvrat->al_link);
		psvrat->al_sister = NULL;
		psvrat->al_atopl.next = NULL;
		psvrat->al_tsize = tsize;
		psvrat->al_name = (char *)psvrat + sizeof(svrattrl);
		psvrat->al_resc = psvrat->al_name + nlen;
		psvrat->al_value = psvrat->al_resc + rlen;
		psvrat->al_nameln = nlen;
		psvrat->al_rescln = rlen;
		psvrat->al_valln = vlen;
		psvrat->al_flags = 0;
		psvrat->al_refct = 1;
		psvrat->al_op = (enum batch_op) ns(Attribute_op(attr));

		COPYSTR(psvrat->al_name, name, nlen);
		if (rlen > 0) {
			COPYSTR(psvrat->al_resc, resc, rlen);
		}
		COPYSTR(psvrat->al_value, value, vlen);
		append_link(phead, &(psvrat->al_link), psvrat);
	}
	return PBSE_NONE;
}
