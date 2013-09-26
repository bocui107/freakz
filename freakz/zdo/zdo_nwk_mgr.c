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
    \file zdo_nwk_mgr.c
    \ingroup zdo
    \brief ZDO network manager

    The network manager in the ZDO controls the (big surprise) network
    management functions of the device. It controls the starting of the
    device, network discovery, network and parent selection, and joining.

    This file also contains the network manager request handlers which are
    server functions of the NWK management object.
*/
/**************************************************************************/
#include "freakz.h"

// static globals used to store info during a remote nwk discovery operation
static mem_ptr_t *rmt_disc_mem_ptr;     ///< Remote nwk discovery mem pointer instantiation
static bool rmt_nwk_disc;               ///< Remote nwk discovery flag
static U8 rmt_nwk_disc_expiry;          ///< Remote nwk timeout value
static U8 leave_req_status;             ///< Leave request status

// static prototypes
static void nwk_disc_send_resp(U8 status);

/**************************************************************************/
/*!
    This should be the first function called and will initiate the start procedure
    for the zigbee device. If its the designated coordinator, then it will start
    the network. Otherwise, it will do a nwk discovery request and choose the best
    available network.
*/
/**************************************************************************/
void zdo_nwk_start()
{
    aps_aib_t *aib = aps_aib_get();

    DBG_PRINT("ZDO_START\n");
    rmt_nwk_disc = false;
    rmt_disc_mem_ptr = NULL;
    rmt_nwk_disc_expiry = 0;

    // reset the device first
    nwk_reset_req(true);

    // then start up the system based on whether we are the coord or router
    if (aib->desig_coord)
    {
        nwk_form_req(aib->chan_mask, DEFAULT_SCAN_DURATION);
    }
    else
    {
        nwk_disc_req(aib->chan_mask, DEFAULT_SCAN_DURATION);
    }
}

/**************************************************************************/
/*!
    This is a slow clock countdown timer to make sure that we clear the
    contents of any scan and reset the nwk disc state.
*/
/**************************************************************************/
void zdo_nwk_mgr_periodic()
{
    if (rmt_nwk_disc)
    {
        if (rmt_nwk_disc_expiry == 0)
        {
            rmt_nwk_disc = false;
            rmt_disc_mem_ptr = NULL;
        }
        else
        {
            rmt_nwk_disc_expiry--;
        }
    }
}

/**************************************************************************/
/*!
    Select the network to join. This function is called after a network discovery
    and will select the network from the list of scan descriptors. If the
    use_ext_pan_id flag is set in the APS Info Base, then it will search for that
    pan ID and join that network.

    Otherwise, it looks for the following criteria in evaluating a network to join:
    - The scan descriptor sender is a potential parent
    - It has capacity
    - It is permitting joining
    - The protocol version matches this device's version
    - The Zigbee stack profile matches this device's profile (ie: Zigbee vs Zigbee Pro)
*/
/**************************************************************************/
static bool zdo_nwk_select()
{
    aps_aib_t *aib = aps_aib_get();
    nwk_nib_t *nib = nwk_nib_get();
    zdo_pcb_t *pcb = zdo_pcb_get();
    nwk_join_req_t args;
    mem_ptr_t *mem_ptr;
    bool capacity           = false;
    bool permit_join        = false;
    bool prot_ver_match     = false;
    bool stack_prof_match   = false;

    // currently, we will join the first network we find that fits our join criteria
    for (mem_ptr = pcb->descr_list; mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // if we specify that we need to join a particular pan, then we need to find a node descr
        // that matches the ext pan id. otherwise, if there's no such limitation, then just go on
        // ahead.
        if ((!aib->use_ext_pan_id) || (aib->use_ext_pan_id == SCAN_ENTRY(mem_ptr)->ext_pan_id))
        {
            if ((aib->use_desig_parent && (aib->desig_parent == SCAN_ENTRY(mem_ptr)->coord_addr.short_addr)) || !aib->use_desig_parent)
            {
                capacity            = (nib->dev_type == NWK_ROUTER) ? (SCAN_ENTRY(mem_ptr)->rtr_cap >0) : (SCAN_ENTRY(mem_ptr)->end_dev_cap > 0);
                permit_join         = (SCAN_ENTRY(mem_ptr)->superfrm_spec & MAC_ASSOC_PERMIT_MASK) >> MAC_ASSOC_PERMIT_OFF;
                prot_ver_match      = (SCAN_ENTRY(mem_ptr)->prot_ver == ZIGBEE_PROTOCOL_VERSION);
                stack_prof_match    = (SCAN_ENTRY(mem_ptr)->stack_profile == ZIGBEE_STACK_PROFILE);

                if (SCAN_ENTRY(mem_ptr)->pot_parent && capacity && permit_join && prot_ver_match && stack_prof_match)
                {
                    args.desc           = pcb->curr_descr = mem_ptr;
                    args.ext_pan_id     = SCAN_ENTRY(mem_ptr)->ext_pan_id;
                    args.join_as_rtr    = (nib->dev_type == NWK_ROUTER);
                    args.rejoin_nwk     = (aib->use_ext_pan_id == 0) ? NWK_JOIN_NORMAL : NWK_JOIN_REJOIN;
                    nwk_join_req(&args);
                    return true;
                }
            }
        }
    }
    return false;
}

