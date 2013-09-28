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
    \defgroup nwk NWK - Network Layer
    \file nwk.c
    \ingroup nwk
    \brief Network Layer toplevel file

    This is the main file of the NWK layer. It handles the data transactions
    for both outgoing and incoming data. Outgoing data is pretty straightforward.
    The data request is called and the NWK header gets added to the data frame
    based on the request arguments.
    Incoming data is slightly more complicated. There are three cases of concern:

    1) Incoming data destined for this node.
    This is data whose destination address is this node's address. In that case,
    the data will get sent up to the next layer.

    2) Incoming data that is not destined for this node.
    In this case, the data will need to be forwarded. Based on the NWK header,
    tree routing or mesh routing will be used. If mesh routing is used, the
    destination will be looked up in the routing tables. If it doesn't exist,
    then a route discovery will be initiated.

    For tree routing, the frame will be forwarded according to its destination
    address.

    If a frame is going to be forwarded, then we take a shortcut and reuse the
    frame buffer. We basically take the rx frame buffer and send it into the
    outgoing frame data path. Also, we rebuild the NWK header and the MAC header
    (one layer below) but we keep the rest of the data intact. This way, it saves
    us from decoding the payload all the way at the top, then repackaging it up
    for transmission.

    3) Incoming data that is a broadcast.
    A broadcast frame will kick off the broadcast sequence of events. This means
    that the passive ACK system (spec'd in the Zigbee spec) will be started,
    the broadcast timers will be started, and we will forward the frame again as a
    broadcast.
*/
/**************************************************************************/
#include "freakz.h"

static nwk_pcb_t pcb;   ///< NWK layer protocol control block instantiation
static nwk_nib_t nib;   ///< NWK information base instantiation

/**************************************************************************/
/*!
    Init the NWK protocol control block, the NWK capability info, set the
    NWK info base to default values, and init the rest of the components
    of the NWK layer.
*/
/**************************************************************************/
void nwk_init()
{
    nwk_capab_info_t info;

    // init the pcb
    memset(&pcb, 0, sizeof(nwk_pcb_t));
    pcb.brc_accept_new = true;

    // generate the initial capability info for this device. this can be modified
    // later if needed when we join a network.
    info.dev_type               = ZIGBEE_DEVICE_TYPE;
    info.alloc_addr             = true;                 // always true for this version
    info.security               = false;                // no high security support
    info.pwr_src                = ZIGBEE_MAINS_POWERED;
    info.rx_on_idle             = ZIGBEE_RX_ON_WHEN_IDLE;

    // init the NIB
    memset(&nib, 0, sizeof(nwk_nib_t));
    nib.capability_info         = nwk_gen_capab_info(&info);
    nib.seq_num                 = (U8)drvr_get_rand();
    nib.rreq_id                 = (U8)drvr_get_rand();
    nib.report_const_cost       = true;
    nib.traxn_persist_time      = 10;
    nib.short_addr              = 0xFFFF;
    nib.stack_profile           = ZIGBEE_STACK_PROFILE;
    nib.max_routers             = ZIGBEE_MAX_ROUTERS;
    nib.max_children            = ZIGBEE_MAX_CHILDREN;
    nib.max_depth               = ZIGBEE_MAX_DEPTH;
    nib.rtr_cap                 = nib.max_routers;
    nib.ed_cap                  = nib.max_children - nib.max_routers;
    nib.dev_type                = ZIGBEE_DEVICE_TYPE;
    nib.joined                  = false;

    nwk_rte_mesh_init();
    nwk_rte_tree_init();
    nwk_neighbor_tbl_init();
    nwk_rte_disc_tbl_init();
    nwk_rte_tbl_init();
    nwk_pend_init();
    nwk_brc_init();
    nwk_addr_map_init();
}

/**************************************************************************/
/*!
    Return a pointer to the NWK protocol control block structure.
*/
/**************************************************************************/
nwk_pcb_t *nwk_pcb_get()
{
    return &pcb;
}

/**************************************************************************/
/*!
    Return a pointer to the NWK information base structure.
*/
/**************************************************************************/
nwk_nib_t *nwk_nib_get()
{
    return &nib;
}

/**************************************************************************/
/*!
    This is the nwk data request service that receives an APS frame
    and slaps on a NWK header. If the frame is meant to be broadcasted,
    it will also startoff the broadcast procedure. Currently, it will send
    the frame to the NWK forward function in case that it needs routing.
*/
/**************************************************************************/
void nwk_data_req(const nwk_data_req_t *req)
{
    nwk_hdr_t nwk_hdr;

    // Fill out and generate the nwk header
    memset(&nwk_hdr, 0, sizeof(nwk_hdr_t));
    nwk_hdr.nwk_frm_ctrl.frame_type     = NWK_DATA_FRM;
    nwk_hdr.nwk_frm_ctrl.disc_route     = req->disc_rte;
    nwk_hdr.nwk_frm_ctrl.protocol_ver   = ZIGBEE_PROTOCOL_VERSION;
    nwk_hdr.seq_num                     = nib.seq_num++;
    nwk_hdr.dest_addr                   = req->dest_addr;
    nwk_hdr.src_addr                    = nib.short_addr;
    nwk_hdr.radius                      = req->radius;
    nwk_hdr.handle                      = req->nsdu_handle;

    // if this is a brc frame that we're originating, we need to add it to the brc table.
    if ((nwk_hdr.dest_addr & NWK_BROADCAST_MASK) == 0xFFF0)
    {
        // set up the brc parameter, then start the brc transmission procedure
        nwk_hdr.nwk_frm_ctrl.disc_route = false;
        if (nwk_brc_start(req->buf, &nwk_hdr) != NWK_SUCCESS)
        {
            return;
        }
    }

    // send it to the fwd function for processing and possible routing
    nwk_fwd(req->buf, &nwk_hdr);
}

/**************************************************************************/
/*!
    Send out the NWK frame to the MAC layer for transmission. This function
    does final assembly of the NWK header and generates the MAC request
    based on the given information.
*/
/**************************************************************************/
static void nwk_tx(buffer_t *buf, nwk_hdr_t *hdr, bool indirect)
{
    mac_pib_t *pib = mac_pib_get();
    mac_data_req_t req;

    // finish filling out the nwk hdr and generate it
    hdr->nwk_frm_ctrl.protocol_ver          = ZIGBEE_PROTOCOL_VERSION;
    hdr->nwk_frm_ctrl.mcast_flag            = false;        // multicast not supported
    hdr->nwk_frm_ctrl.security              = false;
    hdr->nwk_frm_ctrl.src_rte               = false;
    hdr->nwk_frm_ctrl.dest_ieee_addr_flag   = false;
    hdr->nwk_frm_ctrl.src_ieee_addr_flag    = false;

    nwk_gen_header(buf, hdr);
    debug_dump_nwk_hdr(hdr);

    req.src_addr.mode                       = SHORT_ADDR;
    req.dest_addr.mode                      = SHORT_ADDR;
    req.src_addr.short_addr                 = hdr->mac_hdr->src_addr.short_addr;
    req.dest_addr.short_addr                = hdr->mac_hdr->dest_addr.short_addr;
    req.src_pan_id                          = pib->pan_id;
    req.dest_pan_id                         = pib->pan_id;
    req.msdu_handle                         = hdr->handle;
    req.buf                                 = buf;

    // it its a broadcast, then no ack request. otherwise, ack requests on all other transfers.
    req.tx_options = (req.dest_addr.short_addr != 0xFFFF) ? MAC_ACK_REQUEST : 0x0;
    req.tx_options |= indirect ? MAC_INDIRECT_TRANS : 0x0;

    // kick it to the curb!
    mac_data_req(&req);
}

/**************************************************************************/
/*!
    This function is the work horse of the NWK layer. It takes a frame from
    either the data request or the data indication functions (if the dest address
    is not for this node). It will look at the dest address and determine
    how this frame should be transmitted. The forwarding choices are:
    1) broadcast
    2) neighbor table   (the dest is a neighbor of ours)
    3) routing table    (the route path exists)
    4) route discovery  (the route path does not exist but mesh routing is okay)
    5) tree             (none of the above and the route discovery flag is disabled)
*/
/**************************************************************************/
void nwk_fwd(buffer_t *buf, nwk_hdr_t *hdr_in)
{
    U16 next_hop;
    nwk_hdr_t hdr_out;
    mac_hdr_t mac_hdr_out;
    bool indirect = false;
    address_t dest_addr;
    mem_ptr_t *nbor_mem_ptr;

    hdr_out.mac_hdr         = &mac_hdr_out;
    dest_addr.mode          = SHORT_ADDR;
    dest_addr.short_addr    = hdr_in->dest_addr;

    // is it a broadcast?
    if ((hdr_in->dest_addr & NWK_BROADCAST_MASK) == 0xFFF0)
    {
        // the buffer data ptr (dptr) is pointing to the nwk payload now. calc the length of the nwk
        // payload. then we can re-use the buffer, slap on the nwk hdr, and send it back down
        // the stack.
        //
        //lint -e{734} Info 734: Loss of precision (assignment) (31 bits to 8 bits)
        // its okay since we're subtracting a value from a constant
        buf->len = aMaxPHYPacketSize - (buf->dptr - buf->buf);

        // need to change the mac dest address to broadcast in case it isn't. Also change the src
        // addr to the address of this device.
        hdr_out.mac_hdr->dest_addr.short_addr       = MAC_BROADCAST_ADDR;
        hdr_out.src_addr                            = hdr_in->src_addr;
    }
    // is the dest in our neighbor table?
    else if ((nbor_mem_ptr = nwk_neighbor_tbl_get_entry(&dest_addr)) != NULL)
    {

        // check to see if the dest address is in our neighbor table
        if (hdr_in->src_addr != nib.short_addr)
        {
            //lint -e{734} Info 734: Loss of precision (assignment) (31 bits to 8 bits)
            // its okay since we're subtracting a value from a constant
            buf->len = aMaxPHYPacketSize - (buf->dptr - buf->buf);
        }

        // if the neighbor does not keep its rx on when idle (ie: sleeping), then we need to send
        // the frame indirectly.
        if (!NBOR_ENTRY(nbor_mem_ptr)->rx_on_when_idle)
        {
            indirect = true;
        }

        // set the mac dest addr to the neighbor address
        hdr_out.mac_hdr->dest_addr.short_addr       = hdr_in->dest_addr;
        hdr_out.src_addr                            = hdr_in->src_addr;
    }
    // is the dest in our routing table?
    else if ((next_hop = nwk_rte_tbl_get_next_hop(hdr_in->dest_addr)) != INVALID_NWK_ADDR)
    {
        // it's in the routing table. forward it.
        // the buffer dptr is pointing to the nwk payload now. calc the length of the nwk
        // payload. then we can re-use the buffer, slap on the nwk hdr, and send it back down
        // the stack.
        //
        //lint -e{734} Info 734: Loss of precision (assignment) (31 bits to 8 bits)
        // its okay since we're subtracting a value from a constant
        buf->len = aMaxPHYPacketSize - (buf->dptr - buf->buf);

        // set the mac dest addr to the next hop and the nwk src addr to the originator
        hdr_out.mac_hdr->dest_addr.short_addr       = next_hop;
        hdr_out.src_addr                            = hdr_in->src_addr;
    }
    // are we allowed to discover the route?
    else if (hdr_in->nwk_frm_ctrl.disc_route)
    {
        // the destination isn't in our routing tables. discover route is enabled for the
        // frame so we can buffer it and initiate route discovery.
        nwk_pend_add_new(buf, hdr_in);
        nwk_rte_mesh_disc_start(hdr_in->dest_addr);
        return;
    }
    // if all else fails, route it along the tree
    else
    {
        // tree routing is the default routing if everything else fails or we don't allow mesh routing.
        // we'll just recycle the frame buffer and send it on its way out.
        if ((next_hop = nwk_rte_tree_calc_next_hop(hdr_in->dest_addr)) == INVALID_NWK_ADDR)
        {
            // tree routing failed for some reason. collect the stat and go on with life.
            pcb.failed_tree_rte++;
            buf_free(buf);
            DBG_PRINT("NWK: No such neighbor exists.\n");
            return;
        }
        // the buffer dptr is pointing to the nwk payload now. calc the length of the nwk
        // payload. then we can re-use the buffer, slap on the nwk hdr, and send it back down
        // the stack.
        //
        //lint -e{734} Info 734: Loss of precision (assignment) (31 bits to 8 bits)
        // its okay since we're subtracting a value from a constant
        buf->len = aMaxPHYPacketSize - (buf->dptr - buf->buf);

        // set the mac dest addr to the spec'd neighbor addr and the nwk src addr to the originator
        hdr_out.mac_hdr->dest_addr.short_addr   = next_hop;
        hdr_out.src_addr                        = hdr_in->src_addr;
    }

    // fill out the rest of the fields that will be needed to forward this frame
    hdr_out.nwk_frm_ctrl.frame_type             = hdr_in->nwk_frm_ctrl.frame_type;
    hdr_out.seq_num                             = hdr_in->seq_num;
    hdr_out.nwk_frm_ctrl.disc_route             = hdr_in->nwk_frm_ctrl.disc_route;;
    hdr_out.radius                              = hdr_in->radius;
    hdr_out.dest_addr                           = hdr_in->dest_addr;
    hdr_out.handle                              = hdr_in->handle;
    hdr_out.mac_hdr->src_addr.mode              = SHORT_ADDR;
    hdr_out.mac_hdr->dest_addr.mode             = SHORT_ADDR;
    hdr_out.mac_hdr->src_addr.short_addr        = nib.short_addr;
    nwk_tx(buf, &hdr_out, indirect);
}

/**************************************************************************/
/*!
    This function handles the incoming rx data to the NWK layer. There are only
    two types of frames, command and data. A command frame will be parsed and
    routed to the appropriate command function handler. A data frame will be
    checked to see if this device is its destination. If so, then it will
    be sent to the next higher layer. If not, then it will be sent to the
    nwk_fwd function to be forwarded to the next device.
*/
/**************************************************************************/
void mac_data_ind(buffer_t *buf, mac_hdr_t *mac_hdr)
{
    nwk_hdr_t hdr;
    buffer_t *buf_in;
    nwk_cmd_t cmd;
    U8 index;

    // assign incoming mac hdr to the data struct, then parse the frame for the nwk hdr
    hdr.mac_hdr = mac_hdr;
    nwk_parse_hdr(buf, &hdr);
    debug_dump_nwk_hdr(&hdr);

    // if it's a broadcast and the device type doesn't match our device, then discard it.
    if ((hdr.dest_addr & NWK_BROADCAST_MASK) == NWK_BROADCAST_MASK)
    {
        if (!nwk_brc_check_dev_match(hdr.dest_addr))
        {
            buf_free(buf);
            return;
        }
    }

    switch(hdr.nwk_frm_ctrl.frame_type)
    {
    case NWK_DATA_FRM:
        // How we handle this depends on the destination address
        if (hdr.dest_addr == nib.short_addr)
        {
            // we're the dest. send it up for further processing.
            nwk_data_ind(buf, &hdr);

            // check for the radius here. we can't let a frame with a 0 radius get sent out again.
            // if the radius is zero, then drop the frame.
            if (hdr.radius == 0)
            {
                buf_free(buf);
                return;
            }

            // decrement the radius
            hdr.radius--;
            break;
        }
        else if ((hdr.dest_addr & NWK_BROADCAST_MASK) == 0xFFF0)
        {
            // make copy of brc frame, adjust the data pointer, and then send it up
            BUF_ALLOC(buf_in, RX);
            index = buf_in->index;
            memcpy(buf_in, buf, sizeof(buffer_t));
            buf_in->index = index;
            buf_in->dptr = buf_in->buf + (buf->dptr - buf->buf);
            nwk_data_ind(buf_in, &hdr);

            // check for the radius here. we can't let a frame with a 0 radius get sent out again.
            // if the radius is zero, then drop the frame.
            if (hdr.radius == 0)
            {
                buf_free(buf);
                return;
            }

            // decrement the radius
            hdr.radius--;

            // check to see if the sender is in our brc table. if not, then add the senders addr.
            nwk_brc_add_new_sender(hdr.src_addr, hdr.seq_num);

            // now check if its a new brc. if its not, then just discard the frame.
            if (hdr.seq_num == pcb.brc_seq)
            {
                buf_free(buf);
                return;
            }

            // looks like its a new brc. initiate the brc params and start the brc transmission procedure.
            // note: the header src address gets changed inside the brc_start function.
            if (nwk_brc_start(buf, &hdr) != NWK_SUCCESS)
            {
                return;
            }
        }

        // we're not the destination. forward it to the dest address.
        nwk_fwd(buf, &hdr);
        break;

    case NWK_CMD_FRM:
        // Parse command frame and then send to appropriate handler
        nwk_parse_cmd(buf, &cmd);
        debug_dump_nwk_cmd(&cmd);

        // Free the buffer since we're finished with it now. Then handle the command.
        buf_free(buf);

        // command frame handler
        switch (cmd.cmd_frm_id)
        {
        case NWK_CMD_RTE_REQ:
            nwk_rte_mesh_rreq_handler(&hdr, &cmd);
            break;

        case NWK_CMD_RTE_REP:
            nwk_rte_mesh_rrep_handler(&hdr, &cmd);
            break;

        case NWK_CMD_LEAVE:
            nwk_leave_handler(&hdr, &cmd);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

/**************************************************************************/
/*!
    Handle the data confirmation from the MAC layer and sends it to the
    application layer.
*/
/**************************************************************************/
void mac_data_conf(U8 status, U8 handle)
{
    nwk_data_conf(status, handle);
}

/**************************************************************************/
/*!
    Handles the data poll confirmation.
*/
/**************************************************************************/
//lint -e{715} Symbol 'status' not referenced
// its an empty function for now
void mac_poll_conf(U8 status)
{
}

/**************************************************************************/
/*!
    Handles the comm status indication. The comm status indication usually
    goes out when an indirect frame is transmitted or times out.
*/
/**************************************************************************/
//lint -e{715} Symbol 'dest_addr' not referenced
void mac_comm_status_ind(U16 pan_id, address_t dest_addr, U8 status, U8 handle)
{
    nwk_data_conf(status, handle);
}
