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
    \defgroup aps APS - Application Sub-Layer
    \file aps.c
    \ingroup aps
    \brief Application Sublayer toplevel file

    This file implements the APS (Application Sub Layer) functionality.
    The APS handles the data communication (ACK'd and non-ACK'd) via
    the APS data service and interfaces to Application Framework (AF).
    The APS also handles binding and group table management (not implemented).
*/
/**************************************************************************/
#include "freakz.h"

static aps_aib_t aib;   ///< APS Info Block. Contains configuration params for the app layer.

/**************************************************************************/
/*!
    Init the APS layer. Set the APS info base to its default values, and
    start up the rest of the modules in the APS layer.
*/
/**************************************************************************/
void aps_init()
{
    // init the aib
    memset(&aib, 0, sizeof(aps_aib_t));
    aib.aps_ctr         =(U8) drvr_get_rand();
    aib.chan_mask       = aPhyChannelsSupported;
    aib.nonmemb_radius  = 2;
    aib.use_insec_join  = true;

    aps_retry_init();
    aps_dupe_init();
}

/**************************************************************************/
/*!
    Return a pointer to the APS Info Base structure.
*/
/**************************************************************************/
aps_aib_t *aps_aib_get()
{
    return &aib;
}

/**************************************************************************/
/*!
    Request to send data to the APS layer. This function is called by the
    Application Framework when an outgoing frame moves from the AF to the
    APS layer. The header structure is filled out based on the request
    arguments. If binding is used, then the address is also looked up and
    added here. Once the relevant header information has been extracted,
    this function will pass the frame and the header struct to the APS transmit
    function.
*/
/**************************************************************************/
U8 aps_data_req(const aps_data_req_t *req)
{
    aps_hdr_t hdr;
    mem_ptr_t *bnd_mem_ptr;
    U16 nwk_addr;
    U8 status = APS_SUCCESS;

    hdr.aps_frm_ctrl.frm_type   = APS_DATA_FRM;
    hdr.aps_frm_ctrl.ack_req    = (req->tx_opt & (1<<APS_TX_REQ_ACK_TX_OFF)) >> APS_TX_REQ_ACK_TX_OFF;
    hdr.aps_frm_ctrl.security   = req->tx_opt & (1<<APS_TX_SECURITY_ENB_OFF);
    hdr.aps_frm_ctrl.ack_format = false;
    hdr.aps_frm_ctrl.ext_hdr    = false;
    hdr.src_ep                  = req->src_ep;
    hdr.clust_id                = req->clust_id;
    hdr.prof_id                 = req->prof_id;
    hdr.radius                  = req->radius;
    hdr.aps_ctr                 = aib.aps_ctr++;
    hdr.disc_rte                = (req->tx_opt & (1<<APS_TX_RTE_DISC_DISABLE)) ? 0 : 1;
    hdr.handle                  = req->handle;

    switch (req->dest_addr_mode)
    {
    case APS_DEST_ADDR_EP_NONE:
        // loop through binding table. we need to send frame to each matching entry in table.
        for (bnd_mem_ptr = aps_bind_get_head(); bnd_mem_ptr != NULL; bnd_mem_ptr = bnd_mem_ptr->next)
        {
            // if bind entry matches the src endpoint and cluster, then it's a bind match and we need to send it to the target
            if ((BIND_ENTRY(bnd_mem_ptr)->src_ep == req->src_ep) && (BIND_ENTRY(bnd_mem_ptr)->clust == req->clust_id))
            {
                // the delivery mode will be unicast since we're sending a frame to each individual matching dest in
                // the binding table
                hdr.aps_frm_ctrl.delivery_mode = APS_UNICAST;
                hdr.dest_ep = BIND_ENTRY(bnd_mem_ptr)->dest_ep;

                // get the address from the binding entry. if it's a long address, then curse the stupid person who used a long
                // address and try and match it up with its corresponding short address
                if (BIND_ENTRY(bnd_mem_ptr)->dest_addr.mode == SHORT_ADDR)
                {
                    hdr.dest_addr = BIND_ENTRY(bnd_mem_ptr)->dest_addr.short_addr;
                }
                else
                {
                    if ((nwk_addr = nwk_addr_map_get_nwk_addr(BIND_ENTRY(bnd_mem_ptr)->dest_addr.long_addr)) == INVALID_NWK_ADDR)
                    {
                        // can't find the matching nwk address to the ieee address. abort for now.
                        status = APS_NO_SHORT_ADDRESS;
                        break;
                    }
                    hdr.dest_addr = nwk_addr;
                }
                aps_tx(req->buf, &hdr);
            }
        }
        break;

    case APS_GROUP_ADDR_PRESENT:
        hdr.aps_frm_ctrl.delivery_mode  = APS_GROUP;
        hdr.dest_addr                   = NWK_BROADCAST_RXONIDLE;
        hdr.grp_addr                    = req->dest_addr.short_addr;
        aps_tx(req->buf, &hdr);
        break;

    case APS_DEST_ADDR_16_EP_PRESENT:
        hdr.aps_frm_ctrl.delivery_mode  = APS_UNICAST;
        hdr.dest_addr                   = req->dest_addr.short_addr;
        hdr.dest_ep                     = req->dest_ep;
        aps_tx(req->buf, &hdr);
        break;

    case APS_DEST_ADDR_64_EP_PRESENT:
        // fall through
    default:
        status = APS_NOT_SUPPORTED;
        break;
    }
    return status;
}

