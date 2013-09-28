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
    \defgroup af AF - Application Framework
    \file af.c
    \ingroup af

    \brief Application Framework Toplevel File
 
    This file contains functions to implement the Application Framework.
    To summarize what the application framework is, it basically is a
    container for all of the endpoints of the Zigbee device.

    The AF basically consists of a list of endpoints. When a profile is
    added to the device, it must register its endpoint with the AF. The
    registration consists of registering the rx and confirm callbacks,
    as well as the endpoint number and the simple descriptor. After
    registration, the AF will be able to route incoming messages to the
    registered endpoint.

    The AF implements an RX, TX, and confirm queues which serves as
    temporary holding areas for the data and confirmations. Although
    it may be quicker to send the data directly up to the endpoint or
    down to the APS, the latency moving up and down the stack and application
    files could potentially be very long. Hence we can break up the stack
    latency by queuing the tx, rx, and confirm data and having the process
    handle them when there's time.
*/
/**************************************************************************/
#include "freakz.h"

/*********************** Process Declarations **********************/
PROCESS(af_process, "AF Process"); ///< Declaration for the AF process
/*******************************************************************/

// events used in this ep profile
extern process_event_t event_af_tx;
extern process_event_t event_af_rx;
extern process_event_t event_af_conf;

static U8 af_handle;

/**************************************************************************/
/*!
    Init the Application Framework layer. This consists of starting the AF
    process, allocating the AF events, and then starting up the rest of
    the modules in the AF layer.
*/
/**************************************************************************/
void af_init()
{
    event_af_tx     = process_alloc_event();
    event_af_rx     = process_alloc_event();
    event_af_conf   = process_alloc_event();

    process_start(&af_process, NULL);

    af_conf_init();
    af_conf_tbl_init();
    af_ep_init();
    af_rx_init();
    af_tx_init();

    af_handle = drvr_get_rand();
}

/**************************************************************************/
/*!
    Return a handle that can be used in the application layer as a single
    point of reference for all the frames that pass through.
*/
/**************************************************************************/
U8 af_handle_get()
{
    return af_handle++;
}

/**************************************************************************/
/*!
    This is the entry point to the AF's RX data path. This function will be
    called by the APS and the buffer and header will be send to the AF's
    rx queue. An event will be posted, and the AF process will handle
    the event at a later time.
*/
/**************************************************************************/
void af_rx(buffer_t *buf, aps_hdr_t *hdr)
{
    af_rx_add(buf, hdr);
    process_post(&af_process, event_af_rx, NULL);
}

/**************************************************************************/
/*!
    This is the exit point of the AF's RX data path. Here, we grab the frame from
    the AF RX queue, figure out where we need to route it, and then send it to
    the destination endpoint via the endpoint's registered RX callback function.
*/
/**************************************************************************/
void af_rx_handler()
{
    U8 len;
    mem_ptr_t *rx_mem_ptr, *grp_mem_ptr;

    if ((rx_mem_ptr = af_rx_pop()) != NULL)
    {
        // Calculate the length of the payload so we can send it to the endpoint's rx handler
        //lint -e{734} Info 734: Loss of precision (31 bits to 8 bits)
        // doing pointer arithmetic. It won't overflow.
        len = aMaxPHYPacketSize - (RX_ENTRY(rx_mem_ptr)->buf->dptr - RX_ENTRY(rx_mem_ptr)->buf->buf);

        // If we're in group mode, then use the group ID and scan the group table for matches. if a match is found
        // then send the frame to that endpoint for processing. continue until the end of the group table.
        if (RX_ENTRY(rx_mem_ptr)->grp_mode)
        {
            for (grp_mem_ptr = aps_grp_get_head(); grp_mem_ptr != NULL; grp_mem_ptr = grp_mem_ptr->next)
            {
                // we need to get the mem pointer for the group id entry first. then we can figure out the group ID
                if (GROUP_ENTRY(grp_mem_ptr)->id == RX_ENTRY(rx_mem_ptr)->grp_id)
                {
                    af_ep_rx(GROUP_ENTRY(grp_mem_ptr)->ep, RX_ENTRY(rx_mem_ptr)->buf, len, RX_ENTRY(rx_mem_ptr)->src_addr, 
                             RX_ENTRY(rx_mem_ptr)->src_ep, RX_ENTRY(rx_mem_ptr)->clust_id);
                }
            }
        }
        else
        {
            // we're not in group mode so just send the payload to the destination endpoint
            af_ep_rx(RX_ENTRY(rx_mem_ptr)->dest_ep, RX_ENTRY(rx_mem_ptr)->buf, len, RX_ENTRY(rx_mem_ptr)->src_addr, 
                     RX_ENTRY(rx_mem_ptr)->src_ep, RX_ENTRY(rx_mem_ptr)->clust_id);
        }

        // free the rx buffer and the memory. we're done here.
        af_rx_free(rx_mem_ptr);
    }
}

/**************************************************************************/
/*!
    This is the entry point to the AF's confirm path. The confirmation will
    be held in the confirm queue and a confirmation event gets posted to
    the AF process to be handled later.
*/
/**************************************************************************/
void aps_conf(U8 status, U8 handle)
{
    af_conf_add(status, handle);
    process_post(&af_process, event_af_conf, NULL);
}

