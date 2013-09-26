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
    \file nwk_addr_map.c
    \ingroup nwk
    \brief NWK address map

    This file implements the address map for the NWK layer. The address map
    is used to keep a list of devices' nwk address and extended addresses.
    It can be used to look up a device's nwk address as long as the extended
    address is known, or vice versa.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the address map. The address map contains the corresponding
        extended addresses for each network address nickname contained in this table.
        The extended address is sometimes used for things like service discovery or
        some other stuff that the spec writers decided to torture stack writers with.
*/
/**************************************************************************/
LIST(addr_map);

/**************************************************************************/
/*!
    Initialize the NWK address map table.
*/
/**************************************************************************/
void nwk_addr_map_init()
{
    list_init(addr_map);
}

/**************************************************************************/
/*!
    Allocate an address map entry to be used to store a device's nwk and extended
    address.
*/
/**************************************************************************/
static mem_ptr_t *nwk_addr_map_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(nwk_addr_map_t))) != NULL)
    {
        list_add(addr_map, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the addr map entry from the list and free it.
*/
/**************************************************************************/
static void nwk_addr_map_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(addr_map, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Clear the address map of all entries.
*/
/**************************************************************************/
void nwk_addr_map_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(addr_map); mem_ptr != NULL; mem_ptr = list_chop(addr_map))
    {
        nwk_addr_map_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find an addr map entry by the given address. The address structure is used
    to indicate whether we will be looking for an extended addr or a short addr.
*/
/**************************************************************************/
static mem_ptr_t *nwk_addr_map_find(address_t *addr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(addr_map); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (((addr->mode == SHORT_ADDR) && (addr->short_addr == ADDR_MAP_ENTRY(mem_ptr)->nwk_addr)) ||
            ((addr->mode == LONG_ADDR) && (addr->long_addr == ADDR_MAP_ENTRY(mem_ptr)->ext_addr)))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add an address map entry to the address map. Check to see if the entry
    exists first. If not, then allocate a new one and add it.
*/
/**************************************************************************/
void nwk_addr_map_add(U16 nwk_addr, U64 ext_addr, U8 capab)
{
    mem_ptr_t *mem_ptr;
    address_t addr;

    // first check to see if the address exists inside our map
    addr.mode = SHORT_ADDR;
    addr.short_addr = nwk_addr;

    if ((mem_ptr = nwk_addr_map_find(&addr)) == NULL)
    {
        // short address doesnt exist in the addr map. try the long addr.
        addr.mode = LONG_ADDR;
        addr.long_addr = ext_addr;
        if ((mem_ptr = nwk_addr_map_find(&addr)) == NULL)
        {
            // long addr doesnt exist in the addr map either. try to alloc an address.
            if ((mem_ptr = nwk_addr_map_alloc()) == NULL)
            {
                // if no memory, then just return.
                return;
            }
        }
    }

    // if we made it this far, then there must be something on the mem pointer. Overwrite the entry with
    // the address info. only overwrite the capab info if its not 0xff.
    if (mem_ptr)
    {
        ADDR_MAP_ENTRY(mem_ptr)->nwk_addr = nwk_addr;
        ADDR_MAP_ENTRY(mem_ptr)->ext_addr = ext_addr;

        // don't overwrite the capability info if its 0xff (an impossible value)
        if (capab != 0xff)
        {
            ADDR_MAP_ENTRY(mem_ptr)->capab = capab;
        }
    }
}

/**************************************************************************/
/*!
    Remove the specified nwk address entry from the address map.
*/
/**************************************************************************/
void nwk_addr_map_rem_nwk_addr(U16 nwk_addr)
{
    address_t addr;

    addr.mode = SHORT_ADDR;
    addr.short_addr = nwk_addr;
    nwk_addr_map_free(nwk_addr_map_find(&addr));
}

/**************************************************************************/
/*!
    Remove the specified ext address entry from the address map.
*/
/**************************************************************************/
void nwk_addr_map_rem_ext_addr(U64 ext_addr)
{
    address_t addr;

    addr.mode = LONG_ADDR;
    addr.long_addr = ext_addr;
    nwk_addr_map_free(nwk_addr_map_find(&addr));
}

/**************************************************************************/
/*!
    Get the network address, given the extended address.
*/
/**************************************************************************/
U16 nwk_addr_map_get_nwk_addr(U64 ext_addr)
{
    mem_ptr_t *mem_ptr;
    address_t addr;

    addr.mode = LONG_ADDR;
    addr.long_addr = ext_addr;

    if ((mem_ptr = nwk_addr_map_find(&addr)) != NULL)
    {
        return ADDR_MAP_ENTRY(mem_ptr)->nwk_addr;
    }
    return INVALID_NWK_ADDR;
}

/**************************************************************************/
/*!
    Get the extended address, given the network address.
*/
/**************************************************************************/
U64 nwk_addr_map_get_ext_addr(U16 nwk_addr)
{
    mem_ptr_t *mem_ptr;
    address_t addr;

    addr.mode = SHORT_ADDR;
    addr.short_addr = nwk_addr;

    if ((mem_ptr = nwk_addr_map_find(&addr)) != NULL)
    {
        return ADDR_MAP_ENTRY(mem_ptr)->ext_addr;
    }
    return INVALID_EXT_ADDR;
}
