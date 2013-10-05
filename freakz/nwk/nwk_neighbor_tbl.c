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
    \file nwk_neighbor_tbl.c
    \ingroup nwk
    \brief NWK neighbor table

    This file handles functions that manage the NWK layer's neighbor
    table. The neighbor table keeps track of all devices within
    listening range of this one.
*/
#include "freakz.h"

/*
 * List head for the network neighbor table. The neighbor table contains
 * all devices that are within listening range of us that we know about.
 * They should be all the single hop devices around and this table is the first
 * one we turn to when we need to decide how to forward a frame.
 */
LIST(nbor_tbl);

/* Init the neighbor table */
void nwk_neighbor_tbl_init()
{
	list_init(nbor_tbl);
}

/*
 * Search the array for a free entry. If one is found, insert it
 * into the neighbor table and return it.
 */
static mem_ptr_t *nwk_neighbor_tbl_alloc()
{
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = mem_heap_alloc(sizeof(nbor_tbl_entry_t))) != NULL)
		list_add(nbor_tbl, mem_ptr);

	return mem_ptr;
}

/* Remove the entry from the neighbor table and free it */
static void nwk_neighbor_tbl_free(mem_ptr_t *mem_ptr)
{
	if (mem_ptr)
	{
		list_remove(nbor_tbl, mem_ptr);
		mem_heap_free(mem_ptr);
	}
}

/* Remove all entries from the neighbor table */
void nwk_neighbor_tbl_clear()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_chop(nbor_tbl); mem_ptr != NULL; mem_ptr = list_chop(nbor_tbl))
	{
		nwk_neighbor_tbl_free(mem_ptr);
	}
}

/* Returns a pointer to the head of the neighbor table */
mem_ptr_t *nwk_neighbor_tbl_get_head()
{
	return list_head(nbor_tbl);
}

/* Find the specified address in the neighbor table */
static mem_ptr_t *nwk_neighbor_tbl_find(address_t *addr, U16 pan_id)
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(nbor_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (addr->mode == SHORT_ADDR)
		{
			if ((NBOR_ENTRY(mem_ptr)->nwk_addr == addr->short_addr) &&
			    (NBOR_ENTRY(mem_ptr)->pan_id == pan_id))
				break;
		} else if (addr->mode == LONG_ADDR) {
		if ((NBOR_ENTRY(mem_ptr)->ext_addr == addr->long_addr) &&
		    (NBOR_ENTRY(mem_ptr)->pan_id == pan_id))
			break;
		}
	}
	return mem_ptr;
}

/* Add an entry to the neighbor table */
void nwk_neighbor_tbl_add(nbor_tbl_entry_t *entry)
{
	mem_ptr_t *mem_ptr;
	address_t addr;

	if (entry) {
		/* we need to check for a duplicate entry */
		addr.mode = SHORT_ADDR;
		addr.short_addr = entry->nwk_addr;

		if ((mem_ptr = nwk_neighbor_tbl_find(&addr, entry->pan_id)) != NULL)
		{
			/*
			 * it's a dupe. use this entry and
			 * update it with all new info
			 */
			memcpy(NBOR_ENTRY(mem_ptr), entry, sizeof(nbor_tbl_entry_t));
			return;
		}

		/*
		 * passed the dupe check. it's a new entry so alloc
		 * a mem block and copy the info
		 */
		if ((mem_ptr = nwk_neighbor_tbl_alloc()) != NULL)
			memcpy(NBOR_ENTRY(mem_ptr), entry, sizeof(nbor_tbl_entry_t));
	}
}

/*
 * Remove the address from the neighbor table. Only works for devices in the
 * same PAN.
 */
void nwk_neighbor_tbl_rem(address_t *addr)
{
	mac_pib_t *pib = mac_pib_get();
	nwk_neighbor_tbl_free(nwk_neighbor_tbl_find(addr, pib->pan_id));
}

/* Return the parent of this node */
U16 nwk_neighbor_tbl_get_parent()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(nbor_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (NBOR_ENTRY(mem_ptr)->relationship == NWK_PARENT)
			return NBOR_ENTRY(mem_ptr)->nwk_addr;
	}
	return INVALID_NWK_ADDR;
}

/*
 * Check to see if the dest address exists in the neighbor table. This only
 * works for addresses within the same pan.
 */
bool nwk_neighbor_tbl_addr_exists(address_t *addr)
{
	mac_pib_t *pib = mac_pib_get();
	mem_ptr_t *mem_ptr;

	mem_ptr = nwk_neighbor_tbl_find(addr, pib->pan_id);
	if (mem_ptr)
		return true;
	else
		return false;
}

/* Get the neighbor entry from the same PAN with the spec'd address */
mem_ptr_t *nwk_neighbor_tbl_get_entry(address_t *addr)
{
	mac_pib_t *pib = mac_pib_get();
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = nwk_neighbor_tbl_find(addr, pib->pan_id)) != NULL)
	{
		return mem_ptr;
	}
	return NULL;
}

/* Return the number of entries in the neighbor table */
U8 nwk_neighbor_get_cnt()
{
	mem_ptr_t *mem_ptr;
	U8 i = 0;

	for (mem_ptr = list_head(nbor_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		i++;
	}
	return i;
}
