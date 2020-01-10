/*
 * Copyright (C) 1994-2019 Altair Engineering, Inc.
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
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "pbs_transport.h"

/**
 * @brief
 * 	transport_chan_set_before_send - set func to be called before transport send
 *
 * @param[in] fd - file descriptor
 * @param[in] func - func to set as before transport send
 *
 * @return void
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void
transport_chan_set_before_send(int fd, void *func)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->transport_before_send = func;
}

/**
 * @brief
 * 	transport_chan_get_before_send - get func to be called before transport sending data
 *
 * @param[in] fd - file descriptor
 *
 * @return void *
 *
 * @retval !NULL - success
 * @retval NULL - error
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void *
transport_chan_get_before_send(int fd)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return NULL;
	return chan->transport_before_send;
}

/**
 * @brief
 * 	transport_chan_set_after - set func to be called after transport receive data
 *
 * @param[in] fd - file descriptor
 * @param[in] func - func to set as after transport receive
 *
 * @return void
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void
transport_chan_set_after_recv(int fd, void *func)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->transport_after_recv = func;
}

/**
 * @brief
 * 	transport_chan_get_after_recv - get func to be called after transport receiving data
 *
 * @param[in] fd - file descriptor
 *
 * @return void *
 *
 * @retval !NULL - success
 * @retval NULL - error
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void *
transport_chan_get_after_recv(int fd)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return NULL;
	return chan->transport_after_recv;
}

/**
 * @brief
 * 	transport_chan_set_extra - associates optional structure with connection
 *
 * @param[in] fd - file descriptor
 * @param[in] extra - the structure for association
 *
 * @return void
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void
transport_chan_set_extra(int fd, void *extra)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->extra = extra;
}

/**
 * @brief
 * 	transport_chan_get_extra - gets optional structure associated with connection
 *
 * @param[in] fd - file descriptor
 *
 * @return void *
 *
 * @retval !NULL - success
 * @retval NULL - error
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void *
transport_chan_get_extra(int fd)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return NULL;
	return chan->extra;
}

/**
 * @brief
 * 	__transport_read - read data from connection to "fill" the buffer
 *	Update the various buffer pointers.
 *
 * @param[in] fd - socket descriptor
 *
 * @return	int
 *
 * @retval	>0 	number of characters read
 * @retval	0 	if EOD (no data currently avalable)
 * @retval	-1 	if error
 * @retval	-2 	if EOF (stream closed)
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
int
__transport_read(int fd)
{
	int i;
	int (*after_recv)(int) = transport_chan_get_after_recv(fd);
	pbs_dis_buf_t *tp = dis_get_readbuf(fd);

	if (tp == NULL)
		return -1;
	dis_pack_buf(tp);
	dis_resize_buf(tp, PBS_DIS_BUFSZ, 0);
	i = transport_recv(fd, &(tp->tdis_thebuf[tp->tdis_eod]), tp->tdis_bufsize - tp->tdis_eod);
	if (i > 0) {
		tp->tdis_eod += i;
		if (after_recv != NULL) {
			if (after_recv(fd) == -1) {
				return -1;
			}
		}
	}
	return i;
}

/**
 * @brief
 *	flush dis write buffer
 *
 *	Writes "committed" data in buffer to file descriptor,
 *	packs remaining data (if any), resets pointers
 *
 * @param[in] - fd - file descriptor
 *
 * @return int
 *
 * @retval  0 on success
 * @retval -1 on error
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
int
transport_flush(int fd)
{
	int (*before_send)(int, void *, int) = transport_chan_get_before_send(fd);
	pbs_dis_buf_t *tp = dis_get_writebuf(fd);

	if (tp == NULL)
		return -1;
	if (tp->tdis_trail == 0)
		return 0;
	if (before_send != NULL) {
		if (before_send(fd, tp->tdis_thebuf, tp->tdis_trail) == -1)
			return -1;
		if (tp->tdis_trail == 0)
			return 0;
	}
	if (transport_send(fd, tp->tdis_thebuf, tp->tdis_trail) == -1) {
		return (-1);
	}
	tp->tdis_eod = tp->tdis_lead;
	dis_pack_buf(tp);
	return 0;
}

/**
 * @brief
 * 	transport_destroy_chan - release structures associated with fd
 *
 * @param[in] fd - socket descriptor
 *
 * @return void
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void
transport_destroy_chan(int fd)
{
	pbs_transport_chan_t *chan = transport_get_chan(fd);
	if (chan != NULL) {
		if (chan->extra) {
			transport_chan_free_extra(chan->extra);
			chan->extra = NULL;
		}
		free(chan);
		transport_set_chan(fd, NULL);
	}
}


/**
 * @brief
 *	allocate dis buffers associated with connection, if already allocated then clear it
 *
 * @param[in] fd - file descriptor
 *
 * @return void
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
void
transport_setup_chan(int fd, pbs_transport_chan_t * (*inner_transport_get_chan)(int))
{
	pbs_transport_chan_t *chan;
	int rc;

	/* check for bad file descriptor */
	if (fd < 0)
		return;
	chan = (pbs_transport_chan_t *)(*inner_transport_get_chan)(fd);
	if (chan == NULL) {
		if (errno == ENOTCONN)
			return;
		chan = (pbs_transport_chan_t *) calloc(1, sizeof(pbs_transport_chan_t));
		assert(chan != NULL);
		rc = transport_set_chan(fd, chan);
		assert(rc == 0);
	}
}
