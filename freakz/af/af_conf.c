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
    \file af_conf.c
    \ingroup af
    \brief AF confirmation queue

    This file implements the confirmation queue for the AF layer.
    When a data confirm arrives, it's temporarily stored in the
    confirm queue until the AF process has time to handle it.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the AF confirmation queue. Holds the confirmations to be processed. Mainly
        used to provide a break in the confirm path so that the MCU won't be hogged
        by processing the confirmation. Otherwise, since I have no control over
        the application that processes the confirmation, it could end up taking
        a lot of MCU time.
*/
/**************************************************************************/
LIST(af_conf_queue);

/**************************************************************************/
/*!
    Init the confirmation queue.
*/
/**************************************************************************/
void af_conf_init()
{
    list_init(af_conf_queue);
}

/**************************************************************************/
/*!
    Find a free confirm entry and add it to the confirm queue.
*/
/**************************************************************************/
static mem_ptr_t *af_conf_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(af_conf_entry_t))) != NULL)
    {
        list_add(af_conf_queue, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove a confirm entry from the queue and free it.
*/
/**************************************************************************/
void af_conf_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(af_conf_queue, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Allocate and add a confirm entry to the confirm queue. Fill out the entry
    with the specified arguments. This is different from the confirmation
    table which is used to track the confirms and route them to the correct
    endpoint.
*/
/**************************************************************************/
void af_conf_add(U8 status, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = af_conf_alloc()) != NULL)
    {
        CONF_ENTRY(mem_ptr)->status = status;
        CONF_ENTRY(mem_ptr)->handle = handle;
    }
}

/**************************************************************************/
/*!
    Return the first entry in the confirmation queue.
*/
/**************************************************************************/
mem_ptr_t *af_conf_pop()
{
    return list_head(af_conf_queue);
}


