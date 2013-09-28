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
    \file af_ep.c
    \ingroup af
    \brief AF endpoint table
 
    This file contains functions for registering, finding, and removing endpoints
    from the Zigbee device. Endpoint registration works by adding the simple descriptor,
    rx callback, and conf callback to the endpoint list. It also has some
    helper functions that are used by the ZDO for descriptor processing.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the AF endpoint list. This list contains the registered endpoints for this
        device and holds the simple descriptors, as well as the endpoint callbacks
        for received frames and confirmations.
*/
/**************************************************************************/
LIST(ep_list);

/**************************************************************************/
/*!
    Initialize the af endpoint list.
*/
/**************************************************************************/
void af_ep_init()
{
    list_init(ep_list);
}

/**************************************************************************/
/*!
    Allocate a ep_entry structure for registering an endpoint. Add this element
    to the list.
*/
/**************************************************************************/
static mem_ptr_t *af_ep_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(ep_entry_t))) != NULL)
    {
        list_add(ep_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
    Free the specified ep_entry. We need to be careful when we free it though.
    The simple descriptor has some malloc'd fields that we need to free first.
    Otherwise we will have a mem leak.
*/
/**************************************************************************/
static void af_ep_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(ep_list, mem_ptr);

        // careful...order matters here. free the desc pointers first. then the struct.
        mem_heap_free(EP_ENTRY(mem_ptr)->simple_desc);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Remove all entries from the endpoint list.
*/
/**************************************************************************/
void af_ep_clear_all()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(ep_list); mem_ptr != NULL; mem_ptr = list_chop(ep_list))
    {
        af_ep_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
    Register the endpoint with the application framework. To register and endpoint
    requires the simple descriptor and the rx callback function. The rx callback
    function is so that the af knows where to send any incoming messages.
*/
/**************************************************************************/
void af_ep_add(U8 ep_num, U8 *simple_desc, U8 desc_size, bool zcl,
             void (*ep_rx)(U8 *, U8, U16, U8, U16),
             void (*ep_conf)(U8, U8))
{
    mem_ptr_t *mem_ptr;

    if (simple_desc)
    {
        if ((mem_ptr = af_ep_alloc()) != NULL)
        {
            if ((EP_ENTRY(mem_ptr)->simple_desc = mem_heap_alloc(sizeof(desc_size))) != NULL)
            {
                EP_ENTRY(mem_ptr)->ep_num = ep_num;
                EP_ENTRY(mem_ptr)->simple_desc_size = desc_size;
                EP_ENTRY(mem_ptr)->zcl = zcl;
                EP_ENTRY(mem_ptr)->ep_rx = ep_rx;
                EP_ENTRY(mem_ptr)->ep_conf = ep_conf;
                memcpy(EP_ENTRY(mem_ptr)->simple_desc->mmem_ptr.ptr, simple_desc, desc_size);
            }
        }
    }
}

/**************************************************************************/
/*!
    Find the ep entry with the specified endpoint number and return the mem pointer
    to it.
*/
/**************************************************************************/
mem_ptr_t *af_ep_find(U8 ep_num)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(ep_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (EP_ENTRY(mem_ptr)->ep_num == ep_num)
        {
            break;
        }
    }
    return (mem_ptr);
}

/**************************************************************************/
/*!
    Send the received data to the specified destination ep's rx callback function.
*/
/**************************************************************************/
void af_ep_rx(U16 dest_ep, buffer_t *buf, U8 len, U16 src_addr, U8 src_ep, U16 clust_id)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = af_ep_find(dest_ep)) != NULL)
    {
        if (EP_ENTRY(mem_ptr)->ep_rx)
        {
            EP_ENTRY(mem_ptr)->ep_rx(buf->dptr, len, src_addr, src_ep, clust_id);
        }
    }
}

/**************************************************************************/
/*!
        Send a confirmation to the specified endpoint.
*/
/**************************************************************************/
void af_ep_conf(U16 dest_ep, U8 status, U8 handle)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = af_ep_find(dest_ep)) != NULL)
    {
        if (EP_ENTRY(mem_ptr)->ep_conf)
        {
            EP_ENTRY(mem_ptr)->ep_conf(status, handle);
        }
    }
}

