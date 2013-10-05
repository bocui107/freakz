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
    \file nwk_rte_tbl.c
    \ingroup nwk
    \brief NWK mesh routing tables

    This file implements the routing table for the NWK layer. The
    routing table entries will store the next hop information for a
    particular destination nwk address.

    \todo Implement aging algorithm for routing table entries
*/
#include "freakz.h"

/*
 * List head for the routing table. The routing table contains routing
 * entries that hold the next hop addresses for destinations that aren't
 * within a single hop from this device. It's the main mechanism to implement
 * the Zigbee multi-hop routing.
 */
LIST(rte_tbl);

/* Init the routing table */
void nwk_rte_tbl_init()
{
	list_init(rte_tbl);
}

/* Return the lsit head for the routing table */
mem_ptr_t *nwk_rte_tbl_get_head()
{
	return list_head(rte_tbl);
}

/* Find a free entry from the rte pool and add it to the rte table */
static mem_ptr_t *nwk_rte_tbl_alloc()
{
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = mem_heap_alloc(sizeof(rte_entry_t))) != NULL)
	{
		list_add(rte_tbl, mem_ptr);
	}
	return mem_ptr;
}

/* Remove the entry from the route table and free it */
void nwk_rte_tbl_free(mem_ptr_t *mem_ptr)
{
	if (mem_ptr) {
		list_remove(rte_tbl, mem_ptr);
		mem_heap_free(mem_ptr);
	}
}

/* Remove all entries from the routing table */
void nwk_rte_tbl_clear()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_chop(rte_tbl); mem_ptr != NULL; mem_ptr = list_chop(rte_tbl))
	{
		nwk_rte_tbl_free(mem_ptr);
	}
}

/* Find an entry with the specified destination network address */
mem_ptr_t *nwk_rte_tbl_find(U16 dest_addr)
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(rte_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (RTE_ENTRY(mem_ptr)->dest_addr == dest_addr)
			break;
	}
	return mem_ptr;
}

/* Remove the destination address from the routing tables */
void nwk_rte_tbl_rem(U16 addr)
{
	nwk_rte_tbl_free(nwk_rte_tbl_find(addr));
}

/* Get the next hop value from the routing table */
U16 nwk_rte_tbl_get_next_hop(U16 dest_addr)
{
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = nwk_rte_tbl_find(dest_addr)) != NULL)
	{
		if ((RTE_ENTRY(mem_ptr)->status == NWK_ACTIVE) ||
		    (RTE_ENTRY(mem_ptr)->status == NWK_VALIDATION_UNDERWAY))
		{
			/*
			 * set the status to active no matter if it is
			 * active or validation underway since we're
			 * going to use the route.
			 */
			RTE_ENTRY(mem_ptr)->status = NWK_ACTIVE;
			return RTE_ENTRY(mem_ptr)->next_hop;
		}
	}
	return INVALID_NWK_ADDR;
}

/* Check if a route exists in the table */
bool nwk_rte_tbl_rte_exists(U16 dest_addr)
{
	return (nwk_rte_tbl_find(dest_addr)) ? true : false;
}

/*
 * Add an entry to the routing table with the specified destination address
 * and status. First check to see if the entry already exists. If it does,
 * then just update the status. If not, then add it to the table.
 */
void nwk_rte_tbl_add_new(U16 dest_addr, U8 status)
{
	mem_ptr_t *mem_ptr;

	/* first check to see if the destination exists */
	for (mem_ptr = list_head(rte_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (RTE_ENTRY(mem_ptr)->dest_addr == dest_addr)
		{
			RTE_ENTRY(mem_ptr)->status = status;
			return;
		}
	}

	/*
	 * looks like the destination entry doesn't exist.
	 * allocate a new entry and add it to the routing table.
	 */
	if ((mem_ptr = nwk_rte_tbl_alloc()) != NULL)
	{
		RTE_ENTRY(mem_ptr)->dest_addr    = dest_addr;
		RTE_ENTRY(mem_ptr)->status       = status;
	}
}
