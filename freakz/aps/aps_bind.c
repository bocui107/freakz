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
    \file aps_bind.c
    \ingroup aps
    \brief APS binding table

    This file implements the binding table for the APS layer.

    \note Binding functionality has not been fully implemented and tested yet.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the binding table. The binding table allows us to use indirect
        addressing at the application layer. The clusters and endpoints don't need to
        have any idea of where they are sending the data to. They just respond to
        requests and the addresses can be looked up in the binding table. The app layer
        also supports direct addressing, but the binding is supposed to make everything
        easier...for everyone except me...boo hoo...
*/
/**************************************************************************/
LIST(bind_tbl);

/**************************************************************************/
/*!
    Init the binding table.
*/
/**************************************************************************/
void aps_bind_init()
{
    list_init(bind_tbl);
}

/**************************************************************************/
/*!
    Allocate a new binding entry memory pointer and add it to the binding table.
*/
/**************************************************************************/
static mem_ptr_t *aps_bind_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr= mem_heap_alloc(sizeof(aps_bind_t))) != NULL)
    {
        list_add(bind_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Free a binding entry from the binding table.
*/
/**************************************************************************/
static void aps_bind_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(bind_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Locate a binding entry with the specified src endpoint and cluster ID
    and return a memory handle to it. Returns NULL if not found.
*/
/**************************************************************************/
mem_ptr_t *aps_bind_find_dest(U8 src_ep, U16 clust)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(bind_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if ((BIND_ENTRY(mem_ptr)->src_ep == src_ep) && (BIND_ENTRY(mem_ptr)->clust == clust))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Locate an entry with an exact match of src ep, clust, dest addr, and dest ep.
    Return a memory handle only if everything matches.
*/
/**************************************************************************/
static mem_ptr_t *aps_bind_find(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_bind_find_dest(src_ep, clust)) != NULL)
    {
        if (BIND_ENTRY(mem_ptr)->dest_addr.mode != dest_addr->mode)
        {
            // entry does not have the same mode hence its not an exact match
            return NULL;
        }

        if (dest_addr->mode == BIND_GRP_ADDR)
        {
            if (BIND_ENTRY(mem_ptr)->dest_addr.short_addr == dest_addr->short_addr)
            {
                return mem_ptr;
            }
        }
        else if (dest_addr->mode == BIND_EXT_ADDR)
        {
            if ((BIND_ENTRY(mem_ptr)->dest_addr.long_addr == dest_addr->long_addr) &&
                (BIND_ENTRY(mem_ptr)->dest_ep == dest_ep))
            {
                return mem_ptr;
            }
        }
    }
    return NULL;
}

/**************************************************************************/
/*!
    Allocate a free binding entry and add it to the binding table.
*/
/**************************************************************************/
static U8 aps_bind_add(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust)
{
    mem_ptr_t *mem_ptr;

    // first check for a duplicate binding entry
    if ((mem_ptr = aps_bind_find(src_ep, dest_addr, dest_ep, clust)) != NULL)
    {
        // we have a duplicate entry. return success. it's already in the table.
        return APS_SUCCESS;
    }

    // we don't have a similar entry. allocate a new one.
    if ((mem_ptr = aps_bind_alloc()) != NULL)
    {
        BIND_ENTRY(mem_ptr)->src_ep         = src_ep;
        BIND_ENTRY(mem_ptr)->dest_ep        = dest_ep;
        BIND_ENTRY(mem_ptr)->clust          = clust;
        memcpy(&BIND_ENTRY(mem_ptr)->dest_addr, dest_addr, sizeof(address_t));
        return APS_SUCCESS;
    }
    else
    {
        // wtf...we're out of memory
        return APS_FAIL;
    }
}

/**************************************************************************/
/*!
    Return the list head of the binding table.
*/
/**************************************************************************/
mem_ptr_t *aps_bind_get_head()
{
    return list_head(bind_tbl);
}

/**************************************************************************/
/*!
    Add a binding entry with the given parameters. This is just a wrapper
    to make things conform to the spec naming. It just calls the aps_bind_add
    function which will handle the dupe checking, etc and add the entry.
*/
/**************************************************************************/
U8 aps_bind_req(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust)
{
    return aps_bind_add(src_ep, dest_addr, dest_ep, clust);
}

/**************************************************************************/
/*!
    Free a binding entry in the binding table. If a device is found, then it will
    be removed from the table. Otherwise, it will issue an error status.
*/
/**************************************************************************/
U8 aps_unbind_req(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_bind_find(src_ep, dest_addr, dest_ep, clust)) != NULL)
    {
        aps_bind_free(mem_ptr);
        return APS_SUCCESS;
    }
    return APS_NO_BOUND_DEVICE;
}
