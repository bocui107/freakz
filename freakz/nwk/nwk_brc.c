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
    \file nwk_brc.c
    \ingroup nwk
    \brief NWK broadcast handler

    This file handles the NWK layer broadcasts. Broadcasts are a fairly
    complicated affair in Zigbee and require a sequence of actions. It goes
    something like this:
    - A data request comes in from the APS layer with a broadcast address for
    a destination.
    - Start the broadcast sequence by setting flags to indicate that
    a broadcast is in progress and disallowing new broadcasts to start.
    - Send out the broadcast
    - Receive the relayed frames from the nodes that we sent the broadcast to. Since
    the broadcasts are forwarded, we should receive one broadcast from each node
    within listening range.
    - Each received broadcast goes into our broadcast table to record the fact
    that we received the broadcast from the node. This is called passive ack'ing
    in the Zigbee spec.
    - We're supposed to send out the broadcast a certain number of times, however
    the software will stop transmitting when we have received a broadcast from
    all of our neighbors.

    Broadcasts need to be treated with a lot of care in Zigbee because there is
    the potential for them to spiral out of control and crash all the nodes
    on the network.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the broadcast table. When a broadcast is sent, we receive
        return broadcasts containing the same information. The broadcast table
        implements a passive ack mechanism where we can check if all of our neighbors
        sent the broadcast back to us. If anyone missed it, we will then re-send
        the broadcast.

        It's actually kind of stupid. A broadcast is unreliable so I'm still not clear
        on why we need to implement this waste of space. But anyways, here it is.
*/
/**************************************************************************/
LIST(brc_list);

/**************************************************************************/
/*!
    Initialize the broadcast table. We will use this to implement our passive
    ack system.
*/
/**************************************************************************/
void nwk_brc_init()
{
    list_init(brc_list);
}

