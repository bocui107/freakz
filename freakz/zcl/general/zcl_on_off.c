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
    \file zcl_on_off.c 
    \ingroup zcl
    \brief ZCL on/off cluster library

        This file contains the cluster initialization and handling for the
        ZCL On/Off cluster.
*/
/******************************************************************/
#include "freakz.h"
#include "zcl_on_off.h"

/**************************************************************************/
/*!
        Init the on/off cluster.
*/
/**************************************************************************/
void zcl_on_off_init(zcl_on_off_attrib_list_t *attrib_list)
{
    // init the data
    attrib_list->data.on_off = false;

    // init the attribs
    zcl_set_attrib(&attrib_list->list[0], ZCL_ON_OFF_ATTRIB, ZCL_TYPE_BOOL, ZCL_ACCESS_READ_WRITE, &attrib_list->data.on_off);
    zcl_set_attrib(&attrib_list->list[1], ZCL_END_MARKER, 0, 0, NULL);
}

/**************************************************************************/
/*!
        Handle the incoming frames targeted to the on/off cluster. This function
        processes the on/off cluster commands and sets the status to on, off, or
        toggles the value. The action handler will also handle any user defined
        actions that is related to the status.
*/
/**************************************************************************/
void zcl_on_off_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    zcl_attrib_t *attrib;
    U8 action;
    zcl_hdr_t resp_hdr;
    bool tmp;
    
    // first, get the attribute
    if ((attrib = zcl_find_attrib(clust->attrib_list, ZCL_ON_OFF_ATTRIB)) == NULL)
    {
        return;
    }
    
    switch (hdr->cmd)
    {
    case ZCL_ON_OFF_CMD_OFF:
            *(bool *)attrib->data = 0;
            break;
    case ZCL_ON_OFF_CMD_ON:
            *(bool *)attrib->data = 1;
            break;
    case ZCL_ON_OFF_CMD_TOGGLE:
            tmp = *(bool *)attrib->data;
            *(bool *)attrib->data = tmp ? 0 : 1;
            break;
    }

    // implement any user actions that are needed
    if (clust->action_handler)
    {
        // use the enumerations of the command for the action value
        action = hdr->cmd;
        clust->action_handler(action, NULL);
    }

    // send the default response
    if (!hdr->frm_ctrl.dis_def_resp)
    {
        *resp_len = zcl_gen_def_resp(resp, ZCL_STATUS_SUCCESS, &resp_hdr); 
    }
}
    
/**************************************************************************/
/*!
    Generate a cluster specific request for the on/off cluster
*/
/**************************************************************************/
U8 zcl_on_off_gen_req(U8 *data, zcl_hdr_t *hdr)
{
    U8 len, *data_ptr;

    data_ptr = data;

    // gen the header
    len = zcl_gen_hdr(data_ptr, hdr);
    data_ptr += len;

    // return the len
    return data_ptr - data;
}
