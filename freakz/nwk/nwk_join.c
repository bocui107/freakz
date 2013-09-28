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
    \file nwk_join.c
    \ingroup nwk
    \brief NWK join

    This file handles the NWK join service. We can either request to join
    a parent chosen by the higher layers, or we can handle a join request
    coming in from another node.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Request to join a node specified in the request args. To do this, we need
    to change to the channel of interest and issue a MAC association request
    with the router's address, the PAN ID, and our capability information.
    The MAC association request will then assemble an association request
    command frame and send it to the prospective parent.
*/
/**************************************************************************/
void nwk_join_req(const nwk_join_req_t *args)
{
    mac_assoc_req_t assoc_args;
    nwk_pcb_t *pcb = nwk_pcb_get();
    nwk_nib_t *nib = nwk_nib_get();
    U8 capability;

    // check if we're joined to a network. if we already are, then quit
    if (nib->joined)
    {
        nwk_join_conf(NWK_INVALID_REQUEST, 0, (U64)0, 0);
        return;
    }

    // if a candidate was found, then start the mac association process. fill out the arguments for the mac
    // association. first, we need to adjust the capability info based on the join_as_rtr setting.
    capability = nib->capability_info;
    if (args->join_as_rtr)
    {
        // if the router bit is not set in the capability info and we want to join as a router, then we need to set it.
        capability |= (nib->capability_info & NWK_CAPAB_DEV_TYPE_MASK) ? 0 : NWK_CAPAB_DEV_TYPE_MASK;
    }
    else
    {
        // if the router bit is set and we don't want to join as router, then we need to clear it.
        capability &= (nib->capability_info & NWK_CAPAB_DEV_TYPE_MASK) ? ~NWK_CAPAB_DEV_TYPE_MASK : 0xFF;
    }
    assoc_args.capability   = capability;
    assoc_args.channel      = SCAN_ENTRY(args->desc)->channel;
    assoc_args.coord_pan_id = SCAN_ENTRY(args->desc)->coord_pan_id;
    memcpy(&assoc_args.coord_addr, &SCAN_ENTRY(args->desc)->coord_addr, sizeof(address_t));

    // keep a record of which entry we are trying to join.
    pcb->curr_join_target   = args->desc;
    pcb->join_as_rtr        = args->join_as_rtr;
    mac_assoc_req(&assoc_args);
}

/**************************************************************************/
/*!
    This function is called after the MAC is finished with the association process.
    It will indicate the status and if the association was successful, also
    the allocated network address.
*/
/**************************************************************************/
void mac_assoc_conf(U16 addr, U8 mac_status)
{
    nwk_pcb_t *pcb = nwk_pcb_get();
    nwk_nib_t *nib = nwk_nib_get();
    mem_ptr_t *nbor_mem_ptr;
    U8 status;

    // start off with the failure status. if the join was a success, then we can change the status.
    status = NWK_NOT_PERMITTED;

    // get the neighbor table entry
    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&SCAN_ENTRY(pcb->curr_join_target)->coord_addr)) == NULL)
    {
        return;
    }

    if (mac_status == MAC_SUCCESS)
    {
        NBOR_ENTRY(nbor_mem_ptr)->relationship = NWK_PARENT;

        // set up the device now that we are joined
        nib->joined             = true;
        nib->short_addr         = addr;
        nib->ext_pan_ID         = NBOR_ENTRY(nbor_mem_ptr)->ext_pan_id;
        nib->depth              = NBOR_ENTRY(nbor_mem_ptr)->depth + 1;
        nib->cskip              = nwk_rte_tree_calc_cskip(nib->depth);

        // don't forget to set the short address and pan ID in the MAC
        mac_set_short_addr(addr);
        status = NWK_SUCCESS;

        nwk_join_conf(status, addr, nib->ext_pan_ID, SCAN_ENTRY(pcb->curr_join_target)->channel);

        // the join was successful. we can clear the whole descriptor list and free up the RAM.
        mac_scan_descr_clear();
    }
    else
    {
        SCAN_ENTRY(pcb->curr_join_target)->pot_parent = false;
        NBOR_ENTRY(nbor_mem_ptr)->pot_parent = false;
        mac_set_pan_id(0xFFFF);         // reset the pan id to broadcast
        nwk_join_conf(NWK_NOT_PERMITTED, 0, (U64)0, 0);
    }
}