/**************************************************************************/
/*!
    Confirm that the network discovery has finished. After network discovery
    concludes, then this function is called. The scan descriptor list
    should already exist and the next task is to either select a network
    or return the network discovery response frame (if it was a NWK discovery
    request).
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'status' not referenced
void nwk_disc_conf(U8 status, mem_ptr_t *mem_ptr)
{
    nwk_nib_t *nib = nwk_nib_get();
    zdo_pcb_t *pcb = zdo_pcb_get();

    pcb->descr_list = mem_ptr;

    DBG_PRINT("NWK_DISC_CONFIRM\n");

    #if (TEST_SIM == 1)
    {
        sim_node_t *nd = node_get();
        U8 msg[50];

        sprintf((char *)msg, "node %d nwk disc success\n", nd->index);
        format_cmd_str(msg);
        sim_pipe_cmd_out(msg, strlen((char *)msg) + 1);
    }
    #endif

    if (!nib->joined)
    {
        if (rmt_nwk_disc)
        {
            // we're doing a remote network discovery. in this case, send a response.
            nwk_disc_send_resp(AF_SUCCESS);
        }
        else
        {
            // we're doing a normal network discovery so that we can do a network formation or join
            if (zdo_nwk_select())
            {
                DBG_PRINT("ZDO_DISC_CONF: Join requested.\n");
            }
            else
            {
                DBG_PRINT("ZDO_DISC_CONF: Can't find a network to join.\n");
                DBG_PRINT("ZDO_DISC_CONF: Cleaning up and going to idle.\n");
                mac_scan_descr_clear();
            }
        }
        return;
    }
    else
    {
        // if we're joined to a nwk but still asked to do a nwk discovery, then we need
        // to see if it was requested remotely. if not, then probably some error occurred.
        if (rmt_nwk_disc)
        {
            nwk_disc_send_resp(AF_SUCCESS);
        }
        else
        {
            DBG_PRINT("ZDO_DISC_CONF: Cannot join network. Already joined.\n");
            mac_scan_descr_clear();
        }
    }
}

/**************************************************************************/
/*!
    Confirm that the NWK join is finished. The status indicates if the join
    was successful or not. If it was successful, then we need to update our
    NWK and MAC layers with the new nwk address, panID, and channel. Also,
    we need to send out our end device announcement after we are successfully
    joined to the network.
*/
/**************************************************************************/
void nwk_join_conf(U8 status, U16 nwk_addr, U64 ext_pan_id, U8 channel)
{
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();
    zdo_cb_t *zdo_cb = zdo_cb_get();

    DBG_PRINT("NWK_JOIN_CONFIRM\n");
    if (status == NWK_SUCCESS)
    {
        nib->short_addr     = pib->short_addr = nwk_addr;
        nib->ext_pan_ID     = ext_pan_id;

        mac_scan_descr_clear();
        if (zdo_cb->nwk_join_conf)
        {
            zdo_cb->nwk_join_conf(status, nwk_addr, pib->coord_addr.short_addr);
        }

        // if we're shut down during idle time, then define a polling interval where we
        // will wake up and poll the parent.

        // TODO: We need to have a state machine that will wake the device up
        // if in idle to send out the poll.
        //if (!ZIGBEE_RX_ON_WHEN_IDLE)
        //{
        //    nwk_sync_start(ZIGBEE_POLLING_INTERVAL);
        //}

        // send out the device announcement
        //zdo_gen_dev_annce_req(NWK_BROADCAST_RXONIDLE, aib->seq++, nwk_addr, pib->ext_addr, nib->capability_info);

        //DBG_PRINT("ZDO_JOIN_CONF: Successfully joined the network.\n");

        //#if (TEST_SIM == 1)
        //{
        //    sim_node_t *nd = node_get();
        //    U8 msg[50];
        //
        //    sprintf(msg, "node %d nwk join success\n", nd->index);
        //    format_cmd_str(msg);
        //    sim_pipe_cmd_out(msg, strlen(msg) + 1);
        //}
        //#endif
    }
    else if (status == NWK_NO_NETWORKS)
    {
        // we were orphaned and we can't find our parent. restart the device and join another parent
        nib->joined = false;
        zdo_nwk_start();
    }
    else
    {
        // we failed on the previous join. try and find another nwk to join.
        // if no nwk is found, then select nwk will assert an error.
        if (!zdo_nwk_select())
        {
            // handle the case where we can't find a nwk to join
            DBG_PRINT("ZDO_JOIN_CONF: Can't find a network to join.\n");
            DBG_PRINT("ZDO_JOIN_CONF: Cleaning up and going to idle.\n");
            mac_scan_descr_clear();
        }
    }
}