/**************************************************************************/
/*!
    This is the exit point to the AF's confirm path. The confirmation is
    pulled from the confirm queue, the endpoint is looked up, and the
    confirmation is sent to the registered endpoint's confirm callback.
*/
/**************************************************************************/
void af_conf_handler()
{
    mem_ptr_t *conf_mem_ptr;
    mem_ptr_t *conf_tbl_mem_ptr;

    // get the next handle in the confirmation queue
    if ((conf_mem_ptr = af_conf_pop()) != NULL)
    {
        if ((conf_tbl_mem_ptr = af_conf_tbl_find(CONF_ENTRY(conf_mem_ptr)->handle)) != NULL)
        {
            af_ep_conf(CONF_TBL_ENTRY(conf_tbl_mem_ptr)->src_ep, CONF_ENTRY(conf_mem_ptr)->status, CONF_ENTRY(conf_mem_ptr)->handle);
            af_conf_tbl_free(conf_tbl_mem_ptr);
        }
        af_conf_free(conf_mem_ptr);
    }
}

/**************************************************************************/
/*!
    This is the entry point into the AF's TX data path. A buffer is allocated
    and the payload is moved into the buffer. The buffer is added to the AF's
    TX data queue along with the other arguments. The other arguments will
    be used later to make APS request. Once everything is stored in the queue,
    then an event is posted to the AF process for later handling.

    If there is not a minimum amount of buffers available, the function will fail.
    This is because we can't tell when incoming frames will arrive and if we use up
    all the buffers, they can't get processed. This may change in the future as I
    optimize the buffer usage.
*/
/**************************************************************************/
void af_tx(U8 *data, U8 len, U8 src_ep, U16 dest_addr, U8 dest_ep, U16 clust, U16 prof_id, U8 mode, U8 tx_opt, U8 radius, U8 handle)
{
    U8 cnt;
    buffer_t *buf;

    // if its a broadcast, then don't add it to the confirm table
    if ((dest_addr & NWK_BROADCAST_MASK) != NWK_BROADCAST_MASK)
    {
        // add the confirm to the table first so that we can track when a confirm arrives and route it to the proper ep
        af_conf_tbl_add(src_ep, handle);
    }

    // if we don't have enough buffers, then abort the tx and send a confirm
    cnt = buf_get_cnt();
    if (cnt < (MAX_BUF_POOL_SIZE - ZIGBEE_MIN_BUFS_NEEDED))
    {
        BUF_ALLOC(buf, TX);
        buf->dptr -= len;
        buf->len += len;
        memcpy(buf->dptr, data, len);

        af_tx_add(buf, src_ep, dest_addr, dest_ep, clust, prof_id, mode, tx_opt, radius, handle);
        process_post(&af_process, event_af_tx, NULL);
    }
    else
    {
        aps_conf(AF_NO_FREE_BUFS, handle);
    }
}

/**************************************************************************/
/*!
    This is the exit point of the AF's TX data path. The buffer and info
    are pulled from the AF TX data queue. An APS request is generated
    and the data is sent out to the APS layer.
*/
/**************************************************************************/
void af_tx_handler()
{
    mem_ptr_t *mem_ptr;
    aps_data_req_t req;

    if ((mem_ptr = af_tx_pop()) != NULL)
    {
        // generate the aps data request
        req.dest_addr_mode          = TX_ENTRY(mem_ptr)->mode; 

        // switch up the addressing based on the address mode
        switch (req.dest_addr_mode)
        {
        case APS_DEST_ADDR_EP_NONE:
            // no address and ep info for this. this all comes from the binding table
            break;

        case APS_GROUP_ADDR_PRESENT:
            // this is gonna be a group frame. the group id goes into the dest addr 
            // and we just add a dummy endpoint here. the ep will be stripped from
            // the frame in the APS when we build the aps header
            req.dest_addr.mode          = SHORT_ADDR;
            req.dest_addr.short_addr    = TX_ENTRY(mem_ptr)->dest_addr;
            req.dest_ep                 = 0;
            break;

        case APS_DEST_ADDR_16_EP_PRESENT:
            // standard frame with short address 
            req.dest_addr.mode          = SHORT_ADDR;
            req.dest_addr.short_addr    = TX_ENTRY(mem_ptr)->dest_addr;
            req.dest_ep                 = TX_ENTRY(mem_ptr)->dest_ep;
            break;

        case APS_DEST_ADDR_64_EP_PRESENT:
            // not allowed. we are only using the network address for now.
            return;
        }
        req.prof_id                 = TX_ENTRY(mem_ptr)->prof_id;
        req.clust_id                = TX_ENTRY(mem_ptr)->clust;
        req.src_ep                  = TX_ENTRY(mem_ptr)->src_ep;
        req.asdu_len                = TX_ENTRY(mem_ptr)->buf->len;
        req.buf                     = TX_ENTRY(mem_ptr)->buf;
        req.handle                  = TX_ENTRY(mem_ptr)->handle;
        req.tx_opt                  = TX_ENTRY(mem_ptr)->tx_opt;
        req.radius                  = TX_ENTRY(mem_ptr)->radius;
        aps_data_req(&req);
        af_tx_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    This is the main process that handles the different AF events.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'data' (line 257) not referenced
//lint -e{818} Pointer parameter 'data' could be declared as pointing to const
PROCESS_THREAD(af_process, ev, data)
{
    PROCESS_BEGIN();

    //lint -e{716} Info 716: while(1) ...
    // looping forever on purpose
    while (1)
    {
        PROCESS_WAIT_EVENT();
        if (ev == event_af_rx)
        {
            af_rx_handler();
        }
        else if (ev == event_af_tx)
        {
            af_tx_handler();
        }
        else if (ev == event_af_conf)
        {
            af_conf_handler();
        }
    }
    PROCESS_END();
}
