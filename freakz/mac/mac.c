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
    \defgroup mac MAC - Media Access Control Layer
    \file mac.c
    \ingroup mac
    \brief MAC toplevel file

    This file implements the main data handling functions for the mac layer.
    In a way, its the gateway into and out of the stack. The rx data comes in,
    an event gets posted to the mac process, and the mac will send it to the
    rx handler. On the tx side, a mac data request comes in, the function
    takes the data and adds the mac header, then sends it to either the driver
    for immediate transmission or the indirect queue to await later transmission.
*/
#include "freakz.h"

/* MAC protocol control block instantiation */
static mac_pcb_t pcb;

/* MAC information base instantiation */
static mac_pib_t pib;

/*
 * Initialize the various elements in the MAC layer. Reset the protocol
 * control block, start the mac process, init the events, and set the mac
 * info base to its default values.
 */
void mac_init()
{
	mac_queue_init();
	mac_indir_init();
	mac_scan_init();

	/*
	 * Set up the processes. First start the mac process,
	 * then allocate the rx event
	 */
	process_start(&mac_process, NULL);
	event_mac_rx = process_alloc_event();

	/* init the pcb */
	memset(&pcb, 0, sizeof(mac_pcb_t));

	/* init the mac pib */
	memset(&pib, 0, sizeof(mac_pib_t));
	pib.ack_wait_duration		= aMacAckWaitDuration;
	pib.resp_wait_time		= aMacResponseWaitTime;
	pib.coord_addr.mode		= SHORT_ADDR;
	pib.coord_addr.short_addr	= 0xFFFF;
	pib.short_addr			= 0xFFFF;
	pib.pan_id			= 0xFFFF;
	pib.rx_on_when_idle		= true;
	pib.assoc_permit		= true;
	pib.max_csma_backoffs		= 3;
	pib.min_be			= 3;
	pib.dsn				= (U8)drvr_get_rand();

#if (TEST_SIM == 1)
	pib.ext_addr = getpid();
#else
	pib.ext_addr = drvr_get_rand();
#endif

	/* set these in the hardware */
	drvr_set_pan_id(pib.pan_id);
	drvr_set_short_addr(pib.short_addr);
	drvr_set_ext_addr(pib.ext_addr);
}

/* Return a pointer to the MAC protocol control block structure */
mac_pcb_t *mac_pcb_get()
{
	return &pcb;
}

/* Return a pointer to the MAC information base structure */
mac_pib_t *mac_pib_get()
{
	return &pib;
}

/*
 * Handle outgoing data frames coming from the next higher layer. The data
 * in the argument will be used to generate the mac header and will be added
 * to the frame in the specified transmit buffer.
 */
void mac_data_req(mac_data_req_t *req)
{
	mac_hdr_t hdr;
	bool indirect;

	memset(&hdr, 0, sizeof(mac_hdr_t));
	hdr.mac_frm_ctrl.frame_type     = MAC_DATA;
	hdr.mac_frm_ctrl.ack_req        = (bool)(req->tx_options & MAC_ACK_REQUEST);
	hdr.mac_frm_ctrl.frame_ver      = MAC_802_15_4_2006;
	hdr.dsn                         = pib.dsn++;

	hdr.dest_pan_id                 = req->dest_pan_id;
	hdr.src_pan_id                  = req->src_pan_id;
	memcpy(&hdr.src_addr, &req->src_addr, sizeof(address_t));
	memcpy(&hdr.dest_addr, &req->dest_addr, sizeof(address_t));

	/* generate the header */
	mac_gen_header(req->buf, &hdr);
	DBG_PRINT_RAW("\n<OUTGOING>\n");
	debug_dump_mac_hdr(&hdr);

	/* send the frame to the tx handler for processing */
	indirect = (req->tx_options & MAC_INDIRECT_TRANS);
	mac_tx_handler(req->buf,
			&hdr.dest_addr,
			indirect,
			hdr.mac_frm_ctrl.ack_req,
			hdr.dsn,
			req->msdu_handle);
}

