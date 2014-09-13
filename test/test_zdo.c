/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.
    4. This software is subject to the additional restrictions placed on the
       Zigbee Specification's Terms of Use.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file test_zdo.c
    \ingroup test_sim
*/
/**************************************************************************/
#include "freakz.h"
#include "test_zdo.h"

/* callbacks for the ZDO */
static zdo_cb_t test_zdo_cb =
{
	test_zdo_nwk_form_conf,
	test_zdo_nwk_join_conf,
	test_zdo_nwk_join_ind,
	NULL,
	NULL
};

/*
 * Normally for an endpoint, we'd need to register it with the AF. But the
 * ZDO is already present in the stack and is registered with it. The ZDO
 * in the stack communicates with this application via the ZDo callbacks.
 * This apps main function is just to generate requests and process indications
 * and confirmations.
 */
void test_zdo_init()
{
	/* register the callbacks with the zdo layer */
	zdo_reg_cb(&test_zdo_cb);
}

void test_zdo_nwk_join_ind(U16 nwk_addr)
{
	DBG_PRINT_SIMONLY("TEST_ZDO_JOIN_IN: Need to hook this in.\n");
}

/*
 * This is the nwk formation confirm callback which is called from the ZDO
 * layer. This goes into the callback structure.
 */
void test_zdo_nwk_form_conf(U8 status)
{
	if (status == NWK_SUCCESS) {
#ifdef TEST_SIM
		sim_node_t *node = node_get();
		char msg[BUFSIZE];

		sprintf(msg, "node %d nwk form success\n", node->index);
		format_cmd_str((U8 *)msg);
		sim_pipe_cmd_out((U8 *)msg, strlen(msg) + 1);
#endif
		DBG_PRINT_SIMONLY("TEST_ZDO_NWK_FORM_CONF: Network Formation Successful.\n");
	} else {
		DBG_PRINT_SIMONLY("TEST_ZDO_NWK_FORM_CONF: Error occurred forming network.\n");
	}
}

/*
 * This is the callback function for the nwk join confirm that comes from
 * the ZDO layer.
 */
void test_zdo_nwk_join_conf(U8 status, U16 nwk_addr, U16 parent_addr)
{
	if (status == NWK_SUCCESS) {
#ifdef TEST_SIM
		sim_node_t *node = node_get();
		char msg[BUFSIZE];

		sprintf(msg, "node %d nwk join success\n", node->index);
		format_cmd_str((U8 *)msg);
		sim_pipe_cmd_out((U8 *)msg, strlen(msg) + 1);
#endif
		DBG_PRINT_SIMONLY("test_zdo_NWK_JOIN_CONF: Network joined successfully. \
				   Nwk Address: %4X.\n", nwk_addr);
	} else {
		DBG_PRINT_SIMONLY("test_zdo_NWK_JOIN_CONF: Error joining network.\n");
	}
}

void test_zdo_nwk_addr_req(U8 argc, char **argv)
{
	U64 ext_addr;
	zdo_req_t req;

	ext_addr = strtol(argv[1], NULL, 16);

	req.seq = zdo_seq_get();
	req.clust = NWK_ADDR_REQ_CLUST;
	req.type.nwk_addr.req_type      = 0;
	req.type.nwk_addr.start_index   = 0;
	req.type.nwk_addr.ext_addr      = ext_addr;
	zdo_gen_req(MAC_BROADCAST_ADDR, &req);
}

void test_zdo_ieee_addr_req(U8 argc, char **argv)
{
	U16 nwk_addr;
	zdo_req_t req;

	nwk_addr = strtol(argv[1], NULL, 16);

	req.seq = zdo_seq_get();
	req.clust                       = IEEE_ADDR_REQ_CLUST;
	req.type.ieee_addr.req_type     = 0;
	req.type.ieee_addr.start_index  = 0;
	req.type.ieee_addr.nwk_addr     = nwk_addr;
	zdo_gen_req(MAC_BROADCAST_ADDR, &req);
}

/*
 * fmt: cmd, num_in_clust, ...in_clust..., num_out_clust, ...out_clust...
 * ex: edb 3 1 2 3 3 4 5 6
 */