/**************************************************************************/
/*!
    Find a free broadcast entry, add it to the broadcast table, and return
    a pointer to it.
*/
/**************************************************************************/
static mem_ptr_t *nwk_brc_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(nwk_brc_t))) != NULL)
    {
        list_add(brc_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove a broadcast entry from the list and free it.
*/
/**************************************************************************/
static void nwk_brc_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(brc_list, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Remove all entries from the broadcast table.
*/
/**************************************************************************/
void nwk_brc_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(brc_list); mem_ptr != NULL; mem_ptr = list_chop(brc_list))
    {
        nwk_brc_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Search for the specified address and sequence id in the brc table. If found,
    then return true.
*/
/**************************************************************************/
static mem_ptr_t *nwk_brc_find(U16 addr, U8 seq)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(brc_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if ((BRC_ENTRY(mem_ptr)->nwk_addr == addr) && (BRC_ENTRY(mem_ptr)->seq_id == seq))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    After a broadcast is ended, we need to clean up the pcb and set the values
    to their idle state.
*/
/**************************************************************************/
static void nwk_brc_cleanup()
{
    nwk_pcb_t *pcb = nwk_pcb_get();

    pcb->brc_active     = false;
    pcb->brc_retries    = 0;
    pcb->brc_accept_new = true;
    pcb->brc_curr_frm   = NULL;
    nwk_brc_clear();
}

/**************************************************************************/
/*!
    Check if the brc is from a new sender. If its new, then add the sender to
    the brc table. This way, we can keep track of who received the broadcast
    and who didn't.
*/
/**************************************************************************/
void nwk_brc_add_new_sender(const U16 src_addr, const U8 seq_num)
{
    mem_ptr_t *mem_ptr;

    if (nwk_brc_find(src_addr, seq_num) == NULL)
    {
        // we haven't received a brc from this neighbor yet. add it to the brc table.
        if ((mem_ptr = nwk_brc_alloc()) != NULL)
        {
            BRC_ENTRY(mem_ptr)->nwk_addr   = src_addr;
            BRC_ENTRY(mem_ptr)->expiry     = (U8)ZIGBEE_BRC_EXPIRY;
            BRC_ENTRY(mem_ptr)->seq_id     = seq_num;
        }
    }
}

/**************************************************************************/
/*!
    Start a broadcast transmission. We need to check if one is currently in progress.
    If not, then set the flags to indicate that a broadcast is in progress,
    and setup the callback timer to expire the broadcast after the spec'd
    time interval.
*/
/**************************************************************************/
U8 nwk_brc_start(buffer_t *buf, nwk_hdr_t *hdr)
{
    nwk_pcb_t *pcb = nwk_pcb_get();
    nwk_nib_t *nib = nwk_nib_get();
    buffer_t *brc_curr_buf;
    U8 index;

    // if the brc is in the table, then drop it. otherwise, check for other error
    // conditions as well that would cause us to drop the broadcast.
    if (!pcb->brc_accept_new                        ||
        pcb->brc_active                             ||
        (hdr->radius == 0)                          ||
        !nwk_brc_check_dev_match(hdr->dest_addr))
    {
        buf_free(buf);
        return NWK_NOT_PERMITTED;
    }

    // set up the brc fields to indicate the brc status.
    BUF_ALLOC(brc_curr_buf, TX);

    // save off the original index of the alloc'd frame. we'll need to restore it after the memcpy.
    index = brc_curr_buf->index;
    memcpy(brc_curr_buf, buf, sizeof(buffer_t));
    brc_curr_buf->index = index;

    // move the buf data pointer to the correct position in the buffer and add the len field
    brc_curr_buf->dptr = brc_curr_buf->buf + (buf->dptr - buf->buf);

    pcb->brc_accept_new  = false;
    pcb->brc_active      = true;
    pcb->brc_seq         = hdr->seq_num;
    pcb->brc_curr_frm    = brc_curr_buf;
    hdr->src_addr        = nib->short_addr;
    memcpy(&pcb->brc_nwk_hdr, hdr, sizeof(nwk_hdr_t));

    ctimer_set(&pcb->brc_tmr, NWK_PASSIVE_ACK_TIMEOUT, nwk_brc_expire, NULL);
    return NWK_SUCCESS;
}

/**************************************************************************/
/*!
    Check if the broadcast address matches the device type. Return true
    if it matches. The reason we would need to check the device type is that
    Zigbee divides broadcasts up into three types. If we don't match the
    broadcast device type, we need to discard the frame.

    Here are the three device types:
    - Broadcast to all devices
    - Broadcast only to devices that have their receiver on when idle
    - Broadcast only to routers and the coordinator
*/
/**************************************************************************/
bool nwk_brc_check_dev_match(U16 dest_addr)
{
    bool resp = false;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    switch (dest_addr)
    {
    case NWK_BROADCAST_ALL:
        resp = true;
        break;
    case NWK_BROADCAST_RXONIDLE:
        resp = (pib->rx_on_when_idle) ? true : false;
        break;
    case NWK_BROADCAST_ROUTERS_COORD:
        resp = ((nib->dev_type == NWK_COORDINATOR) || (nib->dev_type == NWK_ROUTER)) ? true : false;
        break;
    default:
        break;
    }
    return resp;
}

/**************************************************************************/
/*!
    Check if all the neighbors have relayed the broadcast. We use this to know
    if we can end the broadcast early. If all neighbors have received the
    broadcast frame and relayed it, then there is no need to continue to send
    out broadcasts. Otherwise, we will just flood the network.

    This function uses the indirect memory pointers to the neighbor table entries.
*/
/**************************************************************************/
static bool nwk_brc_check_all_relayed(U16 addr, U8 seq_id)
{
    mem_ptr_t *mem_ptr;

    // we're going to scroll through the neighbor table and check to see if there
    // is a matching brc entry. If there is, then we're going to move on to the next
    // neighbor. If the entry comes up NULL, then that neighbor has not forwarded
    // the broadcast so we will return false.
    for (mem_ptr = nwk_neighbor_tbl_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // make sure the device type matches the broadcast type.
        if ((addr == NWK_BROADCAST_ALL) ||
           ((addr == NWK_BROADCAST_ROUTERS_COORD) && ((NBOR_ENTRY(mem_ptr)->device_type == NWK_ROUTER) || (NBOR_ENTRY(mem_ptr)->device_type == NWK_COORDINATOR))))
        {
            // check if the neighbor table entry exists in the broadcast table. if not, then return false.
            // all nodes have not responded yet.
            if (nwk_brc_find(NBOR_ENTRY(mem_ptr)->nwk_addr, seq_id) == NULL)
            {
                return false;
            }
        }
        else
        {
            // the device type doesn't match. get the next neighbor.
            continue;
        }
    }
    // looks like we're good.
    return true;
}

/**************************************************************************/
/*!
    This is used in the brc timer callback function. When the broadcast
    expires, this function is called and will check to see if all neighbors have
    relayed the broadcast. If not, then it will retry the broadcast.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'ptr' not referenced
//lint -e{818} Info 818: Pointer parameter ptr' could be declared as pointing to const
void nwk_brc_expire(void *ptr)
{
    bool all_relayed = false;
    buffer_t *buf;
    nwk_pcb_t *pcb = nwk_pcb_get();
    U8 index;

    pcb->brc_retries++;

    // check to see if we exceeded the retry count. if we did, then we end the
    // broadcast no matter what.
    if (pcb->brc_retries < NWK_MAX_BRC_RETRIES)
    {
        all_relayed = nwk_brc_check_all_relayed(pcb->brc_nwk_hdr.dest_addr, pcb->brc_seq);
        if (!all_relayed)
        {
            U8 len;

            // we need to do re-broadcast the frame since not all neighbors
            // forwarded it. First jitter the send according to the spec.
            busy_wait(drvr_get_rand() % NWK_BRC_JITTER);

            // copy the buffer, adjust the dptr of the new frame, and resend out the brc
            DBG_PRINT("NWK_BRC: Resending Broadcast. Retry #%02d.\n", pcb->brc_retries);
            BUF_ALLOC(buf, TX);

            // calculate the length of the frame contents
            len = aMaxPHYPacketSize - (pcb->brc_curr_frm->dptr - pcb->brc_curr_frm->buf);

            // save off the original index of the alloc'd frame. we'll need to restore it after the memcpy.
            index = buf->index;
            memcpy(buf, pcb->brc_curr_frm, sizeof(buffer_t));
            buf->index = index;

            // move the buf data pointer to the correct position in the buffer and add the len field
            buf->dptr = buf->buf + (pcb->brc_curr_frm->dptr - pcb->brc_curr_frm->buf);
            buf->len = len;

            nwk_fwd(buf, &pcb->brc_nwk_hdr);
            ctimer_set(&pcb->brc_tmr, NWK_PASSIVE_ACK_TIMEOUT, nwk_brc_expire, NULL);
            return;
        }
    }
    // The broadcast is finished. Either it overran its retries or all the neighbors
    // in the list have responded.
    buf_free(pcb->brc_curr_frm);
    nwk_brc_cleanup();
    DBG_PRINT("NWK_BRC: Broadcast is finished. %s.\n", all_relayed ? "All neighbors broadcasted" : "Broadcast Expired");
}

/**************************************************************************/
/*!
    This is a periodic function that checks for expiring entries. It is
    triggered by the slow clock and is called approximately once per second.
    When called, it will check the broadcast table and decrement the expiry
    in each of the entries. When the expiry reaches zero, the broadcast table
    entry will be freed.
*/
/**************************************************************************/
void nwk_brc_periodic()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(brc_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (BRC_ENTRY(mem_ptr)->expiry == 0)
        {
            nwk_brc_free(mem_ptr);
        }
        else
        {
            BRC_ENTRY(mem_ptr)->expiry--;
        }
    }
}

