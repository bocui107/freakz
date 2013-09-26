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
    \file af_conf_tbl.c
    \ingroup af
    \brief AF confirmation table

    This file implements a table that tracks the data request handles. The
    handles act as identifiers so that when the service confirmations come
    in, the confirmation handle can be used to look up the destination ep.
    Once the destination ep is known, we can route the confirmation to its
    originating endpoint.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the AF confirmation table. Holds the handle of the frame and keeps track of
        the endpoint that it came from. When the confirmation comes back, we know
        where to send it.
*/
/**************************************************************************/
LIST(af_conf_tbl);

/**************************************************************************/
/*!
    Init the confirmation table.
*/
/**************************************************************************/
void af_conf_tbl_init()
{
    list_init(af_conf_tbl);
}

/**************************************************************************/
/*!
    Find a free entry and add it to the confirm table. Also, fill out the
    default expiration time.
*/
/**************************************************************************/
static mem_ptr_t *af_conf_tbl_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(af_conf_tbl_entry_t))) != NULL)
    {
        CONF_TBL_ENTRY(mem_ptr)->expiry = ZIGBEE_CONFIRM_INTERVAL;
        list_add(af_conf_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the entry from the confirm table and free it.
*/
/**************************************************************************/
void af_conf_tbl_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(af_conf_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Find the confirmation table entry by its handle and return the memory pointer
    to it.
*/
/**************************************************************************/
mem_ptr_t *af_conf_tbl_find(U8 handle)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(af_conf_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (CONF_TBL_ENTRY(mem_ptr)->handle == handle)
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Allocate and add an entry to the confirm table and fill it out with
    the specified args.
*/
/**************************************************************************/
void af_conf_tbl_add(U8 src_ep, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = af_conf_tbl_alloc()) != NULL)
    {
        CONF_TBL_ENTRY(mem_ptr)->src_ep = src_ep;
        CONF_TBL_ENTRY(mem_ptr)->handle = handle;
    }
}

/**************************************************************************/
/*!
    This function gets called periodically from the slow clock. It is used mainly
    to expire confirm table entries that are old. If an expiration occurs, it will
    send a confirm with a failure status to the originating endpoint.
*/
/**************************************************************************/
void af_conf_tbl_periodic()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(af_conf_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (CONF_TBL_ENTRY(mem_ptr)->expiry == 0)
        {
            DBG_PRINT_RAW("AF_CONF_TBL TIMEOUT! Src EP: %02X, Handle %02X.\n", CONF_TBL_ENTRY(mem_ptr)->src_ep, CONF_TBL_ENTRY(mem_ptr)->handle);
            aps_conf(AF_TIMEOUT, CONF_TBL_ENTRY(mem_ptr)->handle);
        }
        else
        {
            CONF_TBL_ENTRY(mem_ptr)->expiry--;
        }
    }
}
