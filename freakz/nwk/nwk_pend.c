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
    \file nwk_pend.c
    \ingroup nwk
    \brief NWK pending queue

    Implements the pending list. When a buffer will be routed by mesh routing,
    but the route doesn't exist yet, then route discovery needs to be performed.
    During the time that route discovery is going on, the frame will be stored
    in the pending list. Once a route is located, then the frame will be forwarded
    along the discovered route. If the route discovery takes too long or no route
    is discovered, then the pending list entry will expire and the buffer will
    be freed.
*/
#include "freakz.h"

/*
 * List head for the pending list. The pending list is used to buffer a frame
 * that arrives but is not targeted towards us. If we need to forward it and
 * we don't know where the destination is, then we need to do a route discovery
 * which takes time. So while the route discovery is taking place, we buffer
 * the frame here.
 */
LIST(pend_list);

/* Init the pending list */
void nwk_pend_init()
{
	list_init(pend_list);
}

/*
 * Find a free pend entry and add it to the pending list. Once added, return
 * a pointer to it. If no free entry is found, return NULL.
 */
static mem_ptr_t *nwk_pend_alloc()
{
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = mem_heap_alloc(sizeof(nwk_pend_t))) != NULL)
	{
		list_add(pend_list, mem_ptr);
	}
	return mem_ptr;
}

/* Remove the entry from the pending list and free it */
void nwk_pend_free(mem_ptr_t *mem_ptr)
{
	if (mem_ptr)
	{
		list_remove(pend_list, mem_ptr);
		mem_heap_free(mem_ptr);
	}
}

/* Remove all entries from the pending list */
void nwk_pend_clear()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_chop(pend_list); mem_ptr != NULL; mem_ptr = list_chop(pend_list))
	{
		buf_free(PEND_ENTRY(mem_ptr)->buf);
		nwk_pend_free(mem_ptr);
	}
}

/*
 * Add a new entry to the pending list with the specified buffer and header
 * information.
 */
void nwk_pend_add_new(buffer_t *buf, nwk_hdr_t *hdr)
{
	mem_ptr_t *mem_ptr;

	if ((mem_ptr = nwk_pend_alloc()) != NULL)
	{
		PEND_ENTRY(mem_ptr)->buf = buf;
		memcpy(&PEND_ENTRY(mem_ptr)->frm_ctrl,
		       &hdr->nwk_frm_ctrl,
		       sizeof(nwk_frm_ctrl_t));
		PEND_ENTRY(mem_ptr)->src_addr     = hdr->src_addr;
		PEND_ENTRY(mem_ptr)->dest_addr    = hdr->dest_addr;
		PEND_ENTRY(mem_ptr)->radius       = hdr->radius;
		PEND_ENTRY(mem_ptr)->seq          = hdr->seq_num;
		PEND_ENTRY(mem_ptr)->handle       = hdr->handle;
		PEND_ENTRY(mem_ptr)->expiry       = NWK_PEND_TIMEOUT;
	}
}

/* Send out any transfers in the pending list if the rte table entry exists */
void nwk_pend_send_pending()
{
	mem_ptr_t *mem_ptr;

	/*
	 * route discovery is completed. check the pending queue for
	 * any frames that need forwarding now that we have a new route.
	 */
	for (mem_ptr = list_head(pend_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		/*
		 * go through all of the entries in the pending list
		 * and check if they can be forwarded.
		 */
		if (nwk_rte_tbl_rte_exists(PEND_ENTRY(mem_ptr)->dest_addr))
		{
			nwk_hdr_t hdr_out;

			hdr_out.src_addr    = PEND_ENTRY(mem_ptr)->src_addr;
			hdr_out.dest_addr   = PEND_ENTRY(mem_ptr)->dest_addr;
			hdr_out.radius      = PEND_ENTRY(mem_ptr)->radius;
			hdr_out.handle      = PEND_ENTRY(mem_ptr)->handle;
			hdr_out.seq_num     = PEND_ENTRY(mem_ptr)->seq;
			memcpy(&hdr_out.nwk_frm_ctrl,
				&PEND_ENTRY(mem_ptr)->frm_ctrl,
				sizeof(nwk_frm_ctrl_t));

			nwk_fwd(PEND_ENTRY(mem_ptr)->buf, &hdr_out);
			nwk_pend_free(mem_ptr);
		}
	}
}

/*
 * Periodic function. This function gets called once per second and will
 * decrement the expiry in the pending queue entries. On expiration, the
 * pending entry and its buffer will be freed.
 */
void nwk_pend_periodic()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(pend_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (PEND_ENTRY(mem_ptr)->expiry == 0)
		{
			buf_free(PEND_ENTRY(mem_ptr)->buf);
			nwk_pend_free(mem_ptr);
		} else {
			PEND_ENTRY(mem_ptr)->expiry--;
		}
	}
}
