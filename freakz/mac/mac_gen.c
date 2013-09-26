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
    \file mac_gen.c
    \ingroup mac
    \brief MAC frame generator

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the generation of 802.15.4 header and command data inside
    the specified transmit buffer. Since 802.15.4 specifies variable length fields
    inside its frames, then simple pointer overlays could not be used (as with
    protocols that use fixed length fields). Hence many helper functions are needed
    to assemble the various frame data.
*/
#include <string.h>
#include "freakz.h"

/**************************************************************************/
/*!
    Generate the mac header frame control field from the given header data.
*/
/**************************************************************************/
U16 mac_gen_frm_ctrl(const mac_hdr_t *hdr)
{
    U16 tmp = 0;
    tmp |= (U16)((hdr->src_addr.mode                     & 0x3) << MAC_SRC_ADDR_MODE_OFF);
    tmp |= (U16)((hdr->dest_addr.mode                    & 0x3) << MAC_DEST_ADDR_MODE_OFF);
    tmp |= (U16)((hdr->mac_frm_ctrl.pan_id_compr         & 0x1) << MAC_PAN_ID_COMPR_OFF);
    tmp |= (U16)((hdr->mac_frm_ctrl.ack_req              & 0x1) << MAC_ACK_REQ_OFF);
    tmp |= (U16)((hdr->mac_frm_ctrl.frame_pending        & 0x1) << MAC_FRM_PEND_OFF);
    tmp |= (U16)(hdr->mac_frm_ctrl.frame_type            & 0x3);
    return tmp;
}

/**************************************************************************/
/*!
    Generate the complete MAC frame header and place it in the specified
    buffer. The generated data gets placed in the buffer backwards (ie: from
    the end to the beginning) since we are moving down the stack. You can see
    this happening where we move the bufs data pointer backwards by 'len' bytes.
    We then crawl forward, moving the data pointer up as we fill in the various
    fields.
*/
/**************************************************************************/
void mac_gen_header(buffer_t *buf, mac_hdr_t *hdr)
{
    U8 hdr_size, frame_length;
    hdr->mac_fcf = mac_gen_frm_ctrl(hdr);

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr  = hdr->mac_fcf;
    // this removes the lint warning for this block only

    // Calculate the size of the header
    hdr_size = 4;                                               // length(1) + fcf (2) + dsn (1) = 4 bytes
    if (hdr->dest_addr.mode > 0)
    {
        hdr_size += ((hdr->dest_addr.mode == SHORT_ADDR) ? 4 : 10);     // dest_pan_id (2) + dest_addr (2/8) = 4/10 bytes
    }

    if (hdr->src_addr.mode > 0)
    {
        hdr_size += (hdr->mac_frm_ctrl.pan_id_compr == true) ?  0 : 2;  // if pan_id_compr, then we don't have src_pan_id
        hdr_size += (hdr->src_addr.mode == SHORT_ADDR) ? 2 : 8;
    }

    // Shift the frame_buf->first pointer back (size) bytes to accomodate for the header.
    // Then calculate the length of the frame and add it to the buffer size field. We need to
    // add not only the header size but also 2 bytes for the FCS.
    buf->dptr -= hdr_size;
    buf->len += hdr_size;
    frame_length = ((buf->len) - 1) + MAC_FCS_LENGTH;           // The '-1' is because we don't count the length byte in the frame length

    // Start filling in the frame header. Write in the data, then advance the data pointer.
    *buf->dptr++        = frame_length;
    *(U16 *)buf->dptr   = hdr->mac_fcf;
    buf->dptr           += sizeof(U16);
    *buf->dptr++        = hdr->dsn;

    // pan_id and addresses
    if (hdr->dest_addr.mode > 0)
    {
        // dst_pan_id
        *(U16 *)buf->dptr = hdr->dest_pan_id;
        buf->dptr += sizeof(U16);

        // dst_addr
        if (hdr->dest_addr.mode == SHORT_ADDR)
        {
            *(U16 *)buf->dptr = hdr->dest_addr.short_addr;
            buf->dptr += sizeof(U16);
        }
        else
        {
            *(U64 *)buf->dptr = hdr->dest_addr.long_addr;
            buf->dptr += sizeof(U64);
        }
    }

    if (hdr->src_addr.mode > 0)
    {
        // src_pan_id
        // if pan_id_compr is used, then we won't have a src_pan_id
        if (!hdr->mac_frm_ctrl.pan_id_compr)
        {
            *(U16 *)buf->dptr = hdr->src_pan_id;
            buf->dptr += sizeof(U16);
        }

        // src_addr
        if (hdr->src_addr.mode == SHORT_ADDR)
        {
            *(U16 *)buf->dptr = hdr->src_addr.short_addr;
            buf->dptr += sizeof(U16);
        }
        else
        {
            *(U64 *)buf->dptr = hdr->src_addr.long_addr;
            buf->dptr += sizeof(U64);
        }
    }

    // Move the data pointer to the first byte of this frame
    buf->dptr -= hdr_size;
}

