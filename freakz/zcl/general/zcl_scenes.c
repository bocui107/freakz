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
    \file zcl_scenes.c
    \ingroup zcl


*/
/**************************************************************************/
#include "freakz.h"
#include "zcl_scenes.h"

LIST(scene_tbl);

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_scenes_init(zcl_scenes_attrib_list_t *attrib_list)
{
    // init the data values
    memset(&attrib_list->data, 0, sizeof(zcl_scenes_data_t));
    attrib_list->data.name_supp = ZCL_SCENES_NAME_SUPPORT;

    // init the attribute fields
    zcl_set_attrib(&attrib_list->list[0], ZCL_SCENES_ATTRIB_SCENE_COUNT,    ZCL_TYPE_U8,        ZCL_ACCESS_READ_ONLY, &attrib_list->data.scene_cnt);
    zcl_set_attrib(&attrib_list->list[1], ZCL_SCENES_ATTRIB_CURR_SCENE,     ZCL_TYPE_U8,        ZCL_ACCESS_READ_ONLY, &attrib_list->data.curr_scene);
    zcl_set_attrib(&attrib_list->list[2], ZCL_SCENES_ATTRIB_CURR_GROUP,     ZCL_TYPE_U16,       ZCL_ACCESS_READ_ONLY, &attrib_list->data.curr_grp);
    zcl_set_attrib(&attrib_list->list[3], ZCL_SCENES_ATTRIB_SCENE_VALID,    ZCL_TYPE_BOOL,      ZCL_ACCESS_READ_ONLY, &attrib_list->data.scene_valid);
    zcl_set_attrib(&attrib_list->list[4], ZCL_SCENES_ATTRIB_NAME_SUPP,      ZCL_TYPE_8BITMAP,   ZCL_ACCESS_READ_ONLY, &attrib_list->data.name_supp);
    zcl_set_attrib(&attrib_list->list[5], ZCL_SCENES_ATTRIB_LAST_CONFIG_BY, ZCL_TYPE_IEEE_ADDR, ZCL_ACCESS_READ_ONLY, &attrib_list->data.last_cfg_by);
    zcl_set_attrib(&attrib_list->list[6], ZCL_END_MARKER,                   0,                  0,                    NULL);

    // init the table
    list_init(scene_tbl);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static mem_ptr_t *zcl_scenes_tbl_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(zcl_scenes_entry_t)) != NULL)
    {
        list_add(scene_tbl, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void zcl_scenes_tbl_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(scene_tbl, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static mem_ptr_t *zcl_scenes_tbl_find(U16 grp_id, U8 scene_id)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(scene_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (((ZCL_SCENES_ENTRY(mem_ptr)->grp_id == grp_id) && (ZCL_SCENES_ENTRY(mem_ptr)->id == scene_id))
        {
            break;
        }
    }
    return mem_ptr;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
U8 zcl_scenes_tbl_add(zcl_scenes_entry_t *entry)
{
    mem_ptr_t *mem_ptr;

    if (entry)
    {
        // check to see if the group exists in the group table
        if (!aps_grp_exists(entry->grp_id)
        {
            return ZCL_STATUS_INVALID_FIELD;
        }

        // check to see if the scene exists
        if ((mem_ptr = zcl_scenes_tbl_find(entry->grp_id, entry->id)) != NULL)
        {
            // the scene already exists. replace it.
            memcpy(ZCL_SCENES_ENTRY(mem_ptr), entry, sizeof(zcl_scenes_entry_t));
        }
        else
        {
            // the scene doesn't exist. alloc a new entry for it and copy the contents into it.
            if ((mem_ptr = zcl_scenes_tbl_alloc()) != NULL)
            {
                memcpy(ZCL_SCENES_ENTRY(mem_ptr), entry, sizeof(zcl_scenes_entry_t));
            }
        }
        return ZCL_STATUS_SUCCESS;
    }
    return ZCL_STATUS_FAIL;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
U8 zcl_scenes_tbl_rem(U16 grp_id, U8 scene_id)
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = zcl_scenes_tbl_find(scene_id)) != NULL)
    {
        if (ZCL_SCENES_ENTRY(mem_ptr)->grp_id == grp_id)
        {
            zcl_scenes_tbl_free(mem_ptr);
            return ZCL_STATUS_SUCCESS;
        }
    }
    return ZCL_STATUS_NOT_FOUND;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static U8 zcl_scenes_tbl_clear(U16 grp_id)
{
    mem_ptr_t *mem_ptr;
    status = ZCL_STATUS_INVALID_FIELD;

    for (mem_ptr = list_chop(scene_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (ZCL_SCENES_ENTRY(mem_ptr)->grp_id == grp_id)
        {
            zcl_scenes_tbl_free(mem_ptr);
            status = ZCL_STATUS_SUCCESS;
        }
    }
    return status;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_scenes_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    U8 status, scene_cnt, len, *data_ptr, *resp_ptr, *list_ptr;
    zcl_scenes_entry_t entry;
    zcl_hdr_t resp_hdr;
    mem_ptr_t *mem_ptr;

    // init the pointers
    data_ptr = hdr->payload;
    resp_ptr = resp;
    scene_cnt = 0;

    // start out by copying the common scene info
    entry.grp_id = *(U16 *)data_ptr;
    data_ptr += sizeof(U16);

    // fill out the common parts of the response
    resp_hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    resp_hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_CLI;
    resp_hdr.frm_ctrl.manuf_spec     = false;
    resp_hdr.frm_ctrl.dis_def_resp   = true;
    resp_hdr.seq_num                 = hdr->seq_num;

    // handle the command
    switch (hdr->cmd)
    {
    case ZCL_SCENES_CMD_ADD:
        entry.id = *data_ptr++;
        entry.trans_time = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);
        
        // copy name only if its less than the max name size
        if ((len = strlen(data_ptr)) < ZCL_SCENES_NAME_MAX_LEN)
        {
            memcpy(entry.name, data_ptr, len);
            data_ptr += len;
        }

        // get the remaining len of the payload
        len = hdr->payload_len - (data_ptr - hdr->payload);

        // limit the max len of the extension fields
        entry.ext_len = (len > ZCL_SCENES_EXT_MAX_LEN) ? ZCL_SCENES_EXT_MAX_LEN : len; 
        if (entry.ext_len)
        {
            memcpy(entry.ext_field, data_ptr, entry.ext_len);
            data_ptr += entry.ext_len;
        }
        
        // add to the scene table
        status = zcl_scenes_tbl_add(entry);

        // gen response hdr
        resp_hdr.cmd = ZCL_SCENES_CMD_ADD_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // gen response payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = entry.id;
        break;

    case ZCL_SCENES_CMD_VEIW:
        entry.id = *data_ptr++;
        
        // gen response hdr
        resp_hdr.cmd = ZCL_SCENES_CMD_VIEW_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        if ((mem_ptr = zcl_scenes_tbl_find(entry.grp_id, entry.id)) == NULL)
        {
            // scene not found. generate an error response and break out of the switch
            *resp_ptr++ = ZCL_STATUS_NOT_FOUND;
            *(U16 *)resp_ptr = entry.grp_id;
            resp_ptr += sizeof(U16);
            *resp_ptr++ = entry.id;
            break;
        }

        // scene found. generate success response
        *resp_ptr++ = ZCL_STATUS_SUCCESS;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = entry.id;
        *(U16 *)resp_ptr = ZCL_SCENES_ENTRY(mem_ptr)->trans_time;
        resp_ptr += sizeof(U16);
        
        // copy in the name
        len = strlen(ZCL_SCENES_ENTRY(mem_ptr)->name);
        memcpy(resp_ptr, ZCL_SCENES_ENTRY(mem_ptr)->name, len);
        resp_ptr += len;

        // copy in the extension field
        len = ZCL_SCENES_ENTRY(mem_ptr)->ext_len;
        memcpy(resp_ptr, ZCL_SCENES_ENTRY(mem_ptr)->ext_field, len);
        resp_ptr += len;
        break;
            
    case ZCL_SCENES_CMD_REM:
        entry.id = *data_ptr++;
        status = zcl_scenes_tbl_rem(entry.grp_id, entry.id);

        // gen the response hdr
        resp_hdr.cmd = ZCL_SCENES_CMD_REM_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // gen the response payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = entry.id;
        break;
             
    case ZCL_SCENES_CMD_REM_ALL:
        status = zcl_scenes_tbl_clear(entry.grp_id);

        // gen the response hdr
        resp_hdr.cmd = ZCL_SCENES_CMD_REM_ALL_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // gen the response payload
        *resp_ptr++ = status;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        break;
         
    case ZCL_SCENES_CMD_STORE:
        // need to handle this one in the action_handler
        entry.id = *data_ptr++;
        clust->action_handler(ZCL_SCENES_ACTIONS_STORE, &entry);

        // gen the response hdr
        resp_hdr.cmd = ZCL_SCENES_CMD_STORE_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        // gen the response payload
        // if the grp id doesn't exist, status is invalid. we don't get a status from the action handler
        // so this is the only way to determine success or not
        *resp_ptr++ = aps_grp_exists(entry.grp_id) ? ZCL_STATUS_SUCCESS : ZCL_STATUS_INVALID_FIELD;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = entry.id;
        break;

    case ZCL_SCENES_CMD_RECALL:
        // need to handle this one in the action handler
        entry.id = *data_ptr++;
        clust->action_handler(ZCL_SCENES_ACTION_RECALL, &entry);

        if (!hdr->frm_ctrl.dis_def_resp)
        {
            // gen the status. we don't get the status returned to us from the action handler so we have to figure 
            // it out for ourselves
            status = (zcl_scenes_tbl_find(entry.grp_id, entry.id) == NULL) ? ZCL_STATUS_NOT_FOUND : ZCL_STATUS_SUCCESS;
            len = zcl_gen_def_resp(resp_ptr, status, hdr);
            resp_ptr += len;
        }
        break;

    case ZCL_SCENES_CMD_GET_MEMB:
        list_ptr = resp_ptr + ZCL_HDR_SZ + 5;
        for (mem_ptr = list_head(scene_tbl); mem_ptr != NULL; mem_ptr = mem_ptr->next)
        {
            if (ZCL_SCENES_ENTRY(mem_ptr)->grp_id == entry.grp_id)
            {
                *list_ptr++ = ZCL_SCENES_ENTRY(mem_ptr)->id;
                scene_cnt++;
            }
        }

        resp_hdr.cmd = ZCL_SCENES_CMD_GET_MEMB_RESP;
        len = zcl_gen_hdr(resp_ptr, &resp_hdr);
        resp_ptr += len;

        *resp_ptr++ = ZCL_STATUS_SUCCESS;
        *resp_ptr++ = ZCL_SCENES_GET_MEMB_CAPACITY;
        *(U16 *)resp_ptr = entry.grp_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = scene_cnt;

        // move the resp_ptr to the same position as the list ptr. then we can calculate the payload size accurately
        resp_ptr = list_ptr;
        break;
    }
    *resp_len = resp_ptr - resp;
}

/**************************************************************************/
/*!
    Generate the scenes request commands
*/
/**************************************************************************/
U8 zcl_scenes_gen_req(U8 *data, zcl_hdr_t *hdr, zcl_scenes_entry_t *entry)
{
    U8 len, *data_ptr; 

    // gen the zcl hdr first
    data_ptr = data;
    len = zcl_gen_hdr(data_ptr, hdr);
    data_ptr += len;

    switch (hdr->cmd)
    {
    case ZCL_SCENES_CMD_ADD:
        // fill out the add scene payload
        *(U16 *)data_ptr = entry->grp_id;
        data_ptr += sizeof(U16);
        *data_ptr++ = entry->id;
        *(U16 *)data_ptr = entry->trans_time;
        data_ptr += sizeof(U16);

        // copy in the name
        len = (strlen(entry->name) > ZCL_SCENES_NAME_MAX_LEN) ? ZCL_SCENES_NAME_MAX_LEN : strlen(entry->name);
        memcpy(data_ptr, entry->name, len);
        data_ptr += len;

        // copy in the extension fields
        memcpy(data_ptr, entry->ext_field, entry->ext_len);
        data_ptr += entry->ext_len;
        break;

    case ZCL_SCENES_CMD_VIEW:
    case ZCL_SCENES_CMD_REM:
    case ZCL_SCENES_CMD_STORE:
    case ZCL_SCENES_CMD_RECALL:
        *(U16 *)data_ptr = entry->grp_id;
        data_ptr += sizeof(U16);
        *data_ptr++ = entry->id;
        break;

    case ZCL_SCENES_CMD_REM_ALL:
    case ZCL_SCENES_CMD_GET_MEMB:
        *(U16 *)data_ptr = entry->grp_id;
        data_ptr += sizeof(U16);
        break;
    }
    return data_ptr - data;
}

