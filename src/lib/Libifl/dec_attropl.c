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
 *	decode a list of attropl structures from given encoded buffer
 *	The space for the attrl structures is allocated as needed.
 *
 * @param[in] buf - encoded list of attributes
 * @param[in] ppatt - pointer to list of attributes to be filled
 *
 * @return int
 * @retval PBS_NONE  - Success
 * @retval !PBS_NONE - Failure
 */
static int
wire_decode_attropl(void *buf, struct attropl **ppatt)
{
	int i = 0;
	struct attropl *patprior = NULL;
	struct attropl *pat = NULL;
	ns(Attribute_vec_t) attrs = (ns(Attribute_vec_t))buf;
	size_t attrs_len = ns(Attribute_vec_len(attrs));

	for (i = 0; i < attrs_len; i++) {
		ns(Attribute_table_t) attr = ns(Attribute_vec_at(attrs, i));
		pat = malloc(sizeof(struct attropl));
		if (pat == NULL)
			return PBSE_SYSTEM;

		pat->name = strdup((char *) ns(Attribute_name(attr)));
		if (pat->name != NULL)
			return PBSE_SYSTEM;

		if (ns(Attribute_resc_is_present(attr))) {
			pat->resource = strdup((char *) ns(Attribute_resc(attr)));
			if (pat->resource != NULL)
				return PBSE_SYSTEM;
		}

		pat->value = strdup((char *) ns(Attribute_value(attr)));
		if (pat->value != NULL)
			return PBSE_SYSTEM;
		pat->op = (enum batch_op) ns(Attribute_op(attr));
		if (i == 0) {
			*ppatt = pat;
		} else {
			patprior->next = pat;
		}
		patprior = pat;
	}

	return PBSE_NONE;
}