/**************************************************************************/
/*!
    Generate a header for a MAC command frame. The header for a command frame
    is essentially the same as a data header, except for the frame type. Also,
    command frames require an ACK. Other than that, we will use the generic
    mac gen header function to generate the rest of the fields.
*/
/**************************************************************************/
void mac_gen_cmd_header(buffer_t *buf, mac_hdr_t *hdr, bool ack_req, address_t *src_addr, address_t *dest_addr)
{
    mac_pib_t *pib = mac_pib_get();

    memset(hdr, 0, sizeof(mac_hdr_t));
    hdr->mac_frm_ctrl.frame_type        = MAC_COMMAND;
    hdr->mac_frm_ctrl.ack_req           = ack_req;
    hdr->mac_frm_ctrl.frame_ver         = MAC_802_15_4_2006;
    hdr->dsn                            = pib->dsn++;

    hdr->dest_pan_id = hdr->src_pan_id  = pib->pan_id;
    memcpy(&hdr->src_addr, src_addr, sizeof(address_t));
    memcpy(&hdr->dest_addr, dest_addr, sizeof(address_t));
    mac_gen_header(buf, hdr);
    debug_dump_mac_hdr(hdr);
}

/**************************************************************************/
/*!
    Generate an ack frame and put it in the given buffer. The ack generation is
    a little bit ugly because we need the ack to be fast. It needs to be out
    within 1 msec. Most chips support auto-ACK so this function may end up being
    obsoleted in the future.
*/
/**************************************************************************/
void mac_gen_ack(buffer_t *buf, bool frm_pend, U8 dsn)
{
    U8  hdr_size = 4,
        frame_len = 5;

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr  = hdr->mac_fcf;
    // this removes the lint warning for this block only

    // the magic numbers i'm using are for the fcf. they are limited to these two
    // values for an ack frame.
    U16 fcf = (frm_pend) ? 0x0012 : 0x0002;

    buf->len += hdr_size;
    buf->dptr -= hdr_size;

    // fill out the ack fields. don't use the dptr, we'll manually address the pointers.
    *buf->dptr = frame_len;
    *(U16 *)(buf->dptr+1) = fcf;
    *(buf->dptr + 3) = dsn;
}

/**************************************************************************/
/*!
    Generate a command frame and put it in the given buffer. The command frame
    is based on the cmd argument that is passed in. The format that will be
    implemented is based on the cmd_id of the argument.
*/
/**************************************************************************/
void mac_gen_cmd(buffer_t *buf, mac_cmd_t *cmd)
{
    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr  = hdr->mac_fcf;
    // this removes the lint warning for this block only

    switch (cmd->cmd_id)
    {
    case MAC_DATA_REQ:
    case MAC_BEACON_REQ:
    case MAC_ORPHAN_NOT:
        buf->len++;
        buf->dptr--;
        *buf->dptr = cmd->cmd_id;
        break;

    case MAC_ASSOC_REQ:
        buf->len    += MAC_ASSOC_REQ_LEN;
        buf->dptr   -= MAC_ASSOC_REQ_LEN;
        *buf->dptr++ = cmd->cmd_id;
        *buf->dptr++ = cmd->assoc_req.cap_info;
        buf->dptr   -= MAC_ASSOC_REQ_LEN;
        break;

    case MAC_ASSOC_RESP:
        buf->len    += MAC_ASSOC_RESP_LEN;
        buf->dptr   -= MAC_ASSOC_RESP_LEN;
        *buf->dptr++        = cmd->cmd_id;
        *(U16 *)buf->dptr   = cmd->assoc_resp.short_addr;
        buf->dptr += sizeof(U16);
        *buf->dptr++        = cmd->assoc_resp.assoc_status;
        buf->dptr -= MAC_ASSOC_RESP_LEN;
        break;

    case MAC_COORD_REALIGN:
        buf->len += MAC_COORD_REALIGN_LEN;
        buf->dptr -= MAC_COORD_REALIGN_LEN;
        *buf->dptr++        = cmd->cmd_id;
        *(U16 *)buf->dptr   = cmd->coord_realign.pan_id;
        buf->dptr += sizeof(U16);
        *(U16 *)buf->dptr   = cmd->coord_realign.coord_short_addr;
        buf->dptr += sizeof(U16);
        *buf->dptr++        = cmd->coord_realign.channel;
        *(U16 *)buf->dptr   = cmd->coord_realign.short_addr;
        buf->dptr += sizeof(U16);
        buf->dptr -= MAC_COORD_REALIGN_LEN;
        break;

    default:
        break;
    }
    debug_dump_mac_cmd(cmd);
}

