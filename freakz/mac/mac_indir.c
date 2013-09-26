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
    \file mac_indir.c
    \ingroup mac
    \brief MAC indirect transfer handler

    This file handles the indirect queue which queues up buffers for
    later retrieval by child nodes. The indirect data gets added to the
    queue and when a data request command frame arrives from the child,
    the indirect data is retrieved and sent out. The reason for this is that
    most end devices will be sleeping most of the time so we can't just
    send out frames to them whenever we want to.
*/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the indirect transfer list. The indirect transfer list
        contains all of the frames that are buffered for the sleepy end device
        children.
*/
/**************************************************************************/
LIST(indir_list);

/**************************************************************************/
/*!
    Init the list used to implement the indirect queue.
*/
/**************************************************************************/
void mac_indir_init()
{
    list_init(indir_list);
}

/**************************************************************************/
/*!
    Allocate an indirect entry from the indirect struct pool. If no free entry
    exists, then return NULL.
*/
/**************************************************************************/
static mem_ptr_t *mac_indir_alloc()
{
    nwk_nib_t *nib = nwk_nib_get();
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(mac_indir_t))) != NULL)
    {
        INDIR_ENTRY(mem_ptr)->expiry = nib->traxn_persist_time;
        list_add(indir_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Free an indirect entry back into the indirect pool. This consists of two
    operations, which is to set the alloc value to false and to remove it
    from the indirect list (queue).
*/
/**************************************************************************/
static void mac_indir_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(indir_list, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Clear the indirect queue of all entries. This is usually only done on
    startup.
*/
/**************************************************************************/
void mac_indir_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(indir_list); mem_ptr != NULL; mem_ptr = list_chop(indir_list))
    {
        buf_free(INDIR_ENTRY(mem_ptr)->buf);
        mac_indir_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find a frame with the specified address in the indirect queue. The address
    structure is a union that can be used to specify either a short address or
    an extended address.
*/
/**************************************************************************/
static mem_ptr_t *mac_indir_find(const address_t *addr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(indir_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (((addr->mode == LONG_ADDR)  && (addr->long_addr == INDIR_ENTRY(mem_ptr)->addr.long_addr)) ||
            ((addr->mode == SHORT_ADDR) && (addr->short_addr == INDIR_ENTRY(mem_ptr)->addr.short_addr)))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add an indirect entry to the indirect list and fill in the info.
*/
/**************************************************************************/
void mac_indir_add(buffer_t *buf, address_t *addr, bool ack_req, U8 dsn, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mac_indir_alloc()) != NULL)
    {
        memcpy(&INDIR_ENTRY(mem_ptr)->addr, addr, sizeof(address_t));
        INDIR_ENTRY(mem_ptr)->buf      = buf;
        INDIR_ENTRY(mem_ptr)->dsn      = dsn;
        INDIR_ENTRY(mem_ptr)->handle   = handle;
        INDIR_ENTRY(mem_ptr)->ack_req  = ack_req;
        drvr_set_frm_pend(true);
    }
}

/**************************************************************************/
/*!
    When a data request comes in, this function will handle the data request.
    It checks the indirect list to see if there are any buffers pending for the
    requesting node. If a buffer is found, then it will send the buffer out and
    free the indirect list entry.
*/
/**************************************************************************/
void mac_indir_data_req_handler(const address_t *addr)
{
    mem_ptr_t *mem_ptr;
    mac_pib_t *pib = mac_pib_get();

    if ((mem_ptr = mac_indir_find(addr)) != NULL)
    {
        mac_tx_handler(INDIR_ENTRY(mem_ptr)->buf, NULL, false, INDIR_ENTRY(mem_ptr)->ack_req, INDIR_ENTRY(mem_ptr)->dsn, INDIR_ENTRY(mem_ptr)->handle);
        mac_comm_status_ind(pib->pan_id, INDIR_ENTRY(mem_ptr)->addr, MAC_SUCCESS, INDIR_ENTRY(mem_ptr)->handle);
        mac_indir_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Check the indirect list to see if a frame is pending. If not, then return
    false.
*/
/**************************************************************************/
bool mac_indir_frm_pend(const address_t *addr)
{
    return (mac_indir_find(addr) != NULL);
}

/**************************************************************************/
/*!
    The periodic function is called by the slow clock once per second and
    will decrement the expiry of each indirect entry in the queue. If the
    expiry goes to zero, then that entry has timed out and will be removed
    from the queue. A comm status indication will also be sent to the next
    higher layer stating that the indirect transmission failed.
*/
/**************************************************************************/
void mac_indir_periodic()
{
    mem_ptr_t *mem_ptr;
    mac_pib_t *pib = mac_pib_get();

    for (mem_ptr = list_head(indir_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (INDIR_ENTRY(mem_ptr)->expiry == 0)
        {
            buf_free(INDIR_ENTRY(mem_ptr)->buf);
            mac_indir_free(mem_ptr);
            mac_comm_status_ind(pib->pan_id, INDIR_ENTRY(mem_ptr)->addr, MAC_TRANSACTION_EXPIRED, INDIR_ENTRY(mem_ptr)->handle);
        }
        else
        {
            INDIR_ENTRY(mem_ptr)->expiry--;
        }
    }
}