/**************************************************************************/
/*!
    Generate the frame from the given header and send it to the nwk data service.
*/
/**************************************************************************/
void aps_tx(buffer_t *buf, aps_hdr_t *hdr)
{
    nwk_data_req_t req;

    // if ack request is set, then we need to add the hdr to the retry queue and set the
    // callback timer
    if (hdr->aps_frm_ctrl.ack_req)
    {
        buffer_t *retry_buf;
        U8 index;

        BUF_ALLOC(retry_buf, TX);
        index = retry_buf->index;
        memcpy(retry_buf, buf, sizeof(buffer_t));
        retry_buf->index = index;
        retry_buf->dptr = retry_buf->buf + (buf->dptr - buf->buf);
        aps_retry_add(retry_buf, hdr, hdr->handle);
    }

    // generate the aps header and prep the data request for the nwk layer
    aps_gen_header(buf, hdr);
    debug_dump_aps_hdr(hdr);

    req.buf                 = buf;
    req.dest_addr           = hdr->dest_addr;
    req.nsdu_handle         = hdr->handle;
    req.radius              = hdr->radius;
    req.disc_rte            = hdr->disc_rte;
    req.nwk_dest_addr_mode  = NWK_TX_UNICAST_BROADCAST; // no multicast support yet
    req.security_enb        = false;
    nwk_data_req(&req);
}

/**************************************************************************/
/*!
    This function is called by the NWK layer when incoming data is passed
    to the APS layer. Once the frame arrives, we parse the APS header
    and fill out the aps header struct.

    If it's an ACK frame, then we handle the ACK and then discard the frame.

    Otherwise, if the frame has an ack request on it, then we immediately
    send out an ACK. And finally, we send the frame up to the AF where it
    will be routed to the correct endpoint.
*/
/**************************************************************************/
void nwk_data_ind(buffer_t *buf, const nwk_hdr_t *nwk_hdr)
{
    aps_hdr_t hdr_in;
    aps_hdr_t hdr_out;
    buffer_t *buf_out;

    memset(&hdr_in, 0, sizeof(aps_hdr_t));
    hdr_in.dest_addr    = nwk_hdr->dest_addr;
    hdr_in.src_addr     = nwk_hdr->src_addr;
    hdr_in.radius       = nwk_hdr->radius;
    aps_parse_hdr(buf, &hdr_in);
    debug_dump_aps_hdr(&hdr_in);

    // check the frame to see if its in the duplicate rejection table. If it is, then
    // discard it.
    if (aps_dupe_reject(hdr_in.src_addr, hdr_in.aps_ctr))
    {
        buf_free(buf);
        return;
    }

    // process the frame based on the info in the header
    if (hdr_in.aps_frm_ctrl.frm_type == APS_ACK_FRM)
    {
        // If its an ACK frame, then send it to the handler. we will process the ack there
        // and send the data confirm.
        aps_retry_ack_handler(&hdr_in);
        buf_free(buf);
        return;
    }
    else if (hdr_in.aps_frm_ctrl.ack_req)
    {
        // the incoming frame has its ack request set. first generate the ack request and send it out.
        // then send the frame to the next higher layer.

        // copy the in hdr to the out hdr. we can reuse some of the fields.
        memset(&hdr_out, 0, sizeof(aps_hdr_t));
        memcpy(&hdr_out, &hdr_in, sizeof(aps_hdr_t));
        hdr_out.aps_frm_ctrl.frm_type       = APS_ACK_FRM;
        hdr_out.dest_addr                   = hdr_in.src_addr;
        hdr_out.disc_rte                    = true;
        hdr_out.radius                      = hdr_in.radius;

        if (!hdr_in.aps_frm_ctrl.ack_format)
        {
            // this is an ack to a data frame. need to set the dest/src ep and clust/profile id
            hdr_out.dest_ep     = hdr_in.src_ep;
            hdr_out.src_ep      = hdr_in.dest_ep;
            hdr_out.clust_id    = hdr_in.clust_id;
            hdr_out.prof_id     = hdr_in.prof_id;
        }

        // send out the ack first before forwarding the frame to the next higher layer.
        BUF_ALLOC(buf_out, TX);
        aps_tx(buf_out, &hdr_out);
    }

    // send it to the application framework rx function. It will get parsed and sent to the correct
    // endpoint from there.
    af_rx(buf, &hdr_in);
}

/**************************************************************************/
/*!
    This is the data confirm service called from the NWK layer. We are basically
    just forwarding it up since the confirmation will be handled at the application
    endpoint level.
*/
/**************************************************************************/
void nwk_data_conf(U8 status, U8 handle)
{
    // need to check if the data is reliable or unreliable. If it's a reliable transfer
    // we need to discard the confirm and wait for the APS ack from the destination.
    // Otherwise, we just route it up.
    if (aps_retry_handle_exists(handle))
    {
        // this means that its in our retry queue and its a reliable transfer. discard
        // the confirm from the MAC because we want to the ACK from the remote device's
        // APS layer.
        return;
    }

    // send it on up
    aps_conf(status, handle);
}


