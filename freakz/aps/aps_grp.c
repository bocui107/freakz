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
    \file aps_grp.c
    \ingroup aps
    \brief APS group table

        This file contains functions that implement the group table. The group 
        table allows frames to be sent to multiple groups of endpoints. The reason
        this is desirable is because one Zigbee controller may house multiple 
        identical devices, each on a separate endpoint. The standard case is a set 
        of lights, each one of them have similar functionality, but each endpoint
        controls a different light. Certain lights can be grouped together to be
        controlled by a single switch or dimmer. This would require the use
        of the group table.

        Also, the group table is required by the home automation profile and many
        of the basic clusters.

*/
/**************************************************************************/
#include "freakz.h"

LIST(grp_tbl);          ///< List head for the group table.

/**************************************************************************/
/*!
        Initialize the group table.
*/
/**************************************************************************/
void aps_grp_init()
{
    list_init(grp_tbl);
}

/**************************************************************************/
/*!
    Alloc a group structure element from the memory heap and add it to the
        group table. This function will also return a pointer to the memory handle.
*/
/**************************************************************************/
static mem_ptr_t *aps_grp_alloc()
{
    mem_ptr_t *mem_ptr = NULL;

    if ((mem_ptr = mem_heap_alloc(sizeof(aps_grp_t))) != NULL)
    {
        list_add(grp_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Free the memory pointer that holds the group entry.
*/
/**************************************************************************/
static void aps_grp_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(grp_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find an entry that has a matching id and endpoint and return the mem ptr.
    If it doesn't exist, return a NULL pointer.
*/
/**************************************************************************/
static mem_ptr_t *aps_grp_find_entry(U16 id, U8 ep)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(grp_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // if the id and the endpoints match, then we found the entry we're looking for
        if ((GROUP_ENTRY(mem_ptr)->id == id) && (GROUP_ENTRY(mem_ptr)->ep == ep))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add a group table entry with the specified group ID and endpoint. If it
    already exists, then exit with a APS_SUCCESS status. If it doesn't exist,
    then create a new entry and fill it out.
*/
/**************************************************************************/
U8 aps_grp_add(U16 id, U8 ep)
{
    mem_ptr_t *mem_ptr;

    // check if this entry exists already
    if ((aps_grp_find_entry(id, ep)) != NULL)
    {
        // this entry already exists
        return APS_SUCCESS;
    }

    // alloc a new entry and fill it out
    if ((mem_ptr = aps_grp_alloc()) != NULL)
    {
        GROUP_ENTRY(mem_ptr)->id = id;
        GROUP_ENTRY(mem_ptr)->ep = ep;
        return APS_SUCCESS;
    }
    else
    {
        // we ran out of heap memory. 
        return APS_FAIL;
    }
}

/**************************************************************************/
/*!
        Remove an entry from the group based on its id and its endpoint.
*/
/**************************************************************************/
U8 aps_grp_rem(U16 id, U8 ep)
{
    mem_ptr_t *mem_ptr;

    // find the id mem ptr
    if ((mem_ptr = aps_grp_find_entry(id, ep)) != NULL)
    {
        aps_grp_free(mem_ptr);
        return APS_SUCCESS;
    }
    return APS_INVALID_GROUP;
}

/**************************************************************************/
/*!
    Remove all groups from the group table
*/
/**************************************************************************/
void aps_grp_rem_all()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(grp_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        aps_grp_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Check to see if a group exists in the grp table
*/
/**************************************************************************/
bool aps_grp_exists(U16 id)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(grp_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (GROUP_ENTRY(mem_ptr)->id == id)
        {
            return true;
        }
    }
    return false;
}

/**************************************************************************/
/*!
    Return the list head for the group table.
*/
/**************************************************************************/
mem_ptr_t *aps_grp_get_head()
{
    return list_head(grp_tbl);
}

