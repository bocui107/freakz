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
    \file nwk_gen.c
    \ingroup nwk
    \brief NWK frame generator

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the generation of NWK header and command formatting
    in the transmit buffer.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Generate the frame control field for the NWK header. The frame control
    field is at the beginning of the NWK header and gives information about
    what fields the NWK header contains. The frame control fields are:
    - Frame type
    - Protocol version
    - Discover route
    - Multicast
    - Security
    - Source route
    - Destination IEEE address present
    - Source IEEE address present
*/
/**************************************************************************/
U16 nwk_gen_frm_ctrl(const nwk_hdr_t *hdr)
{
    U16 tmp = 0;
    tmp |= (U16)((hdr->nwk_frm_ctrl.src_ieee_addr_flag   & 0x1) << NWK_SRC_IEEE_ADDR_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.dest_ieee_addr_flag  & 0x1) << NWK_DEST_IEEE_ADDR_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.src_rte              & 0x1) << NWK_SRC_RTE_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.security             & 0x1) << NWK_SEC_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.mcast_flag           & 0x1) << NWK_MCAST_FLAG_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.disc_route           & 0x3) << NWK_DISC_RTE_OFF);
    tmp |= (U16)((hdr->nwk_frm_ctrl.protocol_ver         & 0xf) << NWK_PROT_VER_OFF);
    tmp |= (U16)(hdr->nwk_frm_ctrl.frame_type           & 0x3);
    return tmp;
}

/**************************************************************************/
/*!
    Generate the NWK header for the frame. This is the main function to
    generate the NWK header. The NWK header consists of the following fields:
    - Frame Control Field
    - Destination Address
    - Source Address
    - Radius
    - Sequence number
    - Destination IEEE address  (optional)
    - Source IEEE address       (optional)
    - Multicast
*/
/**************************************************************************/
void nwk_gen_header(buffer_t *buf, nwk_hdr_t *hdr)
{
    U8 nhdr_size = 0;
    hdr->nwk_fcf = nwk_gen_frm_ctrl(hdr);

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr = hdr->nwk_fcf
    // this removes the lint warning for this block only

    // Calculate the size of the nwk header
    nhdr_size = 8;
    nhdr_size += (hdr->nwk_frm_ctrl.dest_ieee_addr_flag) ? 8 : 0;
    nhdr_size += (hdr->nwk_frm_ctrl.src_ieee_addr_flag) ? 8 : 0;
    nhdr_size += (hdr->nwk_frm_ctrl.mcast_flag) ? 1 : 0;

    // fill in the length and adjust the data pointer
    buf->dptr -= nhdr_size;
    buf->len += nhdr_size;

    // fill in the fields of the nwk header
    *(U16 *)buf->dptr = hdr->nwk_fcf;
    buf->dptr += sizeof(U16);

    *(U16 *)buf->dptr = hdr->dest_addr;
    buf->dptr += sizeof(U16);

    *(U16 *)buf->dptr = hdr->src_addr;
    buf->dptr += sizeof(U16);

    *buf->dptr++ = hdr->radius;
    *buf->dptr++ = hdr->seq_num;

    if (hdr->nwk_frm_ctrl.dest_ieee_addr_flag)
    {
        *(U64 *)buf->dptr = hdr->dest_ieee_addr;
        buf->dptr += sizeof(U64);
    }

    if (hdr->nwk_frm_ctrl.src_ieee_addr_flag)
    {
        *(U64 *)buf->dptr = hdr->src_ieee_addr;
        buf->dptr += sizeof(U64);
    }

    // roll back the dptr to the beginning of the header
    buf->dptr -= nhdr_size;
}