/**************************************************************************/
/*!
    Generate the superframe spec field. This field is used in MAC beacons.
*/
/**************************************************************************/
U16 mac_gen_superfrm_spec()
{
    mac_pib_t *pib = mac_pib_get();
    U16 superfrm_spec = 0;

    superfrm_spec |= (pib->assoc_permit & 0x1)  << MAC_ASSOC_PERMIT_OFF;
    superfrm_spec |= (pib->coord & 0x1)         << MAC_COORDINATOR_OFF;
    superfrm_spec |= MAC_FINAL_CAP_SLOT | MAC_SUPERFRAME_ORDER | MAC_BEACON_ORDER;
    return superfrm_spec;
}

/**************************************************************************/
/*!
    Generate a beacon frame and put it in the specified buffer. For this stack,
    a beacon is only needed when a beacon request arrives. Usually when a node
    starts up, it will do a network scan by broadcasting a beacon request command frame.
    All nodes within listening range will then need to generate a beacon and send
    it to the requestor. This is the main use for a beacon in Zigbee.

    Before we generate the MAC beacon, we first need to retrieve the NWK beacon data,
    which consists of nwk info such as end device capacity, router capacity, and
    nwk device info. After that, we can generate the MAC beacon.
*/
/**************************************************************************/
static void mac_gen_beacon(buffer_t *buf)
{
    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // ex: *(U16 *)buf->dptr  = hdr->mac_fcf;
    // this removes the lint warning for this block only

    // we need the nwk payload. since it's constant, we will just grab it
    // directly from the nwk layer. it kind of sucks but we have to break
    // the layer boundary here.
    nwk_gen_beacon(buf);

    // now that we have the nwk beacon payload, we will build the mac portion of the beacon.
    buf->len            += MAC_BEACON_PAYLOAD_SIZE;
    buf->dptr           -= MAC_BEACON_PAYLOAD_SIZE;
    *(U16 *)buf->dptr   = mac_gen_superfrm_spec();      // superfrm spec
    buf->dptr           += sizeof(U16);
    *buf->dptr++        = 0;                            // gts list
    *buf->dptr++        = 0;                            // pend addr list
    buf->dptr           -= MAC_BEACON_PAYLOAD_SIZE;
}

/**************************************************************************/
/*!
    Generate the full beacon frame and send it out. This function calls
    the gen_beacon function and after the beacon is generated, it will
    slap a MAC header on it and send it out.
*/
/**************************************************************************/
void mac_gen_beacon_frm(buffer_t *buf, mac_hdr_t *hdr)
{
    mac_pib_t *pib = mac_pib_get();

    hdr->mac_frm_ctrl.frame_type     = MAC_BEACON;
    hdr->mac_frm_ctrl.ack_req        = false;
    hdr->mac_frm_ctrl.frame_pending  = false;
    hdr->mac_frm_ctrl.frame_ver      = MAC_802_15_4_2006;
    hdr->mac_frm_ctrl.pan_id_compr   = false;
    hdr->src_pan_id                  = pib->pan_id;
    hdr->dest_pan_id                 = pib->pan_id;
    hdr->src_addr.mode               = SHORT_ADDR;
    hdr->src_addr.short_addr         = pib->short_addr;
    hdr->dest_addr.mode              = NO_PAN_ID_ADDR;
    hdr->dsn                         = pib->dsn++;

    mac_gen_beacon(buf);
    mac_gen_header(buf, hdr);
    debug_dump_mac_hdr(hdr);
}
