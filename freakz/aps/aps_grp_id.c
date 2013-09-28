/***************************************************************************
    Copyright (C) 2008 FreakLabs

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Originally written by Christopher Wang.
    Please post support questions to the FreakLabs forum.
***************************************************************************/
/*!
    \file aps_grp_id.c
    \ingroup aps
    \brief APS group ID table

	The Group ID table contains a unique identifier for each group. This id
	will be used by remote devices to address the group and will be stored
	in this table. The group IDs would normally be set up by a commissioning
	device when a new device is added to the network.

*/
/**************************************************************************/
#include "freakz.h"

LIST(grp_id_tbl); 	///< List head for the Group ID table.

/**************************************************************************/
/*!
	Init the group ID table.
*/
/**************************************************************************/
void aps_grp_id_init()
{
    list_init(grp_id_tbl);
}

/**************************************************************************/
/*!
    Alloc a group id structure element from the memory heap and add it to the
	table. This function will also return a handle to the memory block.
*/
/**************************************************************************/
static mem_ptr_t *aps_grp_id_alloc()
{
    mem_ptr_t *mem_ptr = NULL;

    if ((mem_ptr = mem_heap_alloc(sizeof(aps_grp_id_t))) != NULL)
    {
        list_add(grp_id_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
	Free a group ID entry from the table. This function will actually remove
	the memory handle from the table and free the memory block.
*/
/**************************************************************************/
static void aps_grp_id_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(grp_id_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find a group id entry based on the name and the len of the name. It will
    do a mem compare based on the number of chars in the name (len) and if it
    matches, then it will return the name.
*/
/**************************************************************************/
mem_ptr_t *aps_grp_id_find_name(U8 *name)
{
    mem_ptr_t *mem_ptr = NULL;

    for (mem_ptr = list_head(grp_id_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (memcmp(GROUP_ID_ENTRY(mem_ptr)->name, name, GROUP_ID_ENTRY(mem_ptr)->len) == 0)
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Find a group id entry based on the ID. Returns a mem pointer to the entry
    if found or NULL if not found.
*/
/**************************************************************************/
mem_ptr_t *aps_grp_id_find_id(U16 id)
{
    mem_ptr_t *mem_ptr = NULL;

    for (mem_ptr = list_head(grp_id_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (memcmp(&GROUP_ID_ENTRY(mem_ptr)->id, &id, sizeof(U16)) == 0)
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Add a group ID entry to group ID table with the specified parameters.
*/
/**************************************************************************/
U8 aps_grp_id_add(U8 *name, U8 len, U16 id)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_grp_id_find_id(id)) != NULL)
    {
        // an entry with the same id exists. just use this entry and overwrite the name.
        memcpy(GROUP_ID_ENTRY(mem_ptr)->name, name, len);
        GROUP_ID_ENTRY(mem_ptr)->len = len;
        return APS_SUCCESS;
    }
    else if ((mem_ptr = aps_grp_id_find_name(name)) != NULL)
    {
        // an entry with the same name exists. just overwrite the id of the entry.
        GROUP_ID_ENTRY(mem_ptr)->id = id;
        return APS_SUCCESS;
    }
    else
    {
        // this is a completely new entry so alloc a structure and fill in the name, len, and id.
        if ((mem_ptr = aps_grp_id_alloc()) != NULL)
        {
            memcpy(GROUP_ID_ENTRY(mem_ptr)->name, name, len);
            GROUP_ID_ENTRY(mem_ptr)->len    = len;
            GROUP_ID_ENTRY(mem_ptr)->id     = id;
            return APS_SUCCESS;
        }

        // no more memory. hope we don't end up here.
        return APS_FAIL;
    }
}

/**************************************************************************/
/*!
    Remove a group id entry from the group ID table.
*/
/**************************************************************************/
U8 aps_grp_id_remove(U8 *name, U16 id)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_grp_id_find_id(id)) != NULL)
    {
        if (memcmp(GROUP_ID_ENTRY(mem_ptr)->name, name, GROUP_ID_ENTRY(mem_ptr)->len) == 0)
        {
            aps_grp_id_free(mem_ptr);
            return APS_SUCCESS;
        }
    }
    return APS_INVALID_GROUP;
}
