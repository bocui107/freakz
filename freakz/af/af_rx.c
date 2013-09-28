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
    \file af_rx.c
    \ingroup af
    \brief AF rx queue

    These functions implement the RX queue for the Application Framework.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the AF RX queue. Mainly used to provide a break in the receive path so
        that the MCU won't get hogged processing the rx frame. Otherwise, since
        I have no control over the application code that processes the incoming
        frame, it could end up using up a lot of MCU time and causing problems.
*/
/**************************************************************************/
LIST(af_rx_queue);

/**************************************************************************/
/*!
    Init the af_rx queue.
*/
/**************************************************************************/
void af_rx_init()
{
    list_init(af_rx_queue);
}

/**************************************************************************/
/*!
    Find a free queue element and add it to the RX queue.
*/
/**************************************************************************/
static mem_ptr_t *af_rx_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(af_rx_entry_t))) != NULL)
    {
        list_add(af_rx_queue, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Remove the entry from the RX queue and free it.
*/
/**************************************************************************/
void af_rx_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        if (RX_ENTRY(mem_ptr)->buf)
        {
            buf_free(RX_ENTRY(mem_ptr)->buf);
        }
        list_remove(af_rx_queue, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Allocate and add an entry to the RX queue. Fill it out with the specified
    arguments.
*/
/**************************************************************************/
void af_rx_add(buffer_t *buf, const aps_hdr_t *hdr)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = af_rx_alloc()) != NULL)
    {
        RX_ENTRY(mem_ptr)->buf      = buf;
        RX_ENTRY(mem_ptr)->src_addr = hdr->src_addr;
        RX_ENTRY(mem_ptr)->src_ep   = hdr->src_ep;
        RX_ENTRY(mem_ptr)->dest_ep  = hdr->dest_ep;
        RX_ENTRY(mem_ptr)->clust_id = hdr->clust_id;
        RX_ENTRY(mem_ptr)->grp_id   = hdr->grp_addr;
        RX_ENTRY(mem_ptr)->grp_mode = (hdr->aps_frm_ctrl.delivery_mode == APS_GROUP);
    }
}

/**************************************************************************/
/*!
        Pop the first entry off the rx queue and return it.
*/
/**************************************************************************/
mem_ptr_t *af_rx_pop()
{
    return list_head(af_rx_queue);
}