void test_zdo_end_dev_bind(U8 argc, char **argv)
{
	U8 i, tmp, num_in_clust, num_out_clust;
	U8 in_clust[ZDO_MAX_CLUST], out_clust[ZDO_MAX_CLUST];
	U16 *in_clust_ptr, *out_clust_ptr;
	zdo_req_t req;
	mac_pib_t *pib = mac_pib_get();

	num_in_clust = strtol(argv[1], NULL, 10);
	in_clust_ptr = (U16 *)in_clust;

	for (i=0; i<num_in_clust; i++)
	{
		*in_clust_ptr++ = strtol(argv[i+2], NULL, 16);
	}

	num_out_clust = strtol(argv[num_in_clust + 2], NULL, 10);
	out_clust_ptr = (U16 *)out_clust;
	tmp = i + 2 + 1; // i+2 = the last index we left off on for argv. the extra byte is for the clust cnt

	for (i=0; i<num_out_clust; i++)
	{
		*out_clust_ptr++ = strtol(argv[tmp + i], NULL, 16);
	}

	req.seq                         = zdo_seq_get();
	req.clust                       = END_DEV_BIND_REQ_CLUST;
	req.type.ed_bind.target         = 0;
	req.type.ed_bind.src_ext_addr   = pib->ext_addr;
	req.type.ed_bind.src_ep         = 1;
	req.type.ed_bind.prof_id        = ZIGBEE_PROF_ID;
	req.type.ed_bind.num_in_clust   = num_in_clust;
	req.type.ed_bind.in_clust       = in_clust;
	req.type.ed_bind.num_out_clust  = num_out_clust;
	req.type.ed_bind.out_clust      = out_clust;

	// send the end dev bind request to the coordinator
	zdo_gen_req(0, &req);
}

/*
 * fmt: cmd, dest_addr
 * ex: znl 0
 */
void test_zdo_nwk_lqi_req(U8 argc, char **argv)
{
	U16 addr;
	zdo_req_t req;

	addr = strtol(argv[1], NULL, 16);
	req.seq = zdo_seq_get();
	req.clust = NWK_LQI_REQ_CLUST;
	req.type.nwk_lqi.start_index = 0;
	zdo_gen_req(addr, &req);
}

void test_zdo_nwk_leave_req(U8 argc, char **argv)
{
	U16 addr, tmp;
	address_t nwk_addr;
	bool rem_children;
	mac_pib_t *pib = mac_pib_get();
	mem_ptr_t *nbor_mem_ptr;

	if (argc > 3)
	{
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
		return;
	}

	addr = strtol(argv[1], NULL, 16);
	tmp = (argc == 3) ? strtol(argv[2], NULL, 10) : 0;

	// rem children needs to be a bool
	rem_children = tmp ? true : false;

	if (addr == pib->short_addr) {
		nwk_leave_req(0, rem_children, false);
	} else {
		nwk_addr.mode = SHORT_ADDR;
		nwk_addr.short_addr = addr;
		if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&nwk_addr)) != NULL)
		{
			nwk_leave_req(NBOR_ENTRY(nbor_mem_ptr)->ext_addr, rem_children, false);
		}
	}
}

/* format: ndr <target addr> */
void test_zdo_nwk_disc_req(U8 argc, char **argv)
{
	U16 addr;
	zdo_req_t req;

	if (argc != 2)
	{
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
		return;
	}

	addr = strtol(argv[1], NULL, 16);

	req.seq = zdo_seq_get();
	req.clust = NWK_DISC_REQ_CLUST;
	req.type.nwk_disc.scan_channels = aPhyChannelsSupported;
	req.type.nwk_disc.scan_duration = DEFAULT_SCAN_DURATION;
	req.type.nwk_disc.start_index = 0;
	zdo_gen_req(addr, &req);
}

/*
 * format: nur <addr> <operation>
 *
 * 0: change channel <channel>
 * 1: change channel mask and nwk manager
 * 3: energy scan
 */
void test_zdo_nwk_update_req(U8 argc, char **argv)
{
	U32 mask;
	U16 addr, nwk_mgr;
	U8 op, channel, duration;
	zdo_req_t req;

	if (argc < 3) {
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
		return;
	}

	addr = strtol(argv[1], NULL, 16);
	op = strtol(argv[2], NULL, 10);

	switch (op)
	{
	case 0:
		if (argc != 4) {
			DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
			return;
		}

		channel = strtol(argv[3], NULL, 10);
		duration = 0xFE;

		if ((channel > MAC_PHY_CHANNEL_OFFSET) &&
		    (channel < MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS))
		{
			mask = 1 << channel;
		} else {
			DBG_PRINT_SIMONLY("ERROR: Channel is invalid!\n");
			return;
		}
		nwk_mgr = 0;
		break;
	case 1:
		if (argc != 4) {
			DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
		}
		nwk_mgr = strtol(argv[3], NULL, 10);
		duration = 0xFF;
		mask = (unsigned)0x07FFF800;
		break;
	case 2:
		if (argc != 3) {
			DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
		}

		duration = 5;
		mask = (unsigned)0x07FFF800;
		nwk_mgr = 0;
		break;
	default:
		mask = 0;
		duration = 0;
		nwk_mgr = 0;
		break;
	}

	req.seq                             = zdo_seq_get();
	req.clust                           = NWK_UPDATE_REQ_CLUST;
	req.type.nwk_update.scan_channels   = mask;
	req.type.nwk_update.scan_duration   = duration;
	req.type.nwk_update.scan_cnt        = 1;
	req.type.nwk_update.nwk_update_id   = 0;
	req.type.nwk_update.nwk_mngr_addr   = nwk_mgr;
	zdo_gen_req(addr, &req);
}

