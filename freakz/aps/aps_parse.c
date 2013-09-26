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
    \file aps_parse.c
    \ingroup aps
    \brief APS frame parser

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles parsing of the APS header in the frame buffer.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Parse the APS header in the specified frame buffer and extract the data
    to the APS header. The APS header contains the following fields;
    - Frame control field
    - Destination endpoint
    - Group address
    - Profile ID
    - Source endpoint
    - APS counter
    - Extended header
*/
/**************************************************************************/
void aps_parse_hdr(buffer_t *buf, aps_hdr_t *hdr)
{
    bool    dest_ep_flag    = false,
            clust_id_flag   = false,
            src_ep_flag     = false,
            group_addr_flag = false,
            prof_id_flag    = false;

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: hdr->grp_addr = *(U16 *)buf->dptr;
    // this removes the lint warning for this block only

    hdr->aps_fcf = *buf->dptr++;

    // parse the frame control fields
    hdr->aps_frm_ctrl.frm_type          = (hdr->aps_fcf & 0x3);
    hdr->aps_frm_ctrl.delivery_mode     = (hdr->aps_fcf >> APS_DELIV_MODE_OFF)  & 0x3;
    hdr->aps_frm_ctrl.ack_format        = (hdr->aps_fcf >> APS_ACK_FMT_OFF)     & 0x1;
    hdr->aps_frm_ctrl.security          = (hdr->aps_fcf >> APS_SEC_OFF)         & 0x1;
    hdr->aps_frm_ctrl.ack_req           = (hdr->aps_fcf >> APS_ACK_REQ_OFF)     & 0x1;
    hdr->aps_frm_ctrl.ext_hdr           = (hdr->aps_fcf >> APS_EXT_HDR_OFF)     & 0x1;

    // configure flags for parsing the rest of the APS header
    if ((hdr->aps_frm_ctrl.frm_type == APS_DATA_FRM) ||
        (hdr->aps_frm_ctrl.frm_type == APS_ACK_FRM))
    {
        clust_id_flag = true;
        prof_id_flag = true;
    }

    switch (hdr->aps_frm_ctrl.delivery_mode)
    {
    case APS_UNICAST:
        // fall through
    case APS_BROADCAST:
        dest_ep_flag = true;
        src_ep_flag = true;
        break;
    case APS_INDIRECT:
        if (hdr->aps_frm_ctrl.ack_format)
        {
            src_ep_flag = true;
        }
        else
        {
            dest_ep_flag = true;
        }
        break;
    case APS_GROUP:
        group_addr_flag = true;
        src_ep_flag = true;
        break;

    default:
        // TODO: collect statistic here. should never reach.
        return;
    }

    // start parsing the APS header
    if (dest_ep_flag)
    {
        hdr->dest_ep = *buf->dptr++;
    }

    if (group_addr_flag)
    {
        hdr->grp_addr = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
    }

    if (clust_id_flag)
    {
        hdr->clust_id = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
    }

    if (prof_id_flag)
    {
        hdr->prof_id = *(U16 *)buf->dptr;
        buf->dptr += sizeof(U16);
    }

    if (src_ep_flag)
    {
        hdr->src_ep = *buf->dptr++;
    }

    hdr->aps_ctr = *buf->dptr++;
}
