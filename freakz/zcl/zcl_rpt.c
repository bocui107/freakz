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
    \file zcl_rpt.c
    \ingroup zcl
    \brief ZCL attribute reporting

    This file contains functions to implement the Zigbee Cluster Library
    reporting feature. The reporting entries get put into a reporting table.
        The table gets polled once per second by the slow clock and each entry's
        expiry gets decremented. If an entry has an expiry of zero, and it's report
        flag is triggered, a report frame will get sent. Only one attribute per frame
        is currently supported.
*/
/**************************************************************************/
#include "freakz.h"
#include "zcl.h"

/**************************************************************************/
/*!
    List head for the ZCL attribute reporting table. All reports are configured
    as an entry and put inside this table. It will get called by the slow
    clock once per second and any attributes that need processing will be handled.
*/
/**************************************************************************/
LIST(zcl_rpt);

/**************************************************************************/
/*!
        Initialize the reporting table.
*/
/**************************************************************************/
void zcl_rpt_init()
{
    list_init(zcl_rpt);
}

/**************************************************************************/
/*!
        Allocate a memory block to hold the reporting entry and add it to the table.
        A handle to the memory block will be returned.
*/
/**************************************************************************/
static mem_ptr_t *zcl_rpt_alloc()
{
    mem_ptr_t *mem_ptr;
    if ((mem_ptr = mem_heap_alloc(sizeof(zcl_rpt_entry_t))) != NULL)
    {
        list_add(zcl_rpt, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
        Free the reporting entry and remove it from the table. It will also remove
        the entry from the attribute.
*/
/**************************************************************************/
static void zcl_rpt_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        // Nullify the report pointer in the attrib
        ZCL_RPT(mem_ptr)->attrib->rpt = NULL;

        // free the mem pointer
        list_remove(zcl_rpt, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!
        Find the reporting entry based on the addr, ep, cluster ID, and attribute ID. It will
        return a memory handle containing the entry.
*/
/**************************************************************************/
static mem_ptr_t *zcl_rpt_find(U16 src_addr, U8 ep, U16 clust, zcl_attrib_t *attrib)
{
    mem_ptr_t *mem_ptr;

    // first check to see if the reporting entry already exists
    for (mem_ptr = list_head(zcl_rpt); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // if we get a match, then just use that as the reporting entry.
        if ((ZCL_RPT(mem_ptr)->addr     == src_addr)    &&
            (ZCL_RPT(mem_ptr)->ep       == ep)          &&
            (ZCL_RPT(mem_ptr)->attrib   == attrib))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!
        Add a new reporting entry to the table. First alloc a memory block to contain
        the entry and then fill out the entry with the necessary parameters.
*/
/**************************************************************************/
U8 zcl_rpt_add(U16 src_addr, U8 src_ep, U16 clust_id, zcl_attrib_t *attrib, U16 prof_id, U16 timeout, U32 change)
{
    mem_ptr_t *mem_ptr = NULL;

    // if the timeout value is 0xffff, then we will just free the report since
    // that means that the value will not be reported. Also, return if there is no
    // attrib
    if (!attrib)
    {
        return ZCL_STATUS_UNSUP_ATTRIB;
    }

    if (timeout == 0xffff)
    {
        return ZCL_STATUS_SUCCESS;
    }

    if ((mem_ptr = zcl_rpt_find(src_addr, src_ep, clust_id, attrib)) == NULL)
    {
        if ((mem_ptr = zcl_rpt_alloc()) == NULL)
        {
            return ZCL_STATUS_INSUFF_SPACE;
        }
    }

    // fill out the report entry
    ZCL_RPT(mem_ptr)->addr          = src_addr;
    ZCL_RPT(mem_ptr)->ep            = src_ep;
    ZCL_RPT(mem_ptr)->clust         = clust_id;
    ZCL_RPT(mem_ptr)->attrib        = attrib;
    ZCL_RPT(mem_ptr)->prof_id       = prof_id;
    ZCL_RPT(mem_ptr)->timeout       = timeout;
    ZCL_RPT(mem_ptr)->expiry        = timeout;
    ZCL_RPT(mem_ptr)->rpt_needed    = false;
    ZCL_RPT(mem_ptr)->change        = change;

    // add a reference to the attribute to point to this entry
    attrib->rpt                     = mem_ptr;
    return ZCL_STATUS_SUCCESS;
}

/**************************************************************************/
/*!
        Remove a reporting entry from the table. This function will first find the
        particular entry and then free it.
*/
/**************************************************************************/
U8 zcl_rpt_remove(U16 src_addr, U8 src_ep, U16 clust, zcl_attrib_t *attrib)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = zcl_rpt_find(src_addr, src_ep, clust, attrib)) != NULL)
    {
        zcl_rpt_free(mem_ptr);
        return ZCL_STATUS_SUCCESS;
    }
    return ZCL_STATUS_NOT_FOUND;
}

/**************************************************************************/
/*!
    This sends a report of the specified attribute. We only send one attribute
    per report currently.
*/
/**************************************************************************/
static void zcl_rpt_send_rpt(mem_ptr_t *mem_ptr)
{
    U8 ep, len, *rpt_ptr, rpt[ZCL_MAX_RPT_SZ];
    U16 addr, clust, prof_id;
    U32 change; 
    zcl_hdr_t hdr;
    bool rpt_needed;
    zcl_attrib_t *attrib;

    change      = ZCL_RPT(mem_ptr)->change;
    rpt_needed  = ZCL_RPT(mem_ptr)->rpt_needed;
    attrib      = ZCL_RPT(mem_ptr)->attrib;
    ep          = ZCL_RPT(mem_ptr)->ep;
    addr        = ZCL_RPT(mem_ptr)->addr;
    clust       = ZCL_RPT(mem_ptr)->clust;
    prof_id     = ZCL_RPT(mem_ptr)->prof_id;

    // no need to send a report if the change value is greater than zero and 
    // the report needed flag is false. that means that no change larger than
    // the min change value occurred.
    if (change && !rpt_needed)
    {
        return;
    }

    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_CLI;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.seq_num                 = zcl_get_seq_num();
    hdr.cmd                     = ZCL_CMD_REPORT_ATTRIB;

    rpt_ptr = rpt;
    len = zcl_gen_hdr(rpt_ptr, &hdr);
    rpt_ptr += len;

    *(U16 *)rpt_ptr = attrib->id;
    rpt_ptr += sizeof(U16);
    *rpt_ptr++ = attrib->type;

    switch (attrib->type)
    {
    case ZCL_TYPE_BOOL:
    case ZCL_TYPE_8BIT:
    case ZCL_TYPE_U8:
        *rpt_ptr++ = *(U8 *)attrib->data;
        break;

    case ZCL_TYPE_16BIT:
    case ZCL_TYPE_U16:
        *(U16 *)rpt_ptr = *(U16 *)attrib->data;
        rpt_ptr += sizeof(U16);
        break;

    case ZCL_TYPE_32BIT:
    case ZCL_TYPE_U32:
        *(U32 *)rpt_ptr = *(U32 *)attrib->data;
        rpt_ptr += sizeof(U32);
        break;
    default:
        return;
    }
    len = rpt_ptr - rpt;

    af_tx(rpt, len, ep, addr, ep, clust, prof_id, APS_DEST_ADDR_16_EP_PRESENT, 0, 6, af_handle_get()); 
}

/**************************************************************************/
/*!
    This is a periodic function that is called by the slow clock once per second.
    When called, it will decrement the expiry. If the expiry is zero, then it
    will send a report containing that attribute.
*/
/**************************************************************************/
void zcl_rpt_periodic()
{
    mem_ptr_t *mem_ptr;

    // scroll through list and decrement the expiry. 
    for (mem_ptr = list_head(zcl_rpt); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (ZCL_RPT(mem_ptr)->expiry == 0)
        {
            // expiry is zero. send a report.
            zcl_rpt_send_rpt(mem_ptr);

            // reset the expiry
            ZCL_RPT(mem_ptr)->expiry = ZCL_RPT(mem_ptr)->timeout;
        }
        else
        {
            // just decrement the expiry
            ZCL_RPT(mem_ptr)->expiry--;
        }
    }
}
