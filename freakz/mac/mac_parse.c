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
    \file mac_parse.c
    \ingroup mac
    \brief MAC frame parser

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the parsing of 802.15.4 frames. It takes the specified
    incoming buffer and steps through each field, filling out the given
    data structure. There are three basic parsing tasks for MAC frames:
    header parsing, command parsing, and beacon parsing.
*/
#include "freakz.h"

/*
 * Parse the specified incoming buffer and extract the MAC header fields
 * from it. Then add those fields to the given header data structure.
 */
void mac_parse_hdr(buffer_t *buf, mac_hdr_t *hdr)
{
	hdr->mac_fcf = *(U16 *)buf->dptr;
	buf->dptr += sizeof(U16);

	hdr->mac_frm_ctrl.frame_type    = (hdr->mac_fcf                            & 0x3);
	hdr->mac_frm_ctrl.frame_pending = (hdr->mac_fcf >> MAC_FRM_PEND_OFF)       & 0x1;
	hdr->mac_frm_ctrl.ack_req       = (hdr->mac_fcf >> MAC_ACK_REQ_OFF)        & 0x1;
	hdr->mac_frm_ctrl.pan_id_compr  = (hdr->mac_fcf >> MAC_PAN_ID_COMPR_OFF)   & 0x1;
	hdr->dest_addr.mode             = (hdr->mac_fcf >> MAC_DEST_ADDR_MODE_OFF) & 0x3;
	hdr->src_addr.mode              = (hdr->mac_fcf >> MAC_SRC_ADDR_MODE_OFF)  & 0x3;

	hdr->dsn = *buf->dptr++;

	if (hdr->dest_addr.mode > 0) {
		hdr->dest_pan_id = *(U16 *)(buf->dptr);
		buf->dptr += sizeof(U16);

		if (hdr->dest_addr.mode == SHORT_ADDR)
		{
			hdr->dest_addr.short_addr = *(U16 *)(buf->dptr);
			buf->dptr += sizeof(U16);
		} else {
			hdr->dest_addr.long_addr = *(U64 *)(buf->dptr);
			buf->dptr += sizeof(U64);
		}
	}

	if (hdr->src_addr.mode > 0)
	{
		if (!(hdr->mac_frm_ctrl.pan_id_compr))
		{
			hdr->src_pan_id = *(U16 *)(buf->dptr);
			buf->dptr += sizeof(U16);
		}

		if (hdr->src_addr.mode == SHORT_ADDR) {
			hdr->src_addr.short_addr = *(U16 *)(buf->dptr);
			buf->dptr += sizeof(U16);
		} else {
			hdr->src_addr.long_addr = *(U64 *)(buf->dptr);
			buf->dptr += sizeof(U64);
		}
	}
}

/*
 * Parse the incoming buffer and extract the command info. Then fill in the
 * mac command data struct. We only parse command frames that have additional
 * data other than the frame id. If the command frame only consists of the
 * frame ID, then we won't send it to this function since we can use it
 * directly.
 */
void mac_parse_cmd(buffer_t *buf, mac_cmd_t *cmd)
{
	cmd->cmd_id = *buf->dptr++;
	switch (cmd->cmd_id)
	{
	case MAC_ASSOC_REQ:
		cmd->assoc_req.cap_info = *buf->dptr++;
		break;
	case MAC_ASSOC_RESP:
		cmd->assoc_resp.short_addr = *(U16 *)buf->dptr;
		buf->dptr += sizeof(U16);
		cmd->assoc_resp.assoc_status = *buf->dptr++;
		break;
	case MAC_COORD_REALIGN:
		cmd->coord_realign.pan_id = *(U16 *)buf->dptr;
		buf->dptr += sizeof(U16);
		cmd->coord_realign.coord_short_addr = *(U16 *)buf->dptr;
		buf->dptr += sizeof(U16);
		cmd->coord_realign.channel = *buf->dptr++;
		cmd->coord_realign.short_addr = *(U16 *)buf->dptr;
		buf->dptr += sizeof(U16);
		break;
	default:
		break;
	}
}

/*
 * Parse the specified incoming buffer and extract the beacon information.
 * Since we are not using beacons except for association, the beacon order
 * superframe order, final cap slot, and batt_life_ext fields are currently unused.
 *
 * After we extract the beacon data, a scan descriptor entry is allocated
 * and filled out according to the data in the beacon. This will be used
 * later by the upper layers to decide on a network and parent to join.
 *
 * The scan descriptor will be returned by the function. If there is no available
 * memory to alloc a scan descriptor, then this function will return NULL.
 */
void mac_parse_beacon(buffer_t *buf, mac_hdr_t *hdr)
{
	U8 gts, pend_addr_spec, pend_short, pend_long;
	U16 superframe_spec;
	mac_pib_t *pib = mac_pib_get();
	mac_pcb_t *pcb = mac_pcb_get();

	/* if its a unique nwk, then increment the nwk cnt */
	if (mac_scan_descr_find_pan(hdr->src_pan_id) == NULL)
		pcb->nwk_cnt++;

	if (mac_scan_descr_find_addr(&hdr->src_addr) != NULL)
		/* we already have this address in our list. don't add */
		return;

	/*
	 * move the data pointer forward two bytes now that we've
	 * processed the superframe spec field
	 */
	superframe_spec = *(U16 *)buf->dptr;
	buf->dptr += sizeof(U16);
	mac_scan_descr_add(&hdr->src_addr, hdr->src_pan_id, pib->curr_channel, superframe_spec);

	/*
	 * check to see if there are any GTS descriptors.
	 * this should be zero, but in case its not, we
	 * need to skip over the fields.
	 */
	if ((gts = (*buf->dptr++ & MAC_GTS_MASK)) != 0)
		buf->dptr += (MAC_GTS_DESC_SIZE * gts) + MAC_GTS_DIR_SIZE;

	/*
	 * check to see if there are any pending addresses.
	 * these should be zero, but in case they are not,
	 * we need to skip over them.
	 */
	pend_addr_spec = *buf->dptr++;
	pend_short = (pend_addr_spec & MAC_PENDING_SHORT_ADDR_MASK);
	if (pend_short)
		/*
		 * we need to move the dptr ahead and skip over
		 * the short addresses in the list. each short
		 * addr is 16 bits.
		 */
		buf->dptr += pend_short * sizeof(U16);

	pend_long = (pend_addr_spec & MAC_PENDING_LONG_ADDR_MASK);
	pend_long = pend_long >> MAC_PENDING_LONG_ADDR_OFF;
	if (pend_long)
		/*
		 * we need to move the dptr ahead and skip over
		 * the long addresses in the list. each long addr
		 * is 64 bits.
		 */
		buf->dptr += pend_long * sizeof(U64);
}
