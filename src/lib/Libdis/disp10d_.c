/*
 * Copyright (C) 1994-2020 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of both the OpenPBS software ("OpenPBS")
 * and the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * OpenPBS is free software. You can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * OpenPBS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * PBS Pro is commercially licensed software that shares a common core with
 * the OpenPBS software.  For a copy of the commercial license terms and
 * conditions, go to: (http://www.pbspro.com/agreement.html) or contact the
 * Altair Legal Department.
 *
 * Altair's dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of OpenPBS and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair's trademarks, including but not limited to "PBS™",
 * "OpenPBS®", "PBS Professional®", and "PBS Pro™" and Altair's logos is
 * subject to Altair's trademark licensing policies.
 */

#include <pbs_config.h>   /* the master config generated by configure */

#include <math.h>
#include "dis_.h"
/**
 * @file	disp10d_.c
 */
/**
 * @brief
 *		  expon
 *	Returns 10	as a double precision value.
 *
 * @param[in] expon - exponant value
 *
 * @return	double
 * @retval	10^expon value	success
 * @retval	0.0		error
 *
 */

double
disp10d_(int expon)
{
	int		negate;
	int		pow2;
	double		accum;

	if (expon == 0)
		return (1.0);

	/* dis_dmx10 would be initialized by prior call to dis_init_tables */

	if (expon < 0) {
		expon = -expon;
		negate = TRUE;
	}
	else {
		negate = FALSE;
	}
	pow2 = 0;
	do {
		if (expon & 1) {
			accum = dis_dp10[pow2];
			while (expon >>= 1) {
				if (++pow2 > dis_dmx10)
					return (negate ? 0.0 : HUGE_VAL);
				if (expon & 1)
					accum *= dis_dp10[pow2];
			}
			return (negate ? 1.0 / accum : accum);
		}
		expon >>= 1;
	} while (pow2++ < dis_dmx10);
	return (negate ? 0.0 : HUGE_VAL);
}