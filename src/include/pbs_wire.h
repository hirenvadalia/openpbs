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
#ifndef	_PBS_WIRE_H
#define	_PBS_WIRE_H
#ifdef	__cplusplus
extern "C" {
#endif

#include "auth.h"

typedef struct pbs_tcp_auth_data {
	int ctx_status;
	void *ctx;
	auth_def_t *def;
} pbs_tcp_auth_data_t;

typedef struct pbs_tcp_chan {
	pbs_tcp_auth_data_t auths[2];
} pbs_tcp_chan_t;

int dis_flush(int, void *);
void transport_setup_chan(int, pbs_tcp_chan_t * (*)(int));
void transport_destroy_chan(int);

void transport_chan_set_ctx_status(int, int, int);
int transport_chan_get_ctx_status(int, int);
void transport_chan_set_authctx(int, void *, int);
void * transport_chan_get_authctx(int, int);
void transport_chan_set_authdef(int, auth_def_t *, int);
auth_def_t * transport_chan_get_authdef(int, int);
int transport_send_pkt(int, int, void *, size_t);
int transport_recv_pkt(int, int *, void **, size_t *);

pbs_tcp_chan_t * (*pfn_transport_get_chan)(int);
int (*pfn_transport_set_chan)(int, pbs_tcp_chan_t *);
int (*pfn_transport_recv)(int, void *, int);
int (*pfn_transport_send)(int, void *, int);

#define transport_recv(x, y, z) (*pfn_transport_recv)(x, y, z)
#define transport_send(x, y, z) (*pfn_transport_send)(x, y, z)
#define transport_get_chan(x) (*pfn_transport_get_chan)(x)
#define transport_set_chan(x, y) (*pfn_transport_set_chan)(x, y)

#ifdef	__cplusplus
}
#endif
#endif	/* _PBS_WIRE_H */
