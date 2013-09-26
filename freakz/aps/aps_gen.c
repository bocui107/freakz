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
    \file aps_gen.c
    \ingroup aps
    \brief APS frame generator

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles generation of the APS header and frame control field.
*/
/**************************************************************************/
#include <assert.h>
#include "freakz.h"

/**************************************************************************/
/*!
    Generate the APS frame control field. The field members are:
    - Extended header
    - ACK request
    - Security
    - ACK Format
    - Delivery mode
    - Frame type
*/
/**************************************************************************/
U8 aps_gen_frm_ctrl(const aps_hdr_t *hdr)
{
    U8 tmp = 0;

    tmp |= (hdr->aps_frm_ctrl.ext_hdr & 0x1)            << APS_EXT_HDR_OFF;
    tmp |= (hdr->aps_frm_ctrl.ack_req & 0x1)            << APS_ACK_REQ_OFF;
    tmp |= (hdr->aps_frm_ctrl.security & 0x1)           << APS_SEC_OFF;
    tmp |= (U8)(hdr->aps_frm_ctrl.ack_format & 0x1)     << APS_ACK_FMT_OFF;
    tmp |= (U8)(hdr->aps_frm_ctrl.delivery_mode & 0x3)  << APS_DELIV_MODE_OFF;
    tmp |= hdr->aps_frm_ctrl.frm_type & 0x3;
    return tmp;
}

/**************************************************************************/
/*!
    Generate the APS frame header. The frame header contains the following fields;
    - Frame control field
    - Destination endpoint
    - Group address
    - Profile ID
    - Source endpoint
    - APS counter
    - Extended header
*/
/**************************************************************************/
void aps_gen_header(buffer_t *buf, aps_hdr_t *hdr)
{
    U8 ahdr_size;
    bool    dest_ep_flag    = false,
            clust_id_flag   = false,
            src_ep_flag     = false,
            group_addr_flag = false,
            prof_id_flag    = false;

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr = hdr->grp_addr;
    // this removes the lint warning for this block only

    hdr->aps_fcf = aps_gen_frm_ctrl(hdr);

    // init to 2 bytes to include the aps fcf and aps counter which are always present
    ahdr_size = 2;

    // check if cluster id and profile id will be present
    // if data or ack frames, then both are present. For cmd frames, none are present.
    if ((hdr->aps_frm_ctrl.frm_type == APS_DATA_FRM) ||
        (hdr->aps_frm_ctrl.frm_type == APS_ACK_FRM))
    {
        ahdr_size += 4;
        clust_id_flag = true;
        prof_id_flag = true;
    }

    // check if dest and src ep present and group address present
    // if unicast or broadcast, src and dest ep present, no group addr
    // if indirect, src or dest ep present, no group addr
    // if group, src and group addr present, no dest ep
    switch (hdr->aps_frm_ctrl.delivery_mode)
    {
    case APS_UNICAST: // fall through
    case APS_BROADCAST:
        ahdr_size += 2;
        src_ep_flag     = true;
        dest_ep_flag    = true;
        break;
    case APS_INDIRECT:
        ahdr_size += 1;

        // TODO: Reconfirm this portion. The Zigbee 2007 spec changed the indir addr field to ack_format
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
        ahdr_size += 3;
        group_addr_flag = true;
        src_ep_flag     = true;
        break;
    default:
        // TODO: Collect statistic here. it should never reach this point.
        return;
    }

    // fill in the length and adjust the data pointer
    buf->dptr -= ahdr_size;
    buf->len += ahdr_size;

    // fill in the fields of the aps header
    *buf->dptr++ = hdr->aps_fcf;

    if (dest_ep_flag)
    {
        *buf->dptr++ = hdr->dest_ep;
    }

    if (group_addr_flag)
    {
        *(U16 *)buf->dptr = hdr->grp_addr;
        buf->dptr += sizeof(U16);
    }

    if (clust_id_flag)
    {
        *(U16 *)buf->dptr = hdr->clust_id;
        buf->dptr += sizeof(U16);
    }

    if (prof_id_flag)
    {
        *(U16 *)buf->dptr = hdr->prof_id;
        buf->dptr += sizeof(U16);
    }

    if (src_ep_flag)
    {
        *(U8 *)buf->dptr = hdr->src_ep;
        buf->dptr += sizeof(U8);
    }

    *buf->dptr++ = hdr->aps_ctr;

    // roll back the data pointer to the beginning of the header
    buf->dptr -= ahdr_size;
}