/**************************************************************************/
/*!
    This is called when an association request is received. It means that a device
    is trying to join the network and wants to be a child of this router. So we
    start the parent side network join procedure.
*/
/**************************************************************************/
void mac_assoc_ind(mac_assoc_ind_t *args)
{
    mac_pib_t *pib = mac_pib_get();
    nwk_nib_t *nib = nwk_nib_get();
    U8 dev_type;
    bool rx_idle;
    bool dev_match = false, rx_idle_match = false;
    mac_assoc_resp_t resp_args;
    address_t addr;
    mem_ptr_t *nbor_mem_ptr;

    addr.mode       = LONG_ADDR;
    addr.long_addr  = args->dev_addr.long_addr;

    // check to see if the device is already in our neighbor table.
    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&addr)) != NULL)
    {
        // a device was previously associated with the same long address. most likely, it was the same device
        // unless there's a coincidence that allows two devices to have 64-bit extended addresses.

        // the following match comparisons will return true if the comparisons work out. the capability info determines which
        // comparison will be used.
        dev_type        = NBOR_ENTRY(nbor_mem_ptr)->device_type;
        rx_idle         = NBOR_ENTRY(nbor_mem_ptr)->rx_on_when_idle;

        dev_match       = (args->capability & NWK_CAPAB_DEV_TYPE_MASK) ?
                         ((dev_type == NWK_ROUTER) || (dev_type == NWK_COORDINATOR)) : (dev_type == NWK_END_DEVICE);

        rx_idle_match   = (args->capability & NWK_CAPAB_RX_ON_WHEN_IDLE_MASK) ? rx_idle : !rx_idle;

        if (dev_match && rx_idle_match)
        {
            // if the device type and the rx_idle values match, then we can pretty much assume that its the same device trying
            // to associated. just use the nwk address that was previously allocated.
            memcpy(&resp_args.dev_addr, &args->dev_addr, sizeof(address_t));
            resp_args.assoc_short_addr = NBOR_ENTRY(nbor_mem_ptr)->nwk_addr;
            resp_args.status = MAC_SUCCESS;
            mac_assoc_resp(&resp_args);
            return;
        }
        else
        {
            // there's no match. this is a different device. delete the entry and continue with the join process.
            nwk_neighbor_tbl_rem(&addr);
        }
    }

    // the device has not joined this parent before. start the join process.
    if ((args->capability & NWK_CAPAB_DEV_TYPE_MASK) && (nib->rtr_cap > 0))
    {
        // a router is trying to join. gen the address.
        resp_args.assoc_short_addr  = nwk_rte_tree_calc_rtr_addr(nib->cskip, (nib->max_routers - nib->rtr_cap));
        resp_args.status            = MAC_SUCCESS;
        nib->rtr_cap--;
    }
    else if (!(args->capability & NWK_CAPAB_DEV_TYPE_MASK) && (nib->ed_cap > 0))
    {
        // and end device is trying to join. gen the address.

        U8 end_dev_cap =  nib->max_children - (nib->max_routers + nib->ed_cap);
        resp_args.assoc_short_addr  = nwk_rte_tree_calc_ed_addr(nib->cskip, end_dev_cap);
        resp_args.status            = MAC_SUCCESS;
        nib->ed_cap--;
    }
    else
    {
        // we're out of capacity for the particular device. set the status to out of capacity and choose a dummy address.
        resp_args.assoc_short_addr  = 0xFFFF;
        resp_args.status            = MAC_OUT_OF_CAP;
    }

    // if the join was a success, then gen a neighbor entry for this device
    if (resp_args.status == MAC_SUCCESS)
    {
        U8 dev_type = (args->capability & NWK_CAPAB_DEV_TYPE_MASK) ? NWK_ROUTER : NWK_END_DEVICE;
        bool rx_on_idle = (args->capability & NWK_CAPAB_RX_ON_WHEN_IDLE_MASK) >> NWK_CAPAB_RX_ON_IDLE_OFFSET;
        nbor_tbl_entry_t entry;

        memset(&entry, 0, sizeof(nbor_tbl_entry_t));
        entry.ext_addr          = args->dev_addr.long_addr;
        entry.nwk_addr          = resp_args.assoc_short_addr;
        entry.pan_id            = pib->pan_id;
        entry.device_type       = dev_type;
        entry.relationship      = NWK_CHILD;
        entry.rx_on_when_idle   = rx_on_idle;
        entry.channel           = pib->curr_channel;
        entry.depth             = nib->depth + 1;
        nwk_neighbor_tbl_add(&entry);
    }

    // send the association response
    memcpy(&resp_args.dev_addr, &args->dev_addr, sizeof(address_t));
    mac_assoc_resp(&resp_args);
    nwk_join_ind(args->dev_addr.long_addr, resp_args.assoc_short_addr);
}

/**************************************************************************/
/*!
    Indication that an orphan notification command frame was received. If the
    device sending us the orphan notification was previously associated with
    us, then we send back an orphan response with the PAN ID, our address,
    and the device's nwk address.
*/
/**************************************************************************/
void mac_orphan_ind(U64 sender)
{
    nwk_nib_t *nib = nwk_nib_get();
    address_t addr;
    mem_ptr_t *nbor_mem_ptr;

    addr.mode       = LONG_ADDR;
    addr.long_addr  = sender;

    if (nib->dev_type == NWK_END_DEVICE)
    {
        return;
    }

    // look for the long address of the sender in the neighbor table. If found,
    // then send an orphan response.
    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&addr)) != NULL)
    {
        if (NBOR_ENTRY(nbor_mem_ptr)->relationship == NWK_CHILD)
        {
            mac_orphan_resp(sender, NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
        }
    }
}
