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
#include <arpa/inet.h>
#ifdef WIN32
#include <winsock.h>
#endif
#include "pbs_error.h"
#include "pbs_internal.h"
#include "auth.h"

static int transport_chan_is_encrypted(int);

/**
 * @brief
 * 	transport_chan_set_ctx_status - set auth context status tcp chan assosiated with given fd
 *
 * @param[in] fd - file descriptor
 * @param[in] status - auth ctx status
 * @param[in] for_encrypt - is authctx for encrypt/decrypt?
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
transport_chan_set_ctx_status(int fd, int status, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->auths[for_encrypt].ctx_status = status;
}

/**
 * @brief
 * 	transport_chan_get_ctx_status - get auth context status tcp chan assosiated with given fd
 *
 * @param[in] fd - file descriptor
 * @param[in] for_encrypt - whether to get encrypt/decrypt authctx status or for authentication
 *
 * @return int
 *
 * @retval -1 - error
 * @retval !-1 - status
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
int
transport_chan_get_ctx_status(int fd, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return -1;
	return chan->auths[for_encrypt].ctx_status;
}

/**
 * @brief
 * 	transport_chan_set_authctx - associates authenticaion context with connection
 *
 * @param[in] fd - file descriptor
 * @param[in] authctx - the context
 * @param[in] for_encrypt - is authctx for encrypt/decrypt?
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
transport_chan_set_authctx(int fd, void *authctx, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->auths[for_encrypt].ctx = authctx;
}

/**
 * @brief
 * 	transport_chan_get_authctx - gets authentication context associated with connection
 *
 * @param[in] fd - file descriptor
 * @param[in] for_encrypt - whether to get encrypt/decrypt authctx or for authentication
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
transport_chan_get_authctx(int fd, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return NULL;
	return chan->auths[for_encrypt].ctx;
}

/**
 * @brief
 * 	transport_chan_set_authdef - associates authdef structure with connection
 *
 * @param[in] fd - file descriptor
 * @param[in] authdef - the authdef structure for association
 * @param[in] for_encrypt - is authdef for encrypt/decrypt?
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
transport_chan_set_authdef(int fd, auth_def_t *authdef, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return;
	chan->auths[for_encrypt].def = authdef;
}

/**
 * @brief
 * 	transport_chan_get_authdef - gets authdef structure associated with connection
 *
 * @param[in] fd - file descriptor
 * @param[in] for_encrypt - whether to get encrypt/decrypt authdef or for authentication
 *
 * @return auth_def_t *
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
auth_def_t *
transport_chan_get_authdef(int fd, int for_encrypt)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return NULL;
	return chan->auths[for_encrypt].def;
}


/**
 * @brief
 * 	transport_chan_is_encrypted - is chan assosiated with given fd is encrypted?
 *
 * @param[in] fd - file descriptor
 *
 * @return int
 *
 * @retval 0 - not encrypted
 * @retval 1 - encrypted
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
static int
transport_chan_is_encrypted(int fd)
{
	pbs_tcp_chan_t *chan = transport_get_chan(fd);
	if (chan == NULL)
		return 0;
	return (chan->auths[FOR_ENCRYPT].def != NULL && chan->auths[FOR_ENCRYPT].ctx_status == AUTH_STATUS_CTX_READY);
}

/**
 * @brief
 * 	create_pkt - create packet based on given value
 *
 * @param[in] type - type of pkt
 * @param[in] data - data of pkt
 * @param[in] len - length of data
 * @param[out] pkt - generated pkt
 * @param[out] pkt_len - length of generated pkt
 *
 * @return int
 *
 * @retval 0  - success
 * @retval -1 - failure
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
static int
create_pkt(int type, void *data, size_t len, void **pkt, size_t *pkt_len)
{
	int ndlen = 0;
	void *_pkt = NULL;
	char *pos = NULL;
	size_t pktlen = 1 + sizeof(int) + len;

	*pkt = NULL;
	*pkt_len = 0;

	_pkt = malloc(pktlen);
	if (_pkt == NULL)
		return -1;
	pos = (char *)_pkt;
	*pos++ = (char)type;
	ndlen = htonl(len);
	memcpy(pos, &ndlen, sizeof(int));
	pos += sizeof(int);
	memcpy(pos, data, len);
	*pkt = _pkt;
	*pkt_len = pktlen;
	return 0;
}

/**
 * @brief
 * 	parse_pkt - parse given pkt into type, data and data length
 *
 * @param[in] pkt - pkt to be parsed
 * @param[in] pkt_len - length of pkt
 * @param[out] type - type of pkt
 * @param[out] data - data of pkt
 * @param[out] len - length of data
 *
 * @return int
 *
 * @retval 0  - success
 * @retval -1 - failure
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
static int
parse_pkt(void *pkt, size_t pkt_len, int *type, void **data_out, size_t *len_out)
{
	char *pos = (char *)pkt;

	*type = *((unsigned char *)pos);
	pos++;
	*len_out = ntohl(*((int *)pos));
	if (*len_out != (pkt_len - 1 - sizeof(int))) {
		*type = 0;
		*data_out = NULL;
		*len_out = 0;
		return -1;
	}
	pos += sizeof(int);
	*data_out = malloc(*len_out);
	if (data_out == NULL) {
		*type = 0;
		*len_out = 0;
		return -1;
	}
	memcpy(*data_out, pos, *len_out);
	return 0;
}

/**
 * @brief
 * 	transport_send_pkt - create pkt based on given value
 * 	and send it over network. If channel for given fd is
 * 	encrypted then pkt (with given data and type) will be
 * 	encrypted then AUTH_ENCRYPTED_DATA pkt will be sent
 *
 * @param[in] fd - file descriptor
 * @param[in] type - type of pkt
 * @param[in] data_in - data of pkt
 * @param[in] len_in - length of data
 *
 * @return int
 *
 * @retval >= 0  - success
 * @retval -1 - failure
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
int
transport_send_pkt(int fd, int type, void *data_in, size_t len_in)
{
	int i = 0;
	void *pkt = NULL;
	size_t pktlen = 0;

	if (transport_chan_is_encrypted(fd)) {
		void *authctx = transport_chan_get_authctx(fd, FOR_ENCRYPT);
		auth_def_t *authdef = transport_chan_get_authdef(fd, FOR_ENCRYPT);
		void *data_out = NULL;
		size_t len_out = 0;

		if (data_in == NULL || len_in == 0 || authdef == NULL || authdef->encrypt_data == NULL)
			return -1;

		if (create_pkt(type, data_in, len_in, &pkt, &pktlen) != 0)
			return -1;

		if (authdef->encrypt_data(authctx, pkt, pktlen, &data_out, &len_out) != 0) {
			free(pkt);
			return -1;
		}

		free(pkt);

		if (pktlen <= 0) {
			free(data_out);
			return -1;
		}

		if (create_pkt(AUTH_ENCRYPTED_DATA, data_out, len_out, &pkt, &pktlen) != 0) {
			free(data_out);
			return -1;
		}
		free(data_out);
	} else {
		if (create_pkt(type, data_in, len_in, &pkt, &pktlen) != 0) {
			return -1;
		}
	}

	i = transport_send(fd, pkt, pktlen);
	free(pkt);
	if (i > 0 && i != pktlen)
		return -1;
	return i;
}

/**
 * @brief
 * 	transport_recv_pkt - receive pkt over network.
 * 	If channel for given fd is encrypted then we
 * 	will get AUTH_ENCRYPTED_DATA pkt (if not then its error)
 * 	once AUTH_ENCRYPTED_DATA pkt is received, decrypt it
 * 	to find actual unencrypted pkt
 *
 * @param[in] fd - file descriptor
 * @param[out] type - type of pkt
 * @param[out] data_out - data of pkt
 * @param[out] len_out - length of data
 *
 * @return int
 *
 * @retval >= 0  - success
 * @retval -1 - failure
 *
 * @par Side Effects:
 *	None
 *
 * @par MT-safe: Yes
 *
 */
