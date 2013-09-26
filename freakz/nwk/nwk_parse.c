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
    \file nwk_parse.c
    \ingroup nwk
    \brief NWK frame parser

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the parsing of NWK header and command info
    in the receive buffer.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Parse the incoming NWK frame header in the receive buffer and
    stores the contents in the specified NWK header structure.
*/
/**************************************************************************/
void nwk_parse_hdr(buffer_t *buf, nwk_hdr_t *hdr)
{
    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: hdr->nwk_fcf = *(U16 *)buf->dptr;
    // this removes the lint warning for this block only

    hdr->nwk_fcf = *(U16 *)buf->dptr;
    buf->dptr += sizeof(U16);

    hdr->nwk_frm_ctrl.frame_type           = (hdr->nwk_fcf                              & 0x3);
    hdr->nwk_frm_ctrl.protocol_ver         = (hdr->nwk_fcf >> NWK_PROT_VER_OFF)         & 0xf;
    hdr->nwk_frm_ctrl.disc_route           = (hdr->nwk_fcf >> NWK_DISC_RTE_OFF)         & 0x3;
    hdr->nwk_frm_ctrl.mcast_flag           = (hdr->nwk_fcf >> NWK_MCAST_FLAG_OFF)       & 0x1;
    hdr->nwk_frm_ctrl.security             = (hdr->nwk_fcf >> NWK_SEC_OFF)              & 0x1;
    hdr->nwk_frm_ctrl.src_rte              = (hdr->nwk_fcf >> NWK_SRC_RTE_OFF)          & 0x1;
    hdr->nwk_frm_ctrl.dest_ieee_addr_flag  = (hdr->nwk_fcf >> NWK_DEST_IEEE_ADDR_OFF)   & 0x1;
    hdr->nwk_frm_ctrl.src_ieee_addr_flag   = (hdr->nwk_fcf >> NWK_SRC_IEEE_ADDR_OFF)    & 0x1;

    hdr->dest_addr = *(U16 *)buf->dptr;
    buf->dptr += sizeof(U16);

    hdr->src_addr = *(U16 *)buf->dptr;
    buf->dptr += sizeof(U16);

    hdr->radius = *buf->dptr++;
    hdr->seq_num = *buf->dptr++;

    if (hdr->nwk_frm_ctrl.dest_ieee_addr_flag)
    {
        hdr->dest_ieee_addr = *(U64 *)buf->dptr;
        buf->dptr += sizeof(U64);
    }

    if (hdr->nwk_frm_ctrl.src_ieee_addr_flag)
    {
        hdr->src_ieee_addr = *(U64 *)buf->dptr;
        buf->dptr += sizeof(U64);
    }

    if (hdr->nwk_frm_ctrl.mcast_flag)
    {
        hdr->mcast_ctrl = *buf->dptr++;
    }

    if (hdr->nwk_frm_ctrl.src_rte)
    {
        U8 relay_cnt;

        // The source route list is a variable list so we
        // need to calculate the size if its present. That
        // way, we know how much we need to increment the ptr
        // to get to the payload.
        hdr->src_rte_subfrm = buf->dptr;
        relay_cnt = *buf->dptr;
        buf->dptr += relay_cnt * sizeof(U16);
    }
}

/**************************************************************************/
/*!
    Parse the incoming command frame info into a command data structure. The
    command data structure is a union of all the different possible command
    frame types and the format is determined by the command frame ID.
*/
/**************************************************************************/
void nwk_parse_cmd(buffer_t *buf, nwk_cmd_t *cmd)
{
    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: hdr->nwk_fcf = *(U16 *)buf->dptr;
    // this removes the lint warning for this block only

    cmd->cmd_frm_id = *buf->dptr++;
    switch (cmd->cmd_frm_id)
    {
    case NWK_CMD_RTE_REQ:
        // command was a route request.
        cmd->rreq.cmd_opts  = *buf->dptr++;
        cmd->rreq.rreq_id   = *buf->dptr++;
        cmd->rreq.dest_addr = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
        cmd->rreq.path_cost = *buf->dptr++;
        break;

    case NWK_CMD_RTE_REP:
        // command was a route reply.
        cmd->rrep.cmd_opts      = *buf->dptr++;
        cmd->rrep.rreq_id       = *buf->dptr++;
        cmd->rrep.originator    = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
        cmd->rrep.responder     = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
        cmd->rrep.path_cost     = *buf->dptr++;
        break;

    case NWK_CMD_LEAVE:
        // command was a leave request
        cmd->leave.cmd_opts     = *buf->dptr++;
        break;

    default:
        break;
    }
}

