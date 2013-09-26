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
    \file aps_retry.c
    \ingroup aps
    \brief APS retry mechanism

    Implement a retry queue to handle the APS retries on outgoing
    frames that have their ACK request flag set. The retry queue is just
    for reliable transfers that require an APS level ACK. The frame is
    buffered in the retry queue and if an ACK does not come back within
    the specified time, the frame will be re-sent from the retry queue.

    \note The APS retry queue will be removed later and the application
    will perform the retry. Since we are at such a high level, there is
    not much savings to buffer the frame here, versus rebuilding it. It
    would be more efficient to save the RAM that the APS retry queue takes
    up and just have the endpoint profiles handle the retry. It is actually
    easier since most application implementations won't increment
    the app's data pointer until a SUCCESS confirmation is received. Hence
    if no SUCCESS confirmation is received, that would be the signal
    to re-send the data.

    \todo Remove APS retry table
*/
/**************************************************************************/
#include "freakz.h"
LIST(aps_retry);                ///< List head for the APS retry list

/**************************************************************************/
/*!
    Init the retry list.
*/
/**************************************************************************/
void aps_retry_init()
{
    list_init(aps_retry);
}

/**************************************************************************/
/*!
    Find a free retry entry and add it to the retry queue. Also, init the
    parameters in the structure.
*/
/**************************************************************************/
static mem_ptr_t *aps_retry_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(aps_retry_t))) != NULL)
    {
        APS_RETRY_ENTRY(mem_ptr)->retries = APS_MAX_FRAME_RETRIES;
        APS_RETRY_ENTRY(mem_ptr)->expiry = APS_ACK_WAIT_DURATION;
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the entry from the retry queue and free it.
*/
/**************************************************************************/
static void aps_retry_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        buf_free(APS_RETRY_ENTRY(mem_ptr)->buf);
        list_remove(aps_retry, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find the aps retry entry with the specified handle. If found, return
    the memory pointer for the entry.
*/
/**************************************************************************/
static mem_ptr_t *aps_retry_find(U8 handle)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(aps_retry); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (APS_RETRY_ENTRY(mem_ptr)->handle == handle)
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add an entry to the aps retry list and fill out the entry.
*/
/**************************************************************************/
void aps_retry_add(buffer_t *buf, aps_hdr_t *hdr, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_retry_alloc()) != NULL)
    {
        APS_RETRY_ENTRY(mem_ptr)->buf = buf;
        APS_RETRY_ENTRY(mem_ptr)->handle = handle;
        memcpy(&APS_RETRY_ENTRY(mem_ptr)->hdr, hdr, sizeof(aps_hdr_t));
    }
}

/**************************************************************************/
/*!
    Check the incoming ACK header for a match in the queue. A match is indexed
    by the APS counter value, cluster ID, and the source endpoint. If a match
    is found, remove that entry from the retry queue and send a data confirm.
*/
/**************************************************************************/
void aps_retry_ack_handler(const aps_hdr_t *hdr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(aps_retry); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if ((APS_RETRY_ENTRY(mem_ptr)->hdr.aps_ctr     == hdr->aps_ctr)    &&
            (APS_RETRY_ENTRY(mem_ptr)->hdr.clust_id    == hdr->clust_id)   &&
            (APS_RETRY_ENTRY(mem_ptr)->hdr.dest_ep     == hdr->src_ep))
        {
            aps_retry_free(mem_ptr);
            aps_conf(APS_SUCCESS, APS_RETRY_ENTRY(mem_ptr)->handle);
        }
    }
}

/**************************************************************************/
/*!
    Check if the handle exists in the aps retry list.
*/
/**************************************************************************/
bool aps_retry_handle_exists(U8 handle)
{
    mem_ptr_t *mem_ptr;

    mem_ptr = aps_retry_find(handle);
    return (mem_ptr) ? true : false;
}

/**************************************************************************/
/*!
    Check a retry entry that has expired.  If the ACK duration has expired
    with no incoming ACK. We need to either attempt a retry or retire the frame.
*/
/**************************************************************************/
void aps_retry_expire(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        APS_RETRY_ENTRY(mem_ptr)->retries--;
        if (APS_RETRY_ENTRY(mem_ptr)->retries > 0)
        {
            APS_RETRY_ENTRY(mem_ptr)->expiry = APS_ACK_WAIT_DURATION;
            aps_tx(APS_RETRY_ENTRY(mem_ptr)->buf, &APS_RETRY_ENTRY(mem_ptr)->hdr);
        }
        else
        {
            // we've used up all the retries. retire the entry.
            aps_conf(APS_NO_ACK, APS_RETRY_ENTRY(mem_ptr)->handle);
            aps_retry_free(mem_ptr);
        }
    }
}

/**************************************************************************/
/*!
    This is a periodic function that gets called approximately once per second
    by the slow clock. Its only purpose is to check for expired retry entries
    and to decrement the expiry for each of the entries.
*/
/**************************************************************************/
void aps_retry_periodic()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(aps_retry); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // go through the retry list and check for expired entries. if any
        // are expired, then process them. otherwise, decrement the expiry.
        if (APS_RETRY_ENTRY(mem_ptr)->expiry == 0)
        {
            aps_retry_expire(mem_ptr);
        }
        else
        {
            APS_RETRY_ENTRY(mem_ptr)->expiry--;
        }
    }
}
