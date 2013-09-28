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
    \file nwk_leave.c
    \ingroup nwk
    \brief NWK leave

    This file handles the procedure to request the device or a child
    device to leave the network.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Send out the NWK leave request frame. This frame is used to request a device
    or a device's child node to leave the network.
*/
/**************************************************************************/
static void nwk_leave_send_req(nwk_hdr_t *hdr, nwk_cmd_t *cmd)
{
    buffer_t *buf;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    hdr->nwk_frm_ctrl.frame_type         = NWK_CMD_FRM;
    hdr->nwk_frm_ctrl.disc_route         = false;
    hdr->nwk_frm_ctrl.src_ieee_addr_flag = true;
    hdr->src_ieee_addr                   = pib->ext_addr;
    hdr->src_addr                        = nib->short_addr;
    hdr->radius                          = 1;
    hdr->seq_num                         = nib->seq_num;

    BUF_ALLOC(buf, TX);
    nwk_gen_cmd(buf, cmd);
    debug_dump_nwk_cmd(cmd);
    nwk_fwd(buf, hdr);
}

/**************************************************************************/
/*!
    Request the device to leave the network or for specific children to be
    kicked off the network. This function is used to generate the NWK leave
    request frame which is later sent out by the nwk_leave_send_req function.
    We can either choose to ask a device to leave, or its children to leave.
*/
/**************************************************************************/
void nwk_leave_req(U64 addr, bool remove_children, bool rejoin)
{
    nwk_cmd_t cmd;
    nwk_hdr_t hdr;
    nwk_nib_t *nib = nwk_nib_get();
    address_t leave_addr;
    mem_ptr_t *nbor_mem_ptr;

    leave_addr.mode         = LONG_ADDR;
    leave_addr.long_addr    = addr;

    memset(&hdr, 0, sizeof(nwk_hdr_t));
    cmd.cmd_frm_id      = NWK_CMD_LEAVE;
    cmd.leave.cmd_opts  = 0;

    // addr is NULL, then we remove ourselves from the network. otherwise we request the specified
    // device to leave the network.
    if (addr)
    {
        // if address isn't in our table, then return an error response
        if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&leave_addr)) == NULL)
        {
            nwk_leave_conf(&leave_addr, NWK_UNKNOWN_DEVICE);
            return;
        }

        // we are being requested to remove a child. set the request option in the cmd_options field.
        cmd.leave.cmd_opts |= (1 << NWK_LEAVE_REQUEST_OFFSET);
        cmd.leave.cmd_opts |= (remove_children) ? (1<<NWK_LEAVE_REM_CHILDREN_OFFSET)  : 0;
        cmd.leave.cmd_opts |= (rejoin)          ? (1<<NWK_LEAVE_REJOIN_OFFSET)        : 0;

        hdr.nwk_frm_ctrl.dest_ieee_addr_flag    = true;
        hdr.dest_ieee_addr                      = addr;
        hdr.dest_addr                           = NBOR_ENTRY(nbor_mem_ptr)->nwk_addr;
    }
    else
    {
        cmd.leave.cmd_opts |= (remove_children) ? (1<<NWK_LEAVE_REM_CHILDREN_OFFSET)  : 0;
        cmd.leave.cmd_opts |= (rejoin)          ? (1<<NWK_LEAVE_REJOIN_OFFSET)        : 0;

        // remove self from network
        hdr.dest_addr = NWK_BROADCAST_RXONIDLE;
        nib->ext_pan_ID = 0;
    }
    nwk_leave_send_req(&hdr, &cmd);
    nwk_leave_conf(&leave_addr, NWK_SUCCESS);
}

/**************************************************************************/
/*!
    When we receive a leave request, we need to process it and either remove
    ourselves from the network or kick out our children.
*/
/**************************************************************************/
void nwk_leave_handler(const nwk_hdr_t *hdr, const nwk_cmd_t *cmd)
{
    bool rejoin, rem_children, request;
    address_t src_addr, addr;
    mem_ptr_t *nbor_mem_ptr;

    src_addr.mode       = SHORT_ADDR;
    src_addr.short_addr = hdr->src_addr;

    rejoin          = (cmd->leave.cmd_opts >> NWK_LEAVE_REJOIN_OFFSET) & 0x1;
    rem_children    = (cmd->leave.cmd_opts >> NWK_LEAVE_REM_CHILDREN_OFFSET) & 0x1;
    request         = (cmd->leave.cmd_opts >> NWK_LEAVE_REQUEST_OFFSET) & 0x1;

    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&src_addr)) != NULL)
    {
        if (NBOR_ENTRY(nbor_mem_ptr)->relationship == NWK_PARENT)
        {
            if (request || rem_children)
            {
                // this is our parent. we need to leave the network
                addr.mode = LONG_ADDR;
                addr.long_addr = 0;

                nwk_leave_req((U64)0, rem_children, rejoin);
                nwk_leave_ind(&addr, rejoin);
            }
        }
        else
        {
            nwk_leave_ind(&src_addr, rejoin);
        }
    }
}