/**************************************************************************/
/*!
    Confirm that the network formation has concluded. The status will indicate
    if it was successful or not.
*/
/**************************************************************************/
void nwk_form_conf(U8 status)
{
    zdo_cb_t *zdo_cb = zdo_cb_get();

    if (zdo_cb->nwk_form_conf)
    {
        zdo_cb->nwk_form_conf(status);
    }
}

/**************************************************************************/
/*!
    Confirm that the NWK leave operation has concluded.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'addr' not referenced
//lint -e{715} Info 715: Symbol 'status' not referenced
void nwk_leave_conf(address_t *addr, U8 status)
{
    if (status == NWK_SUCCESS)
    {
        leave_req_status = AF_SUCCESS;
        if (addr->mode == LONG_ADDR)
        {
            if (addr->long_addr == 0)
            {
                nwk_neighbor_tbl_clear();
                nwk_rte_tbl_clear();
            }
            else
            {
                mem_ptr_t *nbor_mem_ptr;

                if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(addr)) != NULL)
                {
                    nwk_rte_tbl_rem(NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
                    nwk_addr_map_rem_nwk_addr(NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
                }
                nwk_neighbor_tbl_rem(addr);
            }
        }
    }
    else if (status == NWK_UNKNOWN_DEVICE)
    {
        leave_req_status = AF_DEVICE_NOT_FOUND;
    }
}

/**************************************************************************/
/*!
    Indicate to the ZDO that a NWK leave command request has been received
    or that an associated device has successfully left the network.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'addr' not referenced
//lint -e{715} Info 715: Symbol 'rejoin' not referenced
void nwk_leave_ind(address_t *addr, bool rejoin)
{
    mem_ptr_t *nbor_mem_ptr;

    // we just removed ourselves from the network
    if (((addr->mode == SHORT_ADDR) && (addr->short_addr == 0)) ||
        ((addr->mode == LONG_ADDR) && (addr->long_addr == 0)))
    {
        nwk_neighbor_tbl_clear();
        nwk_rte_tbl_clear();
        nwk_addr_map_clear();
        return;
    }

    // we just were notified that another device removed themselves from the network
    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(addr)) != NULL)
    {
        nwk_rte_tbl_rem(NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
        nwk_addr_map_rem_nwk_addr(NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
        nwk_neighbor_tbl_rem(addr);
    }
}

/**************************************************************************/
/*!
    Indicate that a device has joined the network
*/
/**************************************************************************/
void nwk_join_ind(U64 ext_addr, U16 nwk_addr)
{
    zdo_cb_t *zdo_cb = zdo_cb_get();

    if (zdo_cb->nwk_join_ind)
    {
        zdo_cb->nwk_join_ind(nwk_addr);
    }
}

/**************************************************************************/
// ZDO NWK MANAGER - SERVER FUNCTIONS
/**************************************************************************/

/**************************************************************************/
/*!
    This is the NWK discovery request handler. When a NWK disc request comes in,
    this function will start a network discovery procedure in the MAC layer.
    After the NWK disc confirm function is called, the response will be sent
    to the requestor.
*/
/**************************************************************************/
void zdo_nwk_disc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    zdo_req_t req;

    zdo_parse_req(src_addr, data, clust, &req);

    if ((rmt_disc_mem_ptr = mem_heap_alloc(sizeof(rmt_nwk_disc_t))) == NULL)
    {
        // TODO: Send an error response here...
        return;
    }

    RMT_DISC(rmt_disc_mem_ptr)->start_idx = req.type.nwk_disc.start_index;
    RMT_DISC(rmt_disc_mem_ptr)->seq = req.seq;
    RMT_DISC(rmt_disc_mem_ptr)->src_addr = src_addr;
    rmt_nwk_disc = true;
    rmt_nwk_disc_expiry = ZDO_RMT_NWK_DISC_TIMEOUT;

    nwk_disc_req(req.type.nwk_disc.scan_channels, req.type.nwk_disc.scan_duration);
}

