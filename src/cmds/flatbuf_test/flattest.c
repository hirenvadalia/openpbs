#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "dis.h"
#include "libpbs.h"
#include "list_link.h"
#include "server_limits.h"
#include "attribute.h"
#include "log.h"
#include "pbs_error.h"
#include "credential.h"
#include "batch_request.h"
#include "net_connect.h"
#include "pbs_ifl_builder.h"
#include "pbs_ifl_reader.h"

// Convenient namespace macro to manage long namespace prefix.
#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(PBS_ifl, x)

pbs_tcp_chan_t *chan = NULL;
void *sent_data = NULL;
char *sent_data_pos = NULL;

pbs_tcp_chan_t *
get_chan(int fd)
{
	if (chan == NULL) {
		chan = (pbs_tcp_chan_t *) calloc(1, sizeof(pbs_tcp_chan_t));
		chan->readbuf.tdis_thebuf = calloc(1, PBS_DIS_BUFSZ);
		chan->readbuf.tdis_bufsize = PBS_DIS_BUFSZ;
		chan->writebuf.tdis_thebuf = calloc(1, PBS_DIS_BUFSZ);
		chan->writebuf.tdis_bufsize = PBS_DIS_BUFSZ;
		transport_set_chan(fd, chan);
		dis_clear_buf(&(chan->readbuf));
		dis_clear_buf(&(chan->writebuf));
	}
	return chan;
}

int set_chan(int fd, pbs_tcp_chan_t *c)
{
	chan = c;
	return 0;
}

int fake_tcp_send(int fd, void *data, int len)
{
	sent_data = realloc(sent_data, len + 1);
	memcpy(sent_data, data, len);
	sent_data_pos = sent_data;
	return len;
}

int fake_tcp_recv(int fd, void *data, int len)
{
	memcpy(data, sent_data_pos, len);
	sent_data_pos += len;
	return len;
}

struct batch_request *alloc_brp(int type)
{
	struct batch_request *req;

	req= (struct batch_request *)malloc(sizeof(struct batch_request));
	memset((void *)req, (int)0, sizeof(struct batch_request));
	req->rq_type = type;
	CLEAR_LINK(req->rq_link);
	req->rq_conn = -1;		/* indicate not connected */
	req->rq_orgconn = -1;		/* indicate not connected */
	req->rq_time = time(NULL);
	req->tpp_ack = 1; /* enable acks to be passed by tpp by default */
	req->prot = PROT_TCP; /* not tpp by default */
	req->tppcmd_msgid = NULL; /* NULL msgid to boot */
	req->rq_reply.brp_choice = BATCH_REPLY_CHOICE_NULL;
	return (req);
}

void
free_brp(struct batch_request *preq)
{
	(void)free(preq->rq_extend);
	free_attrlist(&preq->rq_ind.rq_queuejob.rq_attr);
	(void)free(preq);
}

void
encode_wire_ReqHdr_fb(flatcc_builder_t *B, int reqt, char *user)
{
	flatbuffers_string_ref_t usr;

	usr = flatbuffers_string_create_str(B, user);
	ns(Header_start(B));
	ns(Header_reqId_add(B, PBS_BATCH_QueueJob));
	ns(Header_protType_add(B, ns(ProtType_Batch)));
	ns(Header_user_add(B, usr));
	ns(Req_hdr_add(B, ns(Header_end(B))));
}

void
encode_wire_ReqExtend_fb(flatcc_builder_t *B, char *extend)
{
	if (extend && extend[0] != '\0')
	{
		ns(Req_extend_add(B, ns(Extend_create(B, flatbuffers_string_create_str(B, extend)))));
	}
}

flatbuffers_ref_t
encode_wire_attropl_fb(flatcc_builder_t *B, struct attropl *pattropl)
{
	struct attropl *ps;
	flatbuffers_string_ref_t name, resc, value;
	ns(batch_op_enum_t) bop;

	ns(Attribute_vec_start(B));

	for (ps = pattropl; ps; ps = ps->next)
	{
		ns(Attribute_start(B));
		name = flatbuffers_string_create_str(B, ps->name);
		ns(Attribute_name_add(B, name));
		if (ps->resource)
		{
			resc = flatbuffers_string_create_str(B, ps->resource);
			ns(Attribute_resc_add(B, resc));
		}
		value = flatbuffers_string_create_str(B, ps->value);
		ns(Attribute_value_add(B, value));

		bop = (int)ps->op;
		ns(Attribute_op_add(B, bop));
		ns(Attribute_ref_t) attr = ns(Attribute_end(B));

		ns(Attribute_vec_push(B, attr));
	}

	return ((ns(Attribute_vec_end(B))));
}

void
encode_wire_QueueJob_fb(flatcc_builder_t *B, char *jobid, char *destin, struct attropl *aoplp)
{
	ns(Attribute_vec_ref_t) attrs = 0;

	if (jobid == NULL)
		jobid = "";
	if (destin == NULL)
		destin = "";

	flatbuffers_string_ref_t jid = flatbuffers_string_create_str(B, jobid);
	flatbuffers_string_ref_t dst = flatbuffers_string_create_str(B, destin);

	attrs = encode_wire_attropl_fb(B, aoplp);

	ns(Qjob_start(B));
	ns(Qjob_jobId_add(B, jid));
	ns(Qjob_destin_add(B, dst));
	ns(Qjob_attrs_add(B, attrs));
	ns(Req_body_add(B, ns(ReqBody_as_Qjob(ns(Qjob_end(B))))));
}

