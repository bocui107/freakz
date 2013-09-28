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
    \file mac_poll.c
    \ingroup mac
    \brief MAC poll handler

    This file handles the MAC poll service which is used to periodically
    poll the parent node to see if any data is pending for this node.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Generate a data request command frame and send it out. For a sleepy end
    device, the next higher layer will periodically generate a poll request
    to check to see if there is any data for this device since the last sleep
    interval.

    When this function gets called, we go into the MLME_DATA_REQ state so
    since there are some special actions we need to perform during a data request.
*/
/**************************************************************************/
void mac_poll_req()
{
    buffer_t *buf;
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    address_t src_addr;
    mac_pib_t *pib = mac_pib_get();
    mac_pcb_t *pcb = mac_pcb_get();

    BUF_ALLOC(buf, TX);

    // send a data request as part of the association process
    cmd.cmd_id = MAC_DATA_REQ;
    mac_gen_cmd(buf, &cmd);

    if (pcb->mac_state == MLME_ASSOC_REQ)
    {
        // we are in the middle of an association sequence. we don't have a short address yet
        // so use the long address.
        src_addr.mode               = LONG_ADDR;
        src_addr.long_addr          = pib->ext_addr;
    }
    else
    {
        // this is a standard data request. Use our network address.
        hdr.src_addr.mode           = SHORT_ADDR;
        hdr.src_addr.short_addr     = pib->short_addr;
    }
    mac_gen_cmd_header(buf, &hdr, true, &src_addr, &pcb->parent_addr);
    mac_tx_handler(buf, &hdr.dest_addr, false, true, hdr.dsn, ZIGBEE_INVALID_HANDLE);
    pcb->mac_state = MLME_DATA_REQ;
}

/**************************************************************************/
/*!
    This is a callback that gets called from when the data request timer expires.
    When the data request command frame is sent, the parent will send back an ACK
    with the data pending bit indicating whether there is data for that device or not.
    If there is data, the data request timer starts and we wait for the data to come in.
    If no data ever comes, then this function will get called and we send a confirmation
    to the next higher layer informing it of the failure.
*/
/**************************************************************************/
void mac_poll_timeout()
{
    mac_pcb_t *pcb = mac_pcb_get();

    mac_poll_conf(MAC_NO_DATA);
    DBG_PRINT("MAC_POLL_TIMEOUT: Data request timed out.\n");
    pcb->mac_state = MLME_IDLE;
}


