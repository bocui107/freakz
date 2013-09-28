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
    \file zcl_id.c
    \ingroup zcl


*/
/**************************************************************************/
#include "freakz.h"
#include "zcl.h"
#include "zcl_id.h"

// static prototypes
static mem_ptr_t *zcl_id_tmr_alloc();
static void zcl_id_tmr_free(mem_ptr_t *mem_ptr);
static void zcl_id_tmr_add(U8 ep, U16 *time, void (*action_handler)(U8, void *));

/**************************************************************************/
/*!
    This is the timer list that's used to keep track of the time that the
    identify command will be active for each endpoint.
*/
/**************************************************************************/
LIST(id_tmr_list);

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_id_init(zcl_id_attrib_list_t *attrib_list)
{
    // init the data values first
    attrib_list->data.id_time = 0;
     
    // init the attribute fields
    zcl_set_attrib(&attrib_list->list[0], ZCL_ID_ATTRIB, ZCL_TYPE_U16, ZCL_ACCESS_READ_WRITE, &attrib_list->data.id_time);
    zcl_set_attrib(&attrib_list->list[1], ZCL_END_MARKER, 0, 0, NULL);

    // init the timer list
    list_init(id_tmr_list);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static U8 zcl_id_gen_query_resp(U8 *resp, U16 timeout, zcl_hdr_t *hdr)
{
    U8 len, *resp_ptr;
    zcl_hdr_t resp_hdr;

    resp_ptr = resp;
    
    resp_hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_CLI;
    resp_hdr.frm_ctrl.dis_def_resp   = false;
    resp_hdr.frm_ctrl.frm_type       = hdr->frm_ctrl.frm_type;
    resp_hdr.frm_ctrl.manuf_spec     = false;
    resp_hdr.cmd                     = ZCL_ID_CMD_ID_QUERY_RESP;
    resp_hdr.seq_num                 = hdr->seq_num;

    len = zcl_gen_hdr(resp_ptr, &resp_hdr);
    resp_ptr += len;

    // payload is here. 1 byte for cmd, 1 byte for status
    *(U16 *)resp_ptr = timeout;
    resp_ptr += sizeof(U16);

    return resp_ptr - resp;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_id_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    zcl_attrib_t *attrib;
    U16 timeout;
    U8 *data;

    // have the data pointer point to the payload
    data = (hdr) ? hdr->payload : NULL;

    if ((attrib = zcl_find_attrib(clust->attrib_list, ZCL_ID_ATTRIB)) == NULL)
    {
        if (!hdr->frm_ctrl.dis_def_resp)
        {
            *resp_len = zcl_gen_def_resp(resp, ZCL_STATUS_NOT_FOUND, hdr);
        }
        return;
    }

    switch (hdr->cmd)
    {
    case ZCL_ID_CMD_ID:
        // store the value of the identify timeout
        *(U16 *)attrib->data = *(U16 *)data;

        // add an id timer entry to automatically count down the id time 
        zcl_id_tmr_add(ep, (U16 *)attrib->data, clust->action_handler);
         
        if (clust->action_handler)
        {
            clust->action_handler(ZCL_ID_ACTION_ID_ON, NULL);
        }

        if (!hdr->frm_ctrl.dis_def_resp)
        {
            *resp_len = zcl_gen_def_resp(resp, ZCL_STATUS_SUCCESS, hdr); 
        }
        break;

    case ZCL_ID_CMD_ID_QUERY:
        // return the value of the identify timeout
        timeout = *(U16 *)attrib->data;
        *resp_len = zcl_id_gen_query_resp(resp, timeout, hdr);
        break;
    }
}


/**************************************************************************/
/*!

*/
/**************************************************************************/
U8 zcl_id_gen_req(U8 *data, zcl_hdr_t *hdr, U16 timeout)
{
    U8 len, *data_ptr;

    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    if (hdr->cmd == ZCL_ID_CMD_ID)
    {
        *(U16 *)data_ptr = timeout;
        data_ptr += sizeof(U16);
    }
    return data_ptr - data; 
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static mem_ptr_t *zcl_id_tmr_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(zcl_id_tmr_t))) != NULL)
    {
        list_add(id_tmr_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void zcl_id_tmr_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(id_tmr_list, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void zcl_id_tmr_add(U8 ep, U16 *time, void (*action_handler)(U8, void *))
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = zcl_id_tmr_alloc()) != NULL)
    {
        ZCL_ID_TMR(mem_ptr)->ep = ep;
        ZCL_ID_TMR(mem_ptr)->time = time;
        ZCL_ID_TMR(mem_ptr)->action_handler = action_handler;
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
mem_ptr_t *zcl_id_get_head()
{
    return list_head(id_tmr_list);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_id_tmr_periodic()
{
    mem_ptr_t *mem_ptr;
    U16 time;

    for (mem_ptr = list_head(id_tmr_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        // de-reference the time value
        time = *ZCL_ID_TMR(mem_ptr)->time; 

        // check if the timeout occurred. if it didn't then decrement the time and update
        // the id attribute
        if (time == 0)
        {
            ZCL_ID_TMR(mem_ptr)->action_handler(ZCL_ID_ACTION_ID_OFF, NULL);
            zcl_id_tmr_free(mem_ptr);
        }
        else
        {
            time--;
            *ZCL_ID_TMR(mem_ptr)->time = time;
        }
    }
}
