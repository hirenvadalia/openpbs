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
#include "ifl_internal.h"
#include "pbs_error.h"

flatbuffers_ref_t __encode_wire_attrl(void *B, struct attropl *p, int use_op);

/**
 * @brief-
 *	helper function for encode attrl or attrol
 *
 * @param[in] B - pointer to flatbuffer
 * @param[in] p - pointer to attropl structure
 * @param[in] use_op - whether to set op from given attropl or force to SET
 *
 * @return flatbuffers_ref_t
 * @retval flatbuffer vec ref
 *
 */
flatbuffers_ref_t
__encode_wire_attrl(void *B, struct attropl *p, int use_op)
{
	struct attropl *ps = NULL;

	ns(Attribute_vec_start(B));

	for (ps = p; ps; ps = ps->next) {
		ns(Attribute_start(B));
		ns(Attribute_name_add(B, flatbuffers_string_create_str(B, ps->name)));
		if (ps->resource) {
			ns(Attribute_resource_add(B, flatbuffers_string_create_str(B, ps->resource)));
		}
		ns(Attribute_value_add(B, flatbuffers_string_create_str(B, ps->value)));
		if (use_op)
			ns(Attribute_op_add(B, (unsigned int)ps->op));
		else
			ns(Attribute_op_add(B, SET));
		ns(Attribute_vec_push(B, ns(Attribute_end(B))));
	}

	return ((ns(Attribute_vec_end(B))));
}

/**
 * @brief-
 *	encode a list of PBS API "attrl" structures into given flatbuffer
 *
 * @param[in] B - pointer to flatbuffer
 * @param[in] pattrl - pointer to attrl structure
 *
 * @return flatbuffers_ref_t
 * @retval flatbuffer vec ref
 *
 */
flatbuffers_ref_t
encode_wire_attrl(void *B, struct attrl *pattrl)
{
	return __encode_wire_attrl(B, (struct attropl *)pattrl, 0);
}

/**
 * @brief-
 *	encode a list of PBS API "attropl" structures into given flatbuffer
 *
 * @param[in] B - pointer to flatbuffer
 * @param[in] pattrl - pointer to attropl structure
 *
 * @return flatbuffers_ref_t
 * @retval flatbuffer vec ref
 *
 */
flatbuffers_ref_t
encode_wire_attropl(void *B, struct attropl *pattropl)
{
	return __encode_wire_attrl(B, pattropl, 1);
}

/**
 * @brief
 *	decode into a list of PBS API "attrl" structures from given flatbuffer
 *	The space for the attrl structures is allocated as needed.
 *
 * @param[in] attrs - flatbuffer attrs vec
 * @param[in] ppatt - pointer to list of attributes
 *
 * @return int
 * @retval 0 on SUCCESS
 * @retval >0 on failure
 */
int
decode_wire_attrl(ns(Attribute_vec_t) attrs, struct attrl **ppatt)
{
	int i = 0;
	struct attrl *patprior = NULL;
	struct attrl *pat = NULL;
	size_t attrs_len = ns(Attribute_vec_len(attrs));

	for (i = 0; i < attrs_len; i++) {
		ns(Attribute_table_t) attr = ns(Attribute_vec_at(attrs, i));
		pat = new_attrl();
		if (pat == NULL)
			return PBSE_SYSTEM;

		pat->name = strdup((char *)ns(Attribute_name(attr)));
		if (pat->name != NULL)
			goto err;

		if (ns(Attribute_resc_is_present(attr))) {
			pat->resource = strdup((char *)ns(Attribute_resc(attr)));
			if (pat->resource != NULL)
				goto err;
		}

		pat->value = strdup((char *)ns(Attribute_value(attr)));
		if (pat->value != NULL)
			goto err;
		pat->op = (enum batch_op) ns(Attribute_op(attr));
		if (i == 0) {
			/* first one, link to passing in pointer */
			*ppatt = pat;
		} else {
			patprior->next = pat;
		}
		patprior = pat;
	}

err:
	PBS_free_aopl((struct attropl *)pat);
	return PBSE_SYSTEM;
}
