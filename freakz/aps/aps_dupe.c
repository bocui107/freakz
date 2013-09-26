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
    \file aps_dupe.c
    \ingroup aps
    \brief APS duplicate frame checker

    Implement the APS duplicate rejection table. The dupe table checks for duplicate
    data received by the ACK layer. It may be possible that an APS ACK wasn't
    sent out in time from a received frame, so the frame is retried by the
    remote node. Hence, we will get two identical frames. This table is
    used to check if we receive a duplicate, and if so, discard the dupe
    frame.
*/
/**************************************************************************/
#include "freakz.h"
LIST(dupe_tbl);         ///< List head for the APS dupe table.

/**************************************************************************/
/*!
    Init the dupe table.
*/
/**************************************************************************/
void aps_dupe_init()
{
    list_init(dupe_tbl);
}

/**************************************************************************/
/*!
    Find a free entry from the dupe pool and add it to the dupe table.
*/
/**************************************************************************/
static mem_ptr_t *aps_dupe_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(aps_dupe_t))) != NULL)
    {
        list_add(dupe_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove an entry from the dupe table and free it.
*/
/**************************************************************************/
static void aps_dupe_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(dupe_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Allocate and add an entry to the dupe table and fill in the fields
    based on the function args. The entry will stay in the dupe table until
    it expires in DUPE_REJECT_TIMEOUT seconds.
*/
/**************************************************************************/
void aps_dupe_add(U16 src_addr, U8 aps_ctr)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = aps_dupe_alloc()) != NULL)
    {
        DUPE_ENTRY(mem_ptr)->src_addr = src_addr;
        DUPE_ENTRY(mem_ptr)->aps_ctr = aps_ctr;
        DUPE_ENTRY(mem_ptr)->expiry = DUPE_REJECT_TIMEOUT;
    }
}

/**************************************************************************/
/*!
    Checks for an entry in the dupe table with the same src addr
    and aps ctr value. If found, it will return true which means that the
    incoming frame should be discarded.
*/
/**************************************************************************/
bool aps_dupe_reject(U16 src_addr, U8 aps_ctr)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(dupe_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if ((DUPE_ENTRY(mem_ptr)->src_addr == src_addr) && (DUPE_ENTRY(mem_ptr)->aps_ctr == aps_ctr))
        {
            return true;
        }
    }
    return false;
}

/**************************************************************************/
/*!
    This function gets called every second and decrements the timeout value of
    any entries in the duplicate rejection table. If the entry has a timeout of
    zero, then it will be removed from the table.
*/
/**************************************************************************/
void aps_dupe_periodic()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(dupe_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (DUPE_ENTRY(mem_ptr)->expiry == 0)
        {
            aps_dupe_free(mem_ptr);
        }
        else
        {
            DUPE_ENTRY(mem_ptr)->expiry--;
        }
    }
}
