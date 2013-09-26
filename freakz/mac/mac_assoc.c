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
    \file mac_assoc.c
    \ingroup mac
    \brief MAC association

    This file implements the MAC association service. It handles the
    association request, response, and also the orphan response for
    nodes that have been orphaned from their parents.
*/
#include "freakz.h"

/**************************************************************************/
/*!
    An association request frame is used to request to join a router. This function
    will generate an association request command frame and send it out. The argument
    data will be used to generate the mac header and the command header.
*/
/**************************************************************************/
void mac_assoc_req(mac_assoc_req_t *args_in)
{
    mac_pcb_t *pcb = mac_pcb_get();
    mac_pib_t *pib = mac_pib_get();
    buffer_t *buf;
    address_t src_addr;
    mac_cmd_t cmd;
    mac_hdr_t hdr;

    // change the pan id and channel to the specified args
    memcpy(&pib->coord_addr, &args_in->coord_addr, sizeof(address_t));

    // set the channel and pan id to the one that we're going to join
    mac_set_channel(args_in->channel);
    mac_set_pan_id(args_in->coord_pan_id);

    // generate the association request frame
    BUF_ALLOC(buf, TX);
    cmd.cmd_id = MAC_ASSOC_REQ;
    cmd.assoc_req.cap_info = args_in->capability;
    mac_gen_cmd(buf, &cmd);

    src_addr.mode       = LONG_ADDR;
    src_addr.long_addr  = pib->ext_addr;
    mac_gen_cmd_header(buf, &hdr, true, &src_addr, &args_in->coord_addr);

    // set the state to association request and save some info that we'll use later
    memcpy(&pcb->parent_addr, &args_in->coord_addr, sizeof(address_t));
    pcb->assoc_req_dsn  = hdr.dsn;
    pcb->mac_state      = MLME_ASSOC_REQ;

    mac_tx_handler(buf, &hdr.dest_addr, false, true, hdr.dsn, ZIGBEE_INVALID_HANDLE);
}

/**************************************************************************/
/*!
    An association response frame is an indirect frame that is generated
    in response to an association request. If the association request was successful,
    then the association response will contain the address for the requestor. If
    unsuccessful, then the status will display a failure message and the reason for
    the failure.

    This function generates the association response command frame and
    sends it to the indirect transmission queue for retrieval by the requestor.
*/
/**************************************************************************/
void mac_assoc_resp(mac_assoc_resp_t *args)
{
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    address_t src_addr;
    mac_pib_t *pib = mac_pib_get();
    buffer_t *buf = NULL;

    // generate the association response command frame
    BUF_ALLOC(buf, TX);

    cmd.cmd_id                  = MAC_ASSOC_RESP;
    cmd.assoc_resp.assoc_status = (U8)args->status;
    cmd.assoc_resp.short_addr   = args->assoc_short_addr;

    src_addr.mode               = SHORT_ADDR;
    src_addr.short_addr         = pib->short_addr;

    mac_gen_cmd(buf, &cmd);
    mac_gen_cmd_header(buf, &hdr, true, &src_addr, &args->dev_addr);
    mac_tx_handler(buf, &hdr.dest_addr, true, true, hdr.dsn, ZIGBEE_INVALID_HANDLE);
}

/**************************************************************************/
/*!
    If a node loses contact with its parent node, then it is considered an orphan.
    It will then send an orphan request command frame to see if its parent is still
    there. If the parent is there, then it will send an orphan response letting
    the child node know that it is still there. Otherwise the child node will need
    to rejoin another parent.

    This function generates an orphan response and sends it out to the specified address.
*/
/**************************************************************************/
void mac_orphan_resp(U64 orphan_addr, U16 short_addr)
{
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    buffer_t *buf;
    address_t src_addr, dest_addr;
    mac_pib_t *pib = mac_pib_get();

    cmd.cmd_id                          = MAC_COORD_REALIGN;
    cmd.coord_realign.pan_id            = pib->pan_id;
    cmd.coord_realign.coord_short_addr  = pib->short_addr;
    cmd.coord_realign.channel           = pib->curr_channel;
    cmd.coord_realign.short_addr        = short_addr;

    src_addr.mode                       = LONG_ADDR;
    src_addr.long_addr                  = pib->ext_addr;
    dest_addr.mode                      = LONG_ADDR;
    dest_addr.long_addr                 = orphan_addr;

    BUF_ALLOC(buf, TX);
    mac_gen_cmd(buf, &cmd);
    mac_gen_cmd_header(buf, &hdr, true, &src_addr, &dest_addr);
    mac_tx_handler(buf, &hdr.dest_addr, false, true, hdr.dsn, ZIGBEE_INVALID_HANDLE);
}