void test_zdo_server_nwk_leave_req(U8 argc, char **argv)
{
	U16 dest_addr;
	U64 lv_addr;
	bool rem_children;
	zdo_req_t req;

	if (argc != 4) {
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
	}

	dest_addr = strtol(argv[1], NULL, 16);
	lv_addr = (U64)strtol(argv[2], NULL, 16);
	rem_children = (bool)strtol(argv[3], NULL, 10);

	req.seq                     = zdo_seq_get();
	req.clust                   = NWK_LEAVE_REQ_CLUST;
	req.type.leave.addr         = lv_addr;
	req.type.leave.rem_children = rem_children;
	req.type.leave.rejoin       = false;
	zdo_gen_req(dest_addr, &req);
}

void test_zdo_permit_join_req(U8 argc, char **argv)
{
	U16 dest_addr;
	U8 duration;
	zdo_req_t req;

	if (argc != 3) {
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
	}

	dest_addr = strtol(argv[1], NULL, 10);
	duration = strtol(argv[2], NULL, 10);

	req.seq                         = zdo_seq_get();
	req.clust                       = NWK_PERMIT_JOIN_REQ_CLUST;
	req.type.permit_join.duration   = duration;
	req.type.permit_join.tc_sig     = false;
	zdo_gen_req(dest_addr, &req);
}

void test_zdo_bind_req(U8 argc, char **argv)
{
	U64 src_addr;
	U8 src_ep, dest_ep;
	U16 dest_addr, clust_id;
	zdo_req_t req;

	if (argc != 7) {
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
	}

	src_addr_short  = strtol(argv[1], NULL, 16);
	src_addr        = strtol(argv[2], NULL, 16);
	src_ep          = strtol(argv[3], NULL, 16);
	clust_id        = strtol(argv[4], NULL, 16);
	dest_addr       = strtol(argv[5], NULL, 16);
	dest_ep         = strtol(argv[6], NULL, 16);

	req.clust                           = BIND_REQ_CLUST;
	req.seq                             = zdo_seq_get();
	req.type.bind.src_addr              = src_addr;
	req.type.bind.src_ep                = src_ep;
	req.type.bind.clust                 = clust_id;
	req.type.bind.dest_addr.mode        = SHORT_ADDR;
	req.type.bind.dest_addr.short_addr  = dest_addr;
	req.type.bind.dest_ep               = dest_ep;
	zdo_gen_req(dest_addr, &req);
}

/* Send an unbind request to the destination */
void test_zdo_unbind_req(U8 argc, char **argv)
{
	U64 src_addr;
	U8 src_ep, dest_ep;
	U16 src_addr_short, dest_addr, clust_id;
	zdo_req_t req;

	if (argc != 7) {
		DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
	}

	src_addr_short  = strtol(argv[1], NULL, 16);
	src_addr        = strtol(argv[2], NULL, 16);
	src_ep          = strtol(argv[3], NULL, 16);
	clust_id        = strtol(argv[4], NULL, 16);
	dest_addr       = strtol(argv[5], NULL, 16);
	dest_ep         = strtol(argv[6], NULL, 16);

	req.clust                           = UNBIND_REQ_CLUST;
	req.seq                             = zdo_seq_get();
	req.type.bind.src_addr              = src_addr;
	req.type.bind.src_ep                = src_ep;
	req.type.bind.clust                 = clust_id;
	req.type.bind.dest_addr.mode        = SHORT_ADDR;
	req.type.bind.dest_addr.short_addr  = dest_addr;
	req.type.bind.dest_ep               = dest_ep;
	zdo_gen_req(dest_addr, &req);
}