int
decode_wire_svrattrl(ns(Attribute_vec_t) attrs, pbs_list_head *phead)
{
	unsigned int	hasresc;
	size_t		ls;
	unsigned int	data_len;
	unsigned int	numattr;

	int		rc;
	size_t		tsize;

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

		memcpy(psvrat->al_name, name, nlen - 1);
		psvrat->al_name[nlen - 1] = '\0';

		if (rlen > 0) {
			memcpy(psvrat->al_resc, resc, rlen - 1);
			psvrat->al_resc[rlen - 1] = '\0';
		}

		memcpy(psvrat->al_value, value, vlen - 1);
		psvrat->al_value[vlen - 1] = '\0';

		append_link(phead, &(psvrat->al_link), psvrat);
	}
	return 0;
}

void decode_quejob_fb(flatcc_builder_t *B)
{
	int i;
	void *obuf;
	ns(Req_table_t) req;
	ns(Header_table_t) hdr;
	char *resc;
	size_t size;
	struct batch_request *preq;

	obuf = flatcc_builder_finalize_aligned_buffer(B, &size);
	preq = alloc_brp(0);
	preq->rq_conn = 0;

	req = ns(Req_as_root(obuf));
	hdr = ns(Req_hdr(req));

	uint16_t proto = ns(Header_protType(hdr));
	preq->rq_type = ns(Header_reqId(hdr));
	strcpy(preq->rq_user, ns(Header_user(hdr)));

	CLEAR_HEAD(preq->rq_ind.rq_queuejob.rq_attr);
	if (ns(Req_body_type(req)) == ns(ReqBody_Qjob))
	{
		// Cast to appropriate type:
		// C does not require the cast to Weapon_table_t, but C++ does.
		ns(Qjob_table_t) qjob = (ns(Qjob_table_t))ns(Req_body(req));
		ns(Attribute_vec_t) attrs;
		size_t attrs_len;

		strcpy(preq->rq_ind.rq_queuejob.rq_jid, ns(Qjob_jobId(qjob)));
		strcpy(preq->rq_ind.rq_queuejob.rq_destin, ns(Qjob_destin(qjob)));
		decode_wire_svrattrl(ns(Qjob_attrs(qjob)), &(preq->rq_ind.rq_queuejob.rq_attr));
	}

	if (ns(Req_extend_is_present(req))) {
			preq->rq_extend = strdup((char *) ns(Req_extend(req)));
	}
	free(obuf);
	free_brp(preq);
}

char *
PBSD_queuejob_fb(int connect, char *jobid, char *destin, struct attropl *attrib, char *extend, int rpp, char **msgid, int *commit_done)
{
	flatcc_builder_t builder, *B;

	B = &builder;
	flatcc_builder_init(B);
	ns(Req_start_as_root(B));
	encode_wire_ReqHdr_fb(B, PBS_BATCH_QueueJob, "flattest");
	encode_wire_QueueJob_fb(B, jobid, destin, attrib);
	encode_wire_ReqExtend_fb(B, extend);
	ns(Req_end_as_root(B));

	decode_quejob_fb(B);

	return jobid;
}

char *
PBSD_queuejob_dis(int c, char *jobid, char *destin, struct attropl *attrib, char *extend, int rpp, char **msgid, int *commit_done)
{
	int rc;
	int	 proto_type;
	int	 proto_ver;
	struct batch_request *request;


	if ((rc = encode_DIS_ReqHdr(c, PBS_BATCH_QueueJob, "flattest")) ||
		(rc = encode_DIS_QueueJob(c, jobid, destin, attrib)) ||
		(rc = encode_DIS_ReqExtend(c, extend))) {
		return NULL;
	}
	dis_flush(c);

	request = alloc_brp(0);
	request->rq_conn = c;
	decode_DIS_ReqHdr(c, request, &proto_type, &proto_ver);
	decode_DIS_QueueJob(c, request);
	decode_DIS_ReqExtend(c, request);
	free_brp(request);
	return jobid;
}

int main(int argc, char **argv)
{
	char *msgid;
	int commit_done;
	struct attrl *attrib = NULL;
	long st, et;
	struct timeval tval;
	int count = atoi(argv[1]);
	int i;

	pfn_transport_get_chan = get_chan;
	pfn_transport_set_chan = set_chan;
	pfn_transport_recv = fake_tcp_recv;
	pfn_transport_send = fake_tcp_send;

	for (i = 0; i < 20; i++) {
		char t[20] = {'\0'};
		sprintf(t, "testattr_%d", i);
		set_attr(&attrib, t, t);

	}
	for (i = 0; i < 20; i++) {
		char t[20] = {'\0'};
		sprintf(t, "testattrs_%d", i);
		set_attr_resc(&attrib, t, t, t);

	}
	gettimeofday(&tval, NULL);
	st = (tval.tv_sec * 1000L) + (tval.tv_usec / 1000L);
	for (i = 0; i < count; i++)
	{
		PBSD_queuejob_fb(0, "1.server", "someserver", (struct attropl *)attrib, "EX", 0, &msgid, &commit_done);
	}
	gettimeofday(&tval, NULL);
	et = (tval.tv_sec * 1000L) + (tval.tv_usec / 1000L);
	printf("microseconds needed for encode/decode with flatbuffers, for %d times: %ld\n", count, et - st);

	gettimeofday(&tval, NULL);
	st = (tval.tv_sec * 1000L) + (tval.tv_usec / 1000L);
	for (i = 0; i < count; i++)
	{
		PBSD_queuejob_dis(0, "1.server", "someserver", (struct attropl *)attrib, "EX", 0, &msgid, &commit_done);
	}
	gettimeofday(&tval, NULL);
	et = (tval.tv_sec * 1000L) + (tval.tv_usec / 1000L);
	printf("microseconds needed for encode/decode with dis, for %d times: %ld\n", count, et - st);
	return 0;
}