/**************************************************************************/
/*!
    Parse the beacon frame's NWK layer info and fill out the scan descriptor
    fields. This data will later be used by the higher layers to determine
    which network to join and which device to choose for a parent.
*/
/**************************************************************************/
void nwk_parse_beacon(buffer_t *buf, mem_ptr_t *mem_ptr)
{
    nbor_tbl_entry_t nbor_entry;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: hdr->nwk_fcf = *(U16 *)buf->dptr;
    // this removes the lint warning for this block only

    SCAN_ENTRY(mem_ptr)->prot_id       = *buf->dptr++;
    SCAN_ENTRY(mem_ptr)->stack_profile = *buf->dptr    & NWK_STACK_PROFILE_MASK;
    SCAN_ENTRY(mem_ptr)->prot_ver      = (*buf->dptr++ & NWK_PROTOCOL_VERSION_MASK)    >> NWK_PROTOCOL_VERSION_OFF;
    SCAN_ENTRY(mem_ptr)->rtr_cap       = (*buf->dptr   & NWK_ROUTER_CAPACITY_MASK)     >> NWK_ROUTER_CAPACITY_OFF;
    SCAN_ENTRY(mem_ptr)->depth         = (*buf->dptr   & NWK_DEVICE_DEPTH_MASK)        >> NWK_DEVICE_DEPTH_OFF;
    SCAN_ENTRY(mem_ptr)->end_dev_cap   = (*buf->dptr++ & NWK_END_DEVICE_CAPACITY_MASK) >> NWK_END_DEVICE_CAPACITY_OFF;
    SCAN_ENTRY(mem_ptr)->ext_pan_id    = *(U64 *)buf->dptr;
    buf->dptr += sizeof(U64);

    // we need to decide if this is a potential parent for this device. We need to check to see if the
    // potential parent's stack is compatible with ours.
    if (SCAN_ENTRY(mem_ptr)->superfrm_spec & MAC_ASSOC_PERMIT_MASK)
    {
        if (nib->dev_type == NWK_ROUTER)
        {
            SCAN_ENTRY(mem_ptr)->pot_parent = (SCAN_ENTRY(mem_ptr)->rtr_cap) ? true : false;
        }
        else if (nib->dev_type == NWK_END_DEVICE)
        {
            SCAN_ENTRY(mem_ptr)->pot_parent = (SCAN_ENTRY(mem_ptr)->end_dev_cap) ? true : false;
        }
    }
    else
    {
        SCAN_ENTRY(mem_ptr)->pot_parent = false;
    }

    // add in the info to the neighbor table here
    memset(&nbor_entry, 0, sizeof(nbor_tbl_entry_t));
    nbor_entry.ext_pan_id       = SCAN_ENTRY(mem_ptr)->ext_pan_id;
    nbor_entry.pan_id           = SCAN_ENTRY(mem_ptr)->coord_pan_id;
    nbor_entry.device_type      = NWK_ROUTER;                       // only routers send beacons
    nbor_entry.rx_on_when_idle  = true;                             // true for all routers
    nbor_entry.relationship     = NWK_NONE;                         // no relationship for now
    nbor_entry.channel          = pib->curr_channel;
    nbor_entry.depth            = SCAN_ENTRY(mem_ptr)->depth;
    nbor_entry.permit_join      = (SCAN_ENTRY(mem_ptr)->superfrm_spec & (1<<MAC_ASSOC_PERMIT_OFF)) >> MAC_ASSOC_PERMIT_OFF;
    nbor_entry.pot_parent       = SCAN_ENTRY(mem_ptr)->pot_parent;

    if (SCAN_ENTRY(mem_ptr)->coord_addr.mode == SHORT_ADDR)
    {
        nbor_entry.nwk_addr = SCAN_ENTRY(mem_ptr)->coord_addr.short_addr;
        nbor_entry.ext_addr = INVALID_EXT_ADDR;
    }
    else if (SCAN_ENTRY(mem_ptr)->coord_addr.mode == LONG_ADDR)
    {
        nbor_entry.ext_addr = SCAN_ENTRY(mem_ptr)->coord_addr.long_addr;
        nbor_entry.nwk_addr = INVALID_NWK_ADDR;
    }
    nwk_neighbor_tbl_add(&nbor_entry);
}


