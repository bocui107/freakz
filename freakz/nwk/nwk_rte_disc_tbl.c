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
    \file nwk_rte_disc_tbl.c
    \ingroup nwk
    \brief NWK discovery table

    The route discovery table is used for implementing route discovery
    in Zigbee. When trying to discover a route, route request command
    frames are sent out. To understand the usage of the route discovery
    table, its probably best to understand the sequence of events for
    route discovery:

    - A route request command frame is broadcast from the source node.
    - Each node that receives the route request will create a route
    discovery table entry with the route request info.
    - Each node will then broadcast-relay the route request.
    - When the route request reaches the destination node, then the
    destination node will generate a unicast route reply to the node
    that sent it the route request.
    - The node receiving the route reply will then unicast the route
    reply to the node that sent it the route request previously by looking
    up the info in the discovery table. It will also add the path cost
    which is also stored in the discovery table.
    - The reply will be unicast until it reaches the source node.
    - After the specified route discovery time, the route discovery
    entry will expire and be removed from the table.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the route discovery table. When a route request frame is
        received during somebody's route discovery process, then we need to create
        a discovery entry and keep it here. It contains info on the previous hop.
        If we get a route reply before the discovery timeout, then we add this
        discovery entry to the routing table and it ends up becoming a route. That is
        unless we find another path with less of a path cost.
*/
/**************************************************************************/
LIST(disc_tbl);

/**************************************************************************/
/*!
    Init the route discovery table.
*/
/**************************************************************************/
void nwk_rte_disc_tbl_init()
{
    list_init(disc_tbl);
}

/**************************************************************************/
/*!
    Find a free entry in the discovery entry pool and add it to the discovery
    table.
*/
/**************************************************************************/
static mem_ptr_t *nwk_rte_disc_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(disc_entry_t))) != NULL)
    {
        list_add(disc_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the discovery entry from the table and free it.
*/
/**************************************************************************/
void nwk_rte_disc_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(disc_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Remove all entries from the route discovery table.
*/
/**************************************************************************/
void nwk_rte_disc_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(disc_tbl); mem_ptr != NULL; mem_ptr = list_chop(disc_tbl))
    {
        nwk_rte_disc_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find a discovery entry based on the route request ID and the source address.
    This is used when a route reply arrives. The route reply contains the
    original route request ID which is used to identify the discovery entry,
    along with the source address.
*/
/**************************************************************************/
mem_ptr_t *nwk_rte_disc_find(U8 rreq_id, U16 src_addr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(disc_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if ((DISC_ENTRY(mem_ptr)->rreq_id == rreq_id) && (DISC_ENTRY(mem_ptr)->src_addr == src_addr))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add a new entry to the discovery table and fill it out with the specified
    arguments.
*/
/**************************************************************************/
void nwk_rte_disc_add_new(U8 rreq_id, U16 src_addr, U16 sender_addr, U16 dest_addr, U8 cost)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = nwk_rte_disc_alloc()) != NULL)
    {
        DISC_ENTRY(mem_ptr)->rreq_id     = rreq_id;
        DISC_ENTRY(mem_ptr)->sender_addr = sender_addr;
        DISC_ENTRY(mem_ptr)->src_addr    = src_addr;
        DISC_ENTRY(mem_ptr)->dest_addr   = dest_addr;
        DISC_ENTRY(mem_ptr)->fwd_cost    = cost;
        DISC_ENTRY(mem_ptr)->resid_cost  = 0xff;
        DISC_ENTRY(mem_ptr)->expiry      = NWK_RTE_DISC_TIME;
    }
}

/**************************************************************************/
/*!
    This is a periodic funciton that is called once per second by the slow clock
    and decrements the expiry in the route discovery table.
*/
/**************************************************************************/
void nwk_rte_disc_periodic()
{
    mem_ptr_t *rte_mem_ptr;
    mem_ptr_t *disc_mem_ptr;

    for (disc_mem_ptr = list_head(disc_tbl); disc_mem_ptr != NULL; disc_mem_ptr = disc_mem_ptr->next)
    {
        if (DISC_ENTRY(disc_mem_ptr)->expiry == 0)
        {
            // free entry if its expired. also, if the rte table entry is not active, we will free it as well.
            DBG_PRINT("NWK_RTE_DISC_PERIODIC_TMR: Freeing discovery entry - src addr = %02X\n.", DISC_ENTRY(disc_mem_ptr)->src_addr);
            if ((rte_mem_ptr = nwk_rte_tbl_find(DISC_ENTRY(disc_mem_ptr)->dest_addr)) != NULL)
            {
                // free the rte table entry if its not active.
                if (RTE_ENTRY(rte_mem_ptr)->status != NWK_ACTIVE)
                {
                    nwk_rte_tbl_free(rte_mem_ptr);
                }
            }
            nwk_rte_disc_free(disc_mem_ptr);
        }
        else
        {
            // decrement the expiry
            DISC_ENTRY(disc_mem_ptr)->expiry = (DISC_ENTRY(disc_mem_ptr)->expiry > CLOCK_SECOND) ? (DISC_ENTRY(disc_mem_ptr)->expiry - CLOCK_SECOND) : 0;
        }
    }
}