/*
 * Process the received command frame and route it to the correct
 * function.
 */
void mac_cmd_handler(mac_cmd_t *cmd, mac_hdr_t *hdr)
{
	buffer_t *buf_out;
	mac_hdr_t hdr_out;
	nwk_nib_t *nib = nwk_nib_get();

	DBG_PRINT_SIMONLY("\n<INCOMING>");
	debug_dump_mac_cmd(cmd);

	switch (cmd->cmd_id)
	{
	case MAC_ASSOC_REQ:
		if (nib->joined)
		{
			DBG_PRINT_SIMONLY("MAC: MAC Association Request Command Received.\n");
			/*
			 * check to make sure that association is permitted and the
			 * src address is the correct mode
			 */
			if ((pib.assoc_permit) && (hdr->src_addr.mode == LONG_ADDR)) {
				mac_assoc_ind_t assoc_args;
				assoc_args.capability = cmd->assoc_req.cap_info;
				memcpy(&assoc_args.dev_addr, &hdr->src_addr, sizeof(address_t));
				mac_assoc_ind(&assoc_args);
			} else {
				mac_assoc_resp_t resp_args;
				memcpy(&resp_args.dev_addr, &hdr->src_addr, sizeof(address_t));
				resp_args.assoc_short_addr = 0xFFFF;
				resp_args.status = MAC_INVALID_PARAMETER;
				mac_assoc_resp(&resp_args);
			}
		}
		break;
	case MAC_ASSOC_RESP:
		if ((pcb.mac_state == MLME_DATA_REQ) || (pcb.mac_state == MLME_ASSOC_REQ))
			pcb.mac_state = MLME_IDLE;

		mac_assoc_conf(cmd->assoc_resp.short_addr, cmd->assoc_resp.assoc_status);
		break;
	case MAC_DATA_REQ:
		if (nib->joined) {
			DBG_PRINT("MAC: MAC Data Request Command Received.\n");
			mac_indir_data_req_handler(&hdr->src_addr);
		}
		break;
	case MAC_BEACON_REQ:
		if (nib->joined) {
			DBG_PRINT("MAC: MAC Beacon Request Command Received.\n");
			BUF_ALLOC(buf_out, TX);
			mac_gen_beacon_frm(buf_out, &hdr_out);
			mac_tx_handler(buf_out, &hdr_out.dest_addr, false, false, hdr_out.dsn, 0);
		}
		break;
	case MAC_ORPHAN_NOT:
		if (hdr->src_addr.mode == LONG_ADDR)
			mac_orphan_ind(hdr->src_addr.long_addr);
		break;
	case MAC_COORD_REALIGN:
		/*
		 * check over the coord realign frame to make sure that
		 * the data is what we had previously
		 */
		if ((cmd->coord_realign.pan_id == pib.pan_id) &&
		    (cmd->coord_realign.short_addr == pib.short_addr))
		{
			if ((pib.coord_addr.mode == SHORT_ADDR) &&
			    (cmd->coord_realign.coord_short_addr == pib.coord_addr.short_addr))
			{
				pcb.coor_realign_rcvd = true;
			} else if ((pib.coord_addr.mode == LONG_ADDR) &&
				   (hdr->src_addr.mode == LONG_ADDR) &&
				   (hdr->src_addr.long_addr == pib.coord_addr.long_addr))
			{
				pcb.coor_realign_rcvd = true;
			} else {
				return;
			}
		} else {
			return;
		}

		/*
		 * stop the scan once we get a valid coord realignment
		 * command frame the scan will end once the channel is
		 * beyond the channel range
		 */
		pcb.curr_scan_channel = MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS;
		ctimer_stop(&pcb.mlme_tmr);
		mac_scan(NULL);
		break;
	default:
		break;
	}
}