/**************************************************************************/
/*!
    This function is called to send a NWK disc response back to the device
    that requested the NWK discovery be performed.
*/
/**************************************************************************/
void nwk_disc_send_resp(U8 status)
{
    U8 i, size, *resp, *resp_info, resp_data[MAX_APS_PAYLOAD];
    mem_ptr_t *scan_mem_ptr;
    zdo_pcb_t *pcb = zdo_pcb_get();

    if (!rmt_disc_mem_ptr)
    {
        // TODO: send an error response here
        return;
    }

    resp_info = resp_data;
    resp = resp_data + 5; // 4 bytes for the info + 1 byte for the seq

    // fill out the resp first
    i = 0;
    for (scan_mem_ptr = pcb->descr_list; scan_mem_ptr != NULL; scan_mem_ptr = scan_mem_ptr->next)
    {
        if (i < RMT_DISC(rmt_disc_mem_ptr)->start_idx)
        {
            // increment i and don't do anything until we've reached the starting index
            i++;
            continue;
        }

        *(U64 *)resp = SCAN_ENTRY(scan_mem_ptr)->ext_pan_id;
        resp += sizeof(U64);
        *resp++ = SCAN_ENTRY(scan_mem_ptr)->channel;
        *resp++ = ((SCAN_ENTRY(scan_mem_ptr)->prot_ver & 0xf) << 4) | (SCAN_ENTRY(scan_mem_ptr)->stack_profile & 0xf);
        *resp++ = (U8)(SCAN_ENTRY(scan_mem_ptr)->superfrm_spec & 0xff);
        *resp++ = (SCAN_ENTRY(scan_mem_ptr)->superfrm_spec >> MAC_ASSOC_PERMIT_OFF) & 0x1;
        i++;
    }

    // fill out the resp info here
    *resp_info++ = RMT_DISC(rmt_disc_mem_ptr)->seq;             // seq
    *resp_info++ = status;                                      // status
    *resp_info++ = mac_scan_get_entry_cnt();                    // total entry count
    *resp_info++ = RMT_DISC(rmt_disc_mem_ptr)->start_idx;       // starting index
    *resp_info++ = i - RMT_DISC(rmt_disc_mem_ptr)->start_idx;   // number of entries in this frame

    size = resp - resp_data;
    zdo_tx(resp_data, size, RMT_DISC(rmt_disc_mem_ptr)->src_addr, NWK_DISC_RESP_CLUST, 0, af_handle_get());

    // if we've finished the operation, set the expiry to zero and let it clean things up for us.
    rmt_nwk_disc_expiry = (i == mac_scan_get_entry_cnt()) ? 0 : rmt_nwk_disc_expiry;
}

