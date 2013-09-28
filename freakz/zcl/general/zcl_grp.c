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
    \file zcl_grp.c
    \ingroup zcl


*/
/**************************************************************************/
#include "freakz.h"
#include "zcl_grp.h"

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_grp_init(zcl_grp_attrib_list_t *attrib_list)
{
    // init the data
    attrib_list->data.name_supp = ZCL_GRP_NAME_SUPPORT;

    // init the attribs
    zcl_set_attrib(&attrib_list->list[0], ZCL_GRP_ATTRIB_NAME_SUPPORT, ZCL_TYPE_8BITMAP, ZCL_ACCESS_READ_ONLY, &attrib_list->data.name_supp);
    zcl_set_attrib(&attrib_list->list[1], ZCL_END_MARKER, 0, 0, NULL);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_grp_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    zcl_attrib_t *attrib;
    U8 i, len, status, grp_cnt, grp_list_cnt, *data_ptr, *resp_ptr, *grp_id_list;
    U16 grp_id;
    zcl_hdr_t resp_hdr;
    mem_ptr_t *mem_ptr;

    // init the grp list cnt
    grp_list_cnt = 0;

    // init the pointers. the data ptr points to the payload, the resp ptr points to the response, and the grp id list is 5
    // bytes down. we need a separate pointer for the grp id list because it's variable in size and we're going to fill 
    // it out first to determine the grp list cnt which goes in later
    data_ptr    = hdr->payload;
    resp_ptr    = resp;
    grp_id_list = resp + ZCL_HDR_SZ + 2;     // capacity and group cnt fields are 2 bytes

    // fill out the common parts of the response header
    resp_hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    resp_hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_CLI;
    resp_hdr.frm_ctrl.manuf_spec     = false;
    resp_hdr.frm_ctrl.dis_def_resp   = true;
    resp_hdr.seq_num                 = hdr->seq_num;

    // handle the cmd
    switch (hdr->cmd)
    {
    case ZCL_GRP_CMD_ADD_GRP:
        // get the grp id and then add the group. the status will be used in the response
        grp_id = *(U16 *)data_ptr;
        status = aps_grp_add(grp_id, ep) ? ZCL_STATUS_INSUFF_SPACE : ZCL_STATUS_SUCCESS; 
        
        // gen the resp header
        resp_hdr.cmd = ZCL_GRP_CMD_ADD_GRP_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // fill out the resp payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = grp_id;
        resp_ptr += sizeof(U16);
        break;

    case ZCL_GRP_CMD_VIEW_GRP:
        // names are unsupported so mention this in the status
        grp_id = *(U16 *)data_ptr;
        status = ZCL_STATUS_UNSUP_ATTRIB;

        // fill out resp header
        resp_hdr.cmd = ZCL_GRP_CMD_VIEW_GRP_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // fill out resp payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = grp_id;
        resp_ptr += sizeof(U16);
        break;

    case ZCL_GRP_CMD_GET_MEMB:
        // get the grp cnt
        grp_cnt = *data_ptr++;

        // if the grp count equals zero, then we are supposed to dump our whole group table. if it's nonzero,
        // then that means that we need to match up the grp ids in the list with identical ids in our table.
        if (grp_cnt == 0)
        {
            // there isn't a grp list. we just dump our whole grp table and send it out in the response.
            for (mem_ptr = aps_grp_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
            {
                // if we hit the max list size, then don't add any more ids. we don't want to overflow
                // the buffer.
                if (grp_list_cnt < ZCL_GRP_MAX_LIST_SZ)
                {
                    *(U16 *)grp_id_list = GROUP_ENTRY(mem_ptr)->id;
                    grp_id_list += sizeof(U16);
                    grp_list_cnt++;
                }
            }
        }
        else
        {
            // there is a grp list so we need to match up the grp ids with any ids in our table.
            // loop through the grp cnt, get the grp id in the list, and check to see if 
            // it's in our grp tables. 
            for (i=0; i<grp_cnt; i++)
            {
                // get the next grp id in the list
                grp_id = *(U16 *)data_ptr;
                data_ptr += sizeof(U16);
                
                // look in the group table to see if the grp id exists
                for (mem_ptr = aps_grp_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
                {
                    if (grp_id == GROUP_ENTRY(mem_ptr)->id)
                    {
                        // limit the grp list size to prevent overflow
                        if (grp_list_cnt < ZCL_GRP_MAX_LIST_SZ)
                        {
                            // grp id exists in our table. add it to the grp id list in the response
                            // and increment the grp list cnt
                            *(U16 *)grp_id_list = grp_id;
                            grp_id_list += sizeof(U16); 
                            grp_list_cnt++;
                        }
                    }
                }
            }
        }
        // generate the response header
        resp_hdr.cmd = ZCL_GRP_CMD_GET_MEMB_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // generate the response payload. the grp id list should come directly after the grp list cnt
        *resp_ptr++ = ZCL_GRP_CAPACITY_AVAIL;       // set the capacity 
        *resp_ptr++ = grp_list_cnt;                 // set the grp list cnt

        // move the resp ptr to the same position as the grp_id_list pointer. we need to do this to accurately
        // calculate the payload size
        resp_ptr = grp_id_list;
        break;

    case ZCL_GRP_CMD_REM_GRP:
        grp_id = *(U16 *)data_ptr;
        status = aps_grp_rem(grp_id, ep) ? ZCL_STATUS_NOT_FOUND : ZCL_STATUS_SUCCESS;

        // generate the response header
        resp_hdr.cmd = ZCL_GRP_CMD_REM_GRP_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // generate the response payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = grp_id;
        resp_ptr += sizeof(U16);
        break;

    case ZCL_GRP_CMD_REM_ALL:
        // just wipe out the group table
        aps_grp_rem_all();

        // send the default response if it's not disabled in the hdr
        if (!hdr->frm_ctrl.dis_def_resp)
        {
            len = zcl_gen_def_resp(resp_ptr, ZCL_STATUS_SUCCESS, hdr);
            resp_ptr += len;
        }
        break;

    case ZCL_GRP_CMD_ADD_IF_ID:
        // get the grp id. the name will be ignored.
        grp_id = *(U16 *)data_ptr;

        // add all entries in the identify list to the group
        for (mem_ptr = zcl_id_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
        {
            aps_grp_add(grp_id, ZCL_ID_TMR(mem_ptr)->ep);
        }

        // send the default response if it's not disabled in the hdr
        if (!hdr->frm_ctrl.dis_def_resp)
        {
            len = zcl_gen_def_resp(resp_ptr, ZCL_STATUS_SUCCESS, hdr);
            resp_ptr += len;
        }
        break;
    }
    // calculate the resp len here
    *resp_len = resp_ptr - resp;
}