/**************************************************************************/
/*!
    Sort the ep list in increasing order. This function is used by the ZDO
    for the active ep descriptor and match descriptor.
*/
/**************************************************************************/
static void af_ep_sort(U8 *eps, U8 size)
{
        U8 i, j, min, tmp;

        for (i=0; i<size; i++)
        {
                // Set the min value to the highest value
                min = i;
                for (j=i; j<size; j++)
                {
                        if (eps[j] < eps[min])
                        {
                                // If we found a new min value, then replace the value at that index
                                // with the new min value.
                                tmp = eps[min];
                                eps[min] = eps[j];
                                eps[j] = tmp;
                        }
                }
        }
}

/**************************************************************************/
/*!
    Build the active endpoint list and return it along with the endpoint count.
*/
/**************************************************************************/
U8 af_ep_get_active(U8 *active_list)
{
    mem_ptr_t *mem_ptr;
    U8 max, ep_cnt = 0;
    U8 eps[ZIGBEE_MAX_ENDPOINTS];

    // get all endpoint entries and add them to the active endpoint list
    for (mem_ptr = list_head(ep_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        eps[ep_cnt] = EP_ENTRY(mem_ptr)->ep_num;
        ep_cnt++;
    }

    // sort the list in increasing order
    af_ep_sort(eps, ep_cnt);

    // copy the eps list to the incoming list up to the ZIGBEE_MAX_ENDPOINTS limit
    max = (ep_cnt > ZIGBEE_MAX_ENDPOINTS) ? ZIGBEE_MAX_ENDPOINTS : ep_cnt;
    *active_list++ = max;
    memcpy(active_list, eps, max);
    return max + 1;
}

/**************************************************************************/
/*!
    Find matching endpoints to the specified cluster lists. This is used by
    the match descriptor request handler.
*/
/**************************************************************************/
static bool af_ep_match(const mem_ptr_t *mem_ptr, U16 prof_id, const clust_list_t *in_list, const clust_list_t *out_list)
{
    U8 i, j, k;
    simple_desc_t *desc;
    clust_list_t *entry_in, *entry_out, *dev_list;
    const clust_list_t *match_list;

    //lint --e{826} Suppress Info 826: Suspicious pointer-to-pointer conversion (area too small)
    // this removes the lint warning for this block only

    // if the prof id doesn't match, then disqualify the entry
    desc = (simple_desc_t *)EP_ENTRY(mem_ptr)->simple_desc->mmem_ptr.ptr;
    if (desc->prof_id != prof_id)
    {
        return false;
    }

    // add pointers to the input clusters and output clusters
    entry_in = (clust_list_t *)desc->clust_info;
    entry_out = entry_in + 1 + (entry_in->clust_cnt * sizeof(U16));

    // compare each member of the clusters with each member of the entry's clusters.
    for (i=0; i<2; i++)
    {
        // set up the device list to point to this device's ep in/out list
        // set up the match list to point to the incoming in/out list to be matched against
        if (i == 0)
        {
            dev_list    = entry_in;
            match_list  = in_list;
        }
        else
        {
            dev_list    = entry_out;
            match_list  = out_list;
        }

        // check each cluster on the match list to see if it is the same number as any cluster on this
        // device. if there is a matching cluster number, then return true.
        for (j=0; j<match_list->clust_cnt; j++)
        {
            for (k=0; k<dev_list->clust_cnt; k++)
            {
                if (match_list->clust_list[j] == dev_list->clust_list[k])
                {
                    return true;
                }
            }
        }
    }

    // no matches
    return false;
}

/**************************************************************************/
/*!
    Looks for matching endpoints based on the profile id and the cluster lists.
*/
/**************************************************************************/
U8 af_ep_find_matches(U16 prof_id, const clust_list_t *in_list, const clust_list_t *out_list, U8 *eps)
{
    U8 i = 0;
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(ep_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (af_ep_match(mem_ptr, prof_id, in_list, out_list))
        {
            eps[i] = EP_ENTRY(mem_ptr)->ep_num;
            i++;
        }
    }
    af_ep_sort(eps, i);
    return i;
}
