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
    \file mac_retry.c
    \ingroup mac
    \brief MAC retry handler

    This file implements the MAC retry queue. When an outgoing frame has the
    ack request bit set, then an ack is required for that frame. If no ack is
    returned within the specified time limit, then the frame must be re-sent.
    Instead of re-sending from the top application level, where we need to rebuild
    the frame, we buffer the frame in the MAC's retry queue and wait for the ack.
    If an ack is returned properly, then we will free the buffer from the queue.
    Otherwise, we will just resend the buffer from the retry queue up to the
    maximum retries.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the retry list. The retry list is for devices that don't 
        support hardware retries. A frame will be buffered in this list and if
        the transmission fails, the frame will be pulled from this list and retried.
*/
/**************************************************************************/
LIST(mac_retry_list);

static struct ctimer mac_retry_tmr;  ///< Callback timer for the retry timeout

#define MAC_RETRY_TMR_INCREMENT 100     ///< Timeout value that triggers the callback

/**************************************************************************/
/*!
    Init the retry list (queue) and start the periodic timer. The timer is
    always running and on each timeout will check the queue and decrement
    the expiry in any entries in the queue.
*/
/**************************************************************************/
void mac_retry_init()
{
    list_init(mac_retry_list);
    ctimer_set(&mac_retry_tmr, MAC_RETRY_TMR_INCREMENT, mac_retry_periodic, NULL);
}

/**************************************************************************/
/*!
    Allocate a memory pointer. If the alloc is successful, then fill out the
    memory with the mac retry struct info and return the mem ptr.
*/
/**************************************************************************/
static mem_ptr_t *mac_retry_alloc()
{
    mem_ptr_t *mem_ptr = NULL;

    if ((mem_ptr = mem_heap_alloc(sizeof(mac_retry_t))) != NULL)
    {
        RETRY_ENTRY(mem_ptr)->retries    = aMacMaxFrameRetries;
        RETRY_ENTRY(mem_ptr)->expiry     = aMacAckWaitDuration;
        list_add(mac_retry_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the retry mem ptr from the list and free the buf. Then free the managed
    memory.
*/
/**************************************************************************/
static void mac_retry_free(mem_ptr_t *mem_ptr)
{
    list_remove(mac_retry_list, mem_ptr);
    buf_free(RETRY_ENTRY(mem_ptr)->buf);
    mem_heap_free(mem_ptr);
}

/**************************************************************************/
/*!
    Clear all retry memory pointers from the retry list.
*/
/**************************************************************************/
void mac_retry_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(mac_retry_list); mem_ptr != NULL; mem_ptr = list_chop(mac_retry_list))
    {
        mac_retry_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find a retry queue entry based on its DSN. A transmitted frame and its
    corresponding ACK will contain the same DSN. This is how we match up the
    ACK to its frame. Thus, we can use the DSN from the ack to find the retry queue
    entry of a frame.
*/
/**************************************************************************/
static mem_ptr_t *mac_retry_find(U8 dsn)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(mac_retry_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (RETRY_ENTRY(mem_ptr)->dsn == dsn)
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add an entry to the retry list.
*/
/**************************************************************************/
void mac_retry_add(buffer_t *buf, U8 dsn, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mac_retry_alloc()) != NULL)
    {
        RETRY_ENTRY(mem_ptr)->buf      = buf;
        RETRY_ENTRY(mem_ptr)->dsn      = dsn;
        RETRY_ENTRY(mem_ptr)->handle   = handle;
    }
}

/**************************************************************************/
/*!
    Handle an incoming ack. When an ack arrives we send it to the ack handler
    to look for a corresponding retry entry. If an entry is found, then the
    entry will be freed, and a data confirm will be sent to the next higher
    layer. If no entry is found, the ack is just discarded.
*/
/**************************************************************************/
void mac_retry_ack_handler(U8 dsn)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mac_retry_find(dsn)) != NULL)
    {
        mac_data_conf(MAC_SUCCESS, RETRY_ENTRY(mem_ptr)->handle);
        mac_retry_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Remove the retry entry with the specified dsn from the retry list.
*/
/**************************************************************************/
void mac_retry_rem(U8 dsn)
{
    mac_retry_free(mac_retry_find(dsn));
}

/**************************************************************************/
/*!
    The periodic timer decrements the expiry for each entry in the retry queue
    at a specified interval. When the expiry reaches zero, then this function is
    called. If the number of retries is still below the maximum, the frame will
    be re-sent. If the maximum number of retries has been reached, the entry
    will be freed and a data confirm will be sent to the next higher layer with
    a status of NO ACK.
*/
/**************************************************************************/
static void mac_retry_expire(mem_ptr_t *mem_ptr)
{
    RETRY_ENTRY(mem_ptr)->retries--;

    if (RETRY_ENTRY(mem_ptr)->retries > 0)
    {
        RETRY_ENTRY(mem_ptr)->expiry = aMacAckWaitDuration;
        mac_out(RETRY_ENTRY(mem_ptr)->buf, true, RETRY_ENTRY(mem_ptr)->dsn, RETRY_ENTRY(mem_ptr)->handle);
    }
    else
    {
        mac_data_conf(MAC_NO_ACK, RETRY_ENTRY(mem_ptr)->handle);
        mac_retry_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    This is a function that gets called periodically. Its main purpose is to
    check for expired retry entries and to decrement the expiration times
    for each of the entries. If an entry is expired, it will be retried or
    retired based on how many retries are still left for that entry.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'ptr' not referenced
//lint -e{818} Info 818: Pointer parameter ptr' could be declared as pointing to const
void mac_retry_periodic(void *ptr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(mac_retry_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // go through the retry list and check for expired entries. if any
        // are expired, then process them. otherwise, decrement the expiry.
        if (RETRY_ENTRY(mem_ptr)->expiry == 0)
        {
            mac_retry_expire(mem_ptr);
        }
        else
        {
            RETRY_ENTRY(mem_ptr)->expiry--;
        }
    }
    ctimer_set(&mac_retry_tmr, MAC_RETRY_TMR_INCREMENT, mac_retry_periodic, NULL);
}

/**************************************************************************/
/*!
    This function just stops the periodic callback timer.
*/
/**************************************************************************/
void mac_retry_stop_tmr()
{
    ctimer_stop(&mac_retry_tmr);
}