/**************************************************************************/
/*!
    The LQI request handler will generate a list of entries in our neighbor table
    and return them in the LQI response.
*/
/**************************************************************************/
void zdo_nwk_lqi_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    U8 i, size, entry_cnt, tmp, *resp_info, *resp, resp_data[MAX_APS_PAYLOAD];
    zdo_req_t req;
    mem_ptr_t *nbor_mem_ptr;

    // parse the req
    zdo_parse_req(src_addr, data, clust, &req);

    // get the total number of entries and calculate the number of entries
    // we will send back to the requesting device. we can only send about three
    // entries at a time due to size restrictions on the aps payload size
    entry_cnt = nwk_neighbor_get_cnt();

    // we need to separate the response into two parts:
    // resp_info carries info. we will fill this out last
    // resp contains the actual neighbor list entries. we will fill this in first
    resp_info = resp_data;
    resp = resp_data + 5;   // the resp info is 5 bytes wide

    // start looping through all of the neighbor entries
    i = 0;
    for (nbor_mem_ptr = nwk_neighbor_tbl_get_head(); nbor_mem_ptr != NULL; nbor_mem_ptr = nbor_mem_ptr->next)
    {
        // restart loop until we've reached the start index
        if (i < req.type.nwk_lqi.start_index)
        {
            i++;
            continue;
        }

        // break if there is no more room for another nbor entry
        size = resp - resp_data;
        if ((size + ZDO_LQI_NBOR_ENTRY_SZ) > MAX_APS_PAYLOAD)
        {
            break;
        }

        // now fill out the list
        *(U64 *)resp = NBOR_ENTRY(nbor_mem_ptr)->ext_pan_id;
        resp += sizeof(U64);
        *(U64 *)resp = NBOR_ENTRY(nbor_mem_ptr)->ext_addr;
        resp += sizeof(U64);
        *(U16 *)resp = NBOR_ENTRY(nbor_mem_ptr)->nwk_addr;
        resp += sizeof(U16);

        // gen the bit-sliced field
        tmp =  (NBOR_ENTRY(nbor_mem_ptr)->device_type       & 0x3);
        tmp |= (NBOR_ENTRY(nbor_mem_ptr)->rx_on_when_idle   & 0x3) << 2;
        tmp |= (NBOR_ENTRY(nbor_mem_ptr)->relationship      & 0x7) << 4;
        *resp++ = tmp;

        *resp++ = NBOR_ENTRY(nbor_mem_ptr)->permit_join;
        *resp++ = NBOR_ENTRY(nbor_mem_ptr)->depth;
        *resp++ = NBOR_ENTRY(nbor_mem_ptr)->lqi;

        // increment the entry index. we'll use this to keep track of how many
        // entries in this frame.
        i++;
    }

    // fill out the info fields of the data here
    *resp_info++ = req.seq;
    *resp_info++ = AF_SUCCESS;                          // status
    *resp_info++ = entry_cnt;                           // total num neighbor entries
    *resp_info++ = req.type.nwk_lqi.start_index;        // starting index of entries
    *resp_info++ = i - req.type.nwk_lqi.start_index;    // num of entries in this list

    size = resp - resp_data;
    zdo_tx(resp_data, size, src_addr, NWK_LQI_RESP_CLUST, 0, af_handle_get());
}

/**************************************************************************/
/*!
        Handle the NWK leave request. When a device is requested to leave the network,
        it will clean up the tables and handle other preparations to exit the network.
        After that, it will send a response to the requesting device. However if the leave
        request is coming from a device that is also leaving, this device should not
        send a response because it is unknown if it will be received or not.
*/
/**************************************************************************/
void zdo_nwk_leave_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    U8 *resp, resp_data[2];
    zdo_req_t req;
    bool send_resp = true;
    mem_ptr_t *nbor_mem_ptr;
    address_t addr;

    zdo_parse_req(src_addr, data, clust, &req);

    // this needs to be added in case the device making the leave request is the one that will be leaving.
    // if that's the case, then no response can be sent to it so don't send a response.
    addr.mode = LONG_ADDR;
    addr.long_addr = req.type.leave.addr;
    if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&addr)) != NULL)
    {
        send_resp = (src_addr != NBOR_ENTRY(nbor_mem_ptr)->nwk_addr);
    }

    nwk_leave_req(req.type.leave.addr, req.type.leave.rem_children, req.type.leave.rejoin);

    resp = resp_data;
    *resp++ = zdo_seq_get();
    *resp++ = leave_req_status;

    if (send_resp)
    {
        zdo_tx(resp_data, 2, src_addr, NWK_LEAVE_RESP_CLUST, 0, af_handle_get());
    }
}

/**************************************************************************/
/*!
    Handle a NWK permit join request and generate the response. We just parse
    the permit join request and call the nwk permit join function with the request's
    'duration' field as an argument.
*/
/**************************************************************************/
void zdo_nwk_permit_join_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    U8 *resp, resp_data[2];
    zdo_req_t req;

    zdo_parse_req(src_addr, data, clust, &req);
    nwk_permit_join_req(req.type.permit_join.duration);

    resp    = resp_data;
    *resp++ = req.seq;
    *resp++ = AF_SUCCESS;
    zdo_tx(resp_data, 2, src_addr, NWK_PERMIT_JOIN_RESP_CLUST, 0, af_handle_get());
}