/**************************************************************************/
/*!
    Generate NWK command frame formatting. After the NWK header, a command
    frame contains a command ID and other formatted info depending on what type of
    command frame it is.
*/
/**************************************************************************/
void nwk_gen_cmd(buffer_t *buf, const nwk_cmd_t *cmd)
{
    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr = hdr->nwk_fcf
    // this removes the lint warning for this block only

    // command payload size table
    const U8 cmd_size[] =
    {
        0,
        NWK_CMD_SIZE_RTE_REQ,
        NWK_CMD_SIZE_RTE_REP,
        NWK_CMD_SIZE_STATUS,
        NWK_CMD_SIZE_LEAVE,
        NWK_CMD_SIZE_RTE_REC,
        NWK_CMD_SIZE_REJOIN_REQ,
        NWK_CMD_SIZE_REJOIN_RESP,
        NWK_CMD_SIZE_LINK_STATUS,
        NWK_CMD_SIZE_NWK_REPORT,
        NWK_CMD_SIZE_NWK_UPDATE
    };

    // Move the data pointer back and increment the length count of the buffer by "size" bytes
    buf->dptr -= cmd_size[cmd->cmd_frm_id];
    buf->len += cmd_size[cmd->cmd_frm_id];

    // Add the command ID
    *buf->dptr++ = cmd->cmd_frm_id;

    // fill out the rest of the fields
    switch (cmd->cmd_frm_id)
    {
    case NWK_CMD_RTE_REQ:
        *buf->dptr++        = cmd->rreq.cmd_opts;
        *buf->dptr++        = cmd->rreq.rreq_id;
        *(U16 *)buf->dptr   = cmd->rreq.dest_addr;
        buf->dptr += sizeof(U16);
        *buf->dptr++        = cmd->rreq.path_cost;
        break;

    case NWK_CMD_RTE_REP:
        *buf->dptr++        = cmd->rrep.cmd_opts;
        *buf->dptr++        = cmd->rrep.rreq_id;
        *(U16 *)buf->dptr   = cmd->rrep.originator;
        buf->dptr += sizeof(U16);
        *(U16 *)buf->dptr   = cmd->rrep.responder;
        buf->dptr += sizeof(U16);
        *buf->dptr++        = cmd->rrep.path_cost;
        break;

    case NWK_CMD_LEAVE:
        *buf->dptr++        = cmd->leave.cmd_opts;
        break;
    default:
        break;
    }

    // roll back the dptr to the beginning of the command payload
    buf->dptr -= cmd_size[cmd->cmd_frm_id];
}

/**************************************************************************/
/*!
    Generate a network layer beacon payload. A beacon frame contains information
    from both the NWK and MAC layer. This function generates the NWK layer specific
    information for the beacon. This function gets called by the MAC layer when
    its generating a beacon frame in response to a beacon request.
    The fields in the NWK portion of the beaon are as follows:
    - Protocol ID
    - Stack profile
    - Protocol version
    - Router capacity available
    - Device depth
    - End device capacity
    - Extended PAN ID
*/
/**************************************************************************/
void nwk_gen_beacon(buffer_t *buf)
{
    nwk_nib_t *nib = nwk_nib_get();
    bool ed_cap, rtr_cap;


    ed_cap  = nib->ed_cap > 0;
    rtr_cap = nib->rtr_cap > 0;

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr = hdr->nwk_fcf
    // this removes the lint warning for this block only

    buf->len += NWK_BEACON_PAYLOAD_SIZE;
    buf->dptr -= NWK_BEACON_PAYLOAD_SIZE;
    *buf->dptr++ = ZIGBEE_PROTOCOL_ID;
    *buf->dptr++ = (ZIGBEE_PROTOCOL_VERSION << 4) | (nib->stack_profile & 0x0F);
    *buf->dptr++ = ((ed_cap << 7) | ((nib->depth & 0xF) << 3) | ((rtr_cap << 2))) & 0xFC;
    *(U64 *)buf->dptr   = nib->ext_pan_ID;
    buf->dptr += sizeof(U64);
    buf->dptr -= NWK_BEACON_PAYLOAD_SIZE;
}

/**************************************************************************/
/*!
    Generate the capability info for the device. This will be used in the association
    request command frames that we send when we want to join a network.
    The fields in the capability info are as follows:
    - Alternate PAN coordinator (always 0 in the 2007 spec)
    - Device type
    - Power source
    - Rx on when idle
    - Security
    - Allocate address
*/
/**************************************************************************/
U8 nwk_gen_capab_info(const nwk_capab_info_t *info)
{
    U8 capab_info = 0x0;

    capab_info |= (info->dev_type)      ? NWK_CAPAB_DEV_TYPE_MASK           : 0;
    capab_info |= (info->pwr_src)       ? NWK_CAPAB_POWER_SRC_MASK          : 0;
    capab_info |= (info->rx_on_idle)    ? NWK_CAPAB_RX_ON_WHEN_IDLE_MASK    : 0;
    capab_info |= (info->security)      ? NWK_CAPAB_SECURITY_MASK           : 0;
    capab_info |= (info->alloc_addr)    ? NWK_CAPAB_ALLOC_ADDR_MASK         : 0;
    return capab_info;
}

