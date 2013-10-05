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
    \file test_data.c
    \ingroup test_sim

    This file contains the application endpoint for data only transfers. It does
    not use the ZCL and is only used to transfer data directly between Zigbee
    devices.
*/
/**************************************************************************/
#include "freakz.h"
#include "test_data.h"

/* Simple descriptor for this ep */
static U8 test_data_simple_desc[] = {
	TEST_DATA_EP,	/* ep */
	0x00,	/* profile id */
	0xC0,
	0xAA,	/* dev id */
	0x33,	/* dev ver */
	1,	/* num in clusters */
	TEST_DATA_IN_CLUST,	/* in clusters */
	1,			/* num out clusters */
	TEST_DATA_OUT_CLUST	/* out clusters */
};

void test_data_init()
{
	af_ep_add(TEST_DATA_EP,
		  test_data_simple_desc,
		  sizeof(test_data_simple_desc),
		  false,
		  test_data_rx_handler,
		  test_data_conf_handler);
}

void test_data_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id)
{
	U8 i;

	DBG_PRINT("TEST DATA RECEIVED FROM NODE %04X: ", src_addr);
	for (i=0; i<len; i++)
		DBG_PRINT_RAW("%02X ", data[i]);
	DBG_PRINT_RAW("\n");
}

void test_data_conf_handler(U8 status, U8 handle)
{
	DBG_PRINT_SIMONLY("TEST DATA CONF RCVD: Status: %s, Handle: %x.\n",
					debug_dump_af_status(status), handle);
}

/* Extract the dest addr, data, and len from the argv string */
U8 test_data_extract_data(U8 *data, U16 *dest_addr, char **argv)
{
	U8 i;
	U16 dest;

	/* argv[1] = dest_addr, rest is data */
	if (dest_addr)
		dest =  (U16)strtol(argv[1], NULL, 16);

	DBG_PRINT("Data for transmission ");
	for (i = 1; argv[i] != NULL; i++)
	{
		data[i - 2] = (U8)strtol(argv[i], NULL, 16);

		if (i > 1)
			DBG_PRINT_RAW("%02X ", data[i-2]);
	}
	DBG_PRINT_RAW("\n");
	*dest_addr = dest;
	return i-2;
}

void test_data_get_buf_cnt(U8 argc, char **argv)
{
	DBG_PRINT_SIMONLY("CURRENT BUFFERS IN USE: %d.\n", buf_get_cnt());
}

/* Send data via the app sublayer data request service */
void test_data_unicast_data_req(U8 argc, char **argv)
{
	U8 len, data_handle, data[128];
	U16 dest_addr;

	len = test_data_extract_data(data, &dest_addr, argv);
	data_handle = af_handle_get();
	af_tx(data,
	      len,
	      TEST_DATA_EP,
	      dest_addr,
	      TEST_DATA_EP,
	      TEST_DATA_IN_CLUST,
	      TEST_DATA_PROF_ID,
	      APS_DEST_ADDR_16_EP_PRESENT,
	      0,
	      ZIGBEE_DEFAULT_RADIUS,
	      data_handle);
}

/* Send data via the app sublayer data request service using aps level ack */
void test_data_unicast_rel_data_req(U8 argc, char **argv)
{
	U8 len, data_handle, data[128];
	U16 dest_addr;

	len = test_data_extract_data(data, &dest_addr, argv);
	data_handle = af_handle_get();
	af_tx(data,
	      len,
	      TEST_DATA_EP,
	      dest_addr,
	      TEST_DATA_EP,
	      TEST_DATA_IN_CLUST,
	      TEST_DATA_PROF_ID,
	      APS_DEST_ADDR_16_EP_PRESENT,
	      1 << APS_TX_REQ_ACK_TX_OFF,
	      ZIGBEE_DEFAULT_RADIUS,
	      data_handle);
}

void test_data_ind_data_req(U8 argc, char **argv)
{
	U8 i, len, data_handle, data[128];

	DBG_PRINT("Data for transmission ");
	for (i = 0; argv[i+1] != NULL; i++)
	{
		data[i] = (U8)strtol(argv[i+1], NULL, 16);
		DBG_PRINT_RAW("%02X ", data[i]);
	}
	DBG_PRINT_RAW("\n");

	len = i;
	data_handle = af_handle_get();
	af_tx(data,
	      len,
	      TEST_DATA_EP,
	      0,
	      0,
	      TEST_DATA_IN_CLUST,
	      TEST_DATA_PROF_ID,
	      APS_DEST_ADDR_EP_NONE,
	      0,
	      ZIGBEE_DEFAULT_RADIUS,
	      data_handle);
}

void test_data_grp_data_req(U8 argc, char **argv)
{
	U8 len, data_handle, data[128];
	U16 grp_id;

	len = test_data_extract_data(data, &grp_id, argv);
	data_handle = af_handle_get();
	af_tx(data,
	      len,
	      TEST_DATA_EP,
	      grp_id,
	      0,
	      TEST_DATA_IN_CLUST,
	      TEST_DATA_PROF_ID,
	      APS_GROUP_ADDR_PRESENT,
	      0,
	      ZIGBEE_DEFAULT_RADIUS,
	      data_handle);
}