int
transport_recv_pkt(int fd, int *type, void **data_out, size_t *len_out)
{
	int i = 0;
	char ndbuf[sizeof(int)];
	int ndlen = 0;
	size_t data_in_sz = 0;
	void *data_in = NULL;

	*type = 0;
	*data_out = NULL;
	*len_out = 0;

	i = transport_recv(fd, (void *)type, 1);
	if (i != 1)
		return i;

	i = transport_recv(fd, (void *)&ndbuf, sizeof(int));
	if (i != sizeof(int))
		return i;
	memcpy(&ndlen, (void *)&ndbuf, sizeof(int));
	data_in_sz = ntohl(ndlen);
	if (data_in_sz <= 0) {
		return -1;
	}

	data_in = malloc(data_in_sz);
	if (data_in == NULL) {
		return -1;
	}
	i = transport_recv(fd, data_in, data_in_sz);
	if (i != data_in_sz) {
		free(data_in);
		return (i < 0 ? i : -1);
	}

	if (transport_chan_is_encrypted(fd)) {
		void *authctx = transport_chan_get_authctx(fd, FOR_ENCRYPT);
		auth_def_t *authdef = transport_chan_get_authdef(fd, FOR_ENCRYPT);
		void *data = NULL;
		size_t datasz = 0;

		if (*type != AUTH_ENCRYPTED_DATA) {
			free(data_in);
			return -1;
		}

		if (authdef == NULL || authdef->decrypt_data == NULL) {
			free(data_in);
			return -1;
		}

		if (authdef->decrypt_data(authctx, data_in, data_in_sz, &data, &datasz) != 0) {
			free(data_in);
			return -1;
		}

		free(data_in);
		if (parse_pkt(data, datasz, type, &data_in, &data_in_sz) != 0) {
			free(data);
			return -1;
		}
		free(data);
	}
	*data_out = data_in;
	*len_out = data_in_sz;
	return data_in_sz;
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
dis_flush(int fd, void *buf)
{
	size_t len = 0;

	// FIXME: get flatcc aligned buffer

	/* DIS doesn't have pkt type, pass 0 always for type in transport_send_pkt */
	if (transport_send_pkt(fd, 0, buf, len) <= 0)
		return -1;
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
	pbs_tcp_chan_t *chan = NULL;

	if (pfn_transport_get_chan == NULL)
		return;
	chan = transport_get_chan(fd);
	if (chan != NULL) {
		if (chan->auths[FOR_AUTH].ctx || chan->auths[FOR_ENCRYPT].ctx) {
			/* DO NOT free authdef here, it will be done in unload_auths() */
			if (chan->auths[FOR_AUTH].ctx && chan->auths[FOR_AUTH].def) {
				chan->auths[FOR_AUTH].def->destroy_ctx(chan->auths[FOR_AUTH].ctx);
			}
			if (chan->auths[FOR_ENCRYPT].def != chan->auths[FOR_AUTH].def &&
				chan->auths[FOR_ENCRYPT].ctx &&
				chan->auths[FOR_ENCRYPT].def) {
				chan->auths[FOR_ENCRYPT].def->destroy_ctx(chan->auths[FOR_ENCRYPT].ctx);
			}
			chan->auths[FOR_AUTH].ctx = NULL;
			chan->auths[FOR_AUTH].def = NULL;
			chan->auths[FOR_AUTH].ctx_status = AUTH_STATUS_UNKNOWN;
			chan->auths[FOR_ENCRYPT].ctx = NULL;
			chan->auths[FOR_ENCRYPT].def = NULL;
			chan->auths[FOR_ENCRYPT].ctx_status = AUTH_STATUS_UNKNOWN;
		}
		free(chan);
		transport_set_chan(fd, NULL);
	}
}


/**
 * @brief
 *	allocate (if not) chan structures associated with connection
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
transport_setup_chan(int fd, pbs_tcp_chan_t * (*inner_transport_get_chan)(int))
{
	pbs_tcp_chan_t *chan;
	int rc;

	/* check for bad file descriptor */
	if (fd < 0)
		return;
	chan = (pbs_tcp_chan_t *)(*inner_transport_get_chan)(fd);
	if (chan == NULL) {
		if (errno == ENOTCONN)
			return;
		chan = (pbs_tcp_chan_t *) calloc(1, sizeof(pbs_tcp_chan_t));
		assert(chan != NULL);
		rc = transport_set_chan(fd, chan);
		assert(rc == 0);
	}
}