/**************************************************************************/
/*!
    The NWK update request handler will parse the NWK update request and
    issue the corresponding command. The NWK update request has three different
    functions:
    - Request a channel change (freq agility)
    - Request a change in the channel mask and nwk manager
    - Request an energy scan
*/
/**************************************************************************/
void zdo_nwk_update_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    U8 i, j, status, list_cnt, size, *resp_info, *resp, resp_data[MAX_APS_PAYLOAD];
    mac_pcb_t *pcb = mac_pcb_get();
    mac_pib_t *pib = mac_pib_get();
    zdo_req_t req;

    zdo_parse_req(src_addr, data, clust, &req);

    // set the response pointers
    resp_info = resp_data;
    resp = resp_data + 11;  // seq + the resp hdr = 11 bytes
    list_cnt = 0;

    // a broadcast transmission signifies that some event occurred on the network
    if (req.type.nwk_update.scan_duration == 0xFE)
    {
        // we were just informed to change the channel. scan the channel list
        // and find the bit that is set. there should only be one and that is the
        // channel of the new network.
        for (i = MAC_PHY_CHANNEL_OFFSET; i < (MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS); i++)
        {
            //lint -e{701} Info 701: Shift left of signed quantity (int)
            // shifting a '1' by i places
            if ((1<<i) & req.type.nwk_update.scan_channels)
            {
                break;
            }
        }
        // we've discovered the new channel and we will change to that channel
        mac_set_channel(i);
        return;
    }
    else if (req.type.nwk_update.scan_duration == 0xFF)
    {
        // we're given a new channel mask and new nwk manager address
        aps_aib_t *aib = aps_aib_get();
        nwk_nib_t *nib = nwk_nib_get();

        aib->chan_mask = req.type.nwk_update.scan_channels;
        nib->nwk_mngr = req.type.nwk_update.nwk_mngr_addr;
        return;
    }
    else if (req.type.nwk_update.scan_duration <= 0x5)
    {
        // save off the channel that we're using
        U8 channel = pib->curr_channel;

        // an energy scan was requested. we need to perform it for as many times as was
        // requested
        pcb->channel_mask = req.type.nwk_update.scan_channels;
        pcb->duration = req.type.nwk_update.scan_duration;

        // handle the energy scans. we will scan as many times as in the scan cnt field of the request
        for (i=0; i<req.type.nwk_update.scan_cnt; i++)
        {
            mac_scan_energy();

            // we need to parse the energy list and return the results of the energy scan
            for (j=0; j<MAC_MAX_CHANNELS; j++)
            {
                // go through the channel mask and return the results of the channels we scanned
                if ((1<<(j + MAC_PHY_CHANNEL_OFFSET)) & pcb->channel_mask)
                {
                    // don't add any more entries to the list if we're at the max size of the payload
                    if (list_cnt < (MAX_APS_PAYLOAD - 11))
                    {
                        // if we scanned the channel, then add the result in the energy list
                        *resp++ = pcb->energy_list[j];
                        list_cnt++;
                    }
                }
            }
        }
        mac_set_channel(channel);
        status = AF_SUCCESS;
    }
    else
    {
        status = AF_NOT_SUPPORTED;
    }

    *resp_info++ = req.seq;
    *resp_info++ = status;                   // status
    *(U32 *)resp_info = pcb->channel_mask;  // scanned channels
    resp_info += sizeof(U32);
    *(U16 *)resp_info = pcb->total_xmit;    // total transmissions
    resp_info += sizeof(U16);
    *(U16 *)resp_info = pcb->total_fail;    // total transmission failures
    resp_info += sizeof(U16);
    *resp_info++ = list_cnt;                // total values in energy list

    size = resp - resp_data;
    zdo_tx(resp_data, size, src_addr, NWK_UPDATE_RESP_CLUST, 0, af_handle_get());
}

/**************************************************************************/
/*!
        Handle the LQI response. This will need to be user defined.

        /todo Add to callback table
*/
/**************************************************************************/
void zdo_nwk_lqi_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    zdo_resp_t resp;
    zdo_parse_resp(data, clust, &resp);
}

/**************************************************************************/
/*!
        Handle the discovery response. This will need to be user defined.

        /todo Add to callback table
*/
/**************************************************************************/
void zdo_nwk_disc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    zdo_resp_t resp;
    zdo_parse_resp(data, clust, &resp);
}

/**************************************************************************/
/*!
        Handle the network update response. This will need to be user defined.

        /todo Add to callback table
*/
/**************************************************************************/
void zdo_nwk_update_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    zdo_resp_t resp;
    zdo_parse_resp(data, clust, &resp);
}

/**************************************************************************/
/*!
    Handle the network leave response. This will need to be user defined.

    /todo Add to callback table
*/
/**************************************************************************/
void zdo_nwk_leave_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
    zdo_resp_t resp;
    zdo_parse_resp(data, clust, &resp);
}
