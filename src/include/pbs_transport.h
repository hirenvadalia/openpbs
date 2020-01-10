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
#ifndef	_DIS_H
#define	_DIS_H
#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <limits.h>
#include <float.h>
#include "Long.h"
#include "pbs_gss.h"

extern void set_transport_to_tcp();

typedef struct pbs_transport_chan {
	int (*transport_after_recv)(int);
	int (*transport_before_send)(int, void *, int);
	void *extra;
} pbs_transport_chan_t;

void transport_chan_set_extra(int, void *);
void * transport_chan_get_extra(int);
void transport_chan_set_before_send(int, void *);
void * transport_chan_get_before_send(int);
void transport_chan_set_after_recv(int, void *);
void * transport_chan_get_after_recv(int);
int transport_flush(int);
void transport_setup_chan(int, pbs_transport_chan_t * (*)(int));
void transport_destroy_chan(int);

pbs_transport_chan_t * (*pfn_transport_get_chan)(int);
int (*pfn_transport_set_chan)(int, pbs_transport_chan_t *);
void (*pfn_transport_chan_free_extra)(void *);
int (*pfn_transport_recv)(int, void *, int);
int (*pfn_transport_send)(int, void *, int);

#define transport_recv(x, y, z) (*pfn_transport_recv)(x, y, z)
#define transport_send(x, y, z) (*pfn_transport_send)(x, y, z)
#define transport_get_chan(x) (*pfn_transport_get_chan)(x)
#define transport_set_chan(x, y) (*pfn_transport_set_chan)(x, y)
#define transport_chan_free_extra(x) \
	do { \
		if (pfn_transport_chan_free_extra != NULL) { \
			(*pfn_transport_chan_free_extra)(x); \
		} \
	} while(0)

#ifdef	__cplusplus
}
#endif
#endif	/* _DIS_H */
