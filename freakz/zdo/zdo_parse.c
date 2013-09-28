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
    \file zdo_parse.c
    \ingroup zdo
    \brief ZDO frame parser

        Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the parsing of the ZDO header.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        Parse the incoming ZDO raw data request frame and fill out the request 
        structure that was passed into it. Return the status of the request 
        operation.
*/
/**************************************************************************/
U8 zdo_parse_req(U16 addr, U8 *data_in, U16 clust, zdo_req_t *req)
{
    U8 *data = data_in;

    req->clust = clust;
    req->seq = *data++;
    
    switch (clust)
    {
    case NWK_ADDR_REQ_CLUST:
        req->type.nwk_addr.ext_addr = *(U64 *)data; 
        data += sizeof(U64);
        req->type.nwk_addr.req_type = *data++;
        req->type.nwk_addr.start_index = *data++;      
        break;

    case IEEE_ADDR_REQ_CLUST:   
        req->type.ieee_addr.nwk_addr = *(U16 *)data; 
        data += sizeof(U16);
        req->type.ieee_addr.req_type = *data++;
        req->type.ieee_addr.start_index = *data++;   
        break;

    case NODE_DESC_REQ_CLUST:      
    case PWR_DESC_REQ_CLUST:       
    case ACTIVE_EP_REQ_CLUST:      
        req->type.node_desc.nwk_addr = *(U16 *)data; 
        data += sizeof(U16);
        break;

    case SIMPLE_DESC_REQ_CLUST:    
        req->type.simple_desc.nwk_addr = *(U16 *)data; 
        data += sizeof(U16);
        req->type.simple_desc.ep = *data++;
        break;

    case DEV_ANNCE_REQ_CLUST:
        req->type.dev_annce.nwk_addr = *(U16 *)data; 
        data += sizeof(U16);      
        req->type.dev_annce.ext_addr = *(U64 *)data; 
        data += sizeof(U64);
        req->type.dev_annce.capab_info = *data++;
        break;

    case NWK_DISC_REQ_CLUST:       
        req->type.nwk_disc.scan_channels = *(U32 *)data; 
        data += sizeof(U32);
        req->type.nwk_disc.scan_duration = *data++;
        req->type.nwk_disc.start_index = *data++;
        break;

    case NWK_LQI_REQ_CLUST:
        req->type.nwk_lqi.start_index = *data++;
        break;

    case NWK_LEAVE_REQ_CLUST:
        req->type.leave.addr = *(U64 *)data;
        data += sizeof(U64);
        req->type.leave.rem_children = (*data >> 1) & 0x1;
        req->type.leave.rejoin = *data & 0x1;
        data++;
        break;

    case NWK_PERMIT_JOIN_REQ_CLUST:
        req->type.permit_join.duration = *data++;
        req->type.permit_join.tc_sig = *data++;
        break;        

    case NWK_UPDATE_REQ_CLUST:     
        req->type.nwk_update.scan_channels = *(U32 *)data; 
        data += sizeof(U32);
        req->type.nwk_update.scan_duration = *data++;

        if (req->type.nwk_update.scan_duration == 0xFE)
        {
            req->type.nwk_update.nwk_update_id = *data++; 
        }
        else if (req->type.nwk_update.scan_duration == 0xFF)
        {
            req->type.nwk_update.nwk_update_id = *data++;
            req->type.nwk_update.nwk_mngr_addr = *(U16 *)data; 
            data += sizeof(U16);
        }
        else if (req->type.nwk_update.scan_duration <= 5)
        {
            req->type.nwk_update.scan_cnt = *data++;
        }
        break;

    case END_DEV_BIND_REQ_CLUST:
        req->type.ed_bind.target = *(U16 *)data;
        data += sizeof(U16);
        req->type.ed_bind.src_ext_addr = *(U64 *)data;
        data += sizeof(U64);
        req->type.ed_bind.src_ep = *data++;
        req->type.ed_bind.prof_id = *(U16 *)data;
        data += sizeof(U16);
        req->type.ed_bind.num_in_clust = *data++;
        memcpy(req->type.ed_bind.in_clust, data, 2 * req->type.ed_bind.num_in_clust);
        data += 2 * req->type.ed_bind.num_in_clust;
        req->type.ed_bind.num_out_clust = *data++;
        memcpy(req->type.ed_bind.out_clust, data, 2 * req->type.ed_bind.num_out_clust);
        data += 2 * req->type.ed_bind.num_out_clust;
        break;

    case BIND_REQ_CLUST:
    case UNBIND_REQ_CLUST:
        req->type.bind.src_addr = *(U64 *)data; 
        data += sizeof(U64);
        req->type.bind.src_ep = *data++;
        req->type.bind.clust = *(U16 *)data; 
        data += sizeof(U16);
        req->type.bind.dest_addr.mode = *data++;

        if (req->type.bind.dest_addr.mode == BIND_GRP_ADDR)
        {
            req->type.bind.dest_addr.short_addr = *(U16 *)data; 
            data += sizeof(U16);
        }
        else if (req->type.bind.dest_addr.mode == BIND_EXT_ADDR)
        {
            req->type.bind.dest_addr.long_addr = *(U64 *)data;
            data += sizeof(U64);
            req->type.bind.dest_ep = *data++;
        }
        break;
    default:
        return AF_INV_REQUESTTYPE;
    }
    debug_dump_zdo_request(req);
    return AF_SUCCESS;
}

/**************************************************************************/
/*!
        Parse the raw data response frame and fill out the response structure that
        was passed into the frame. Return the status of the response parsing 
        operation.
*/
/**************************************************************************/
U8 zdo_parse_resp(U8 *data_in, U16 clust, zdo_resp_t *resp)
{
    U8 *data = data_in;

    resp->seq = *data++;
    resp->clust = clust;
    switch (clust)
    {
    case END_DEV_BIND_RESP_CLUST:
    case BIND_RESP_CLUST:
    case UNBIND_RESP_CLUST:
        resp->type.bind.status = *data++;
        break;

    case NWK_LQI_RESP_CLUST:
        resp->type.nwk_lqi.status           = *data++;
        resp->type.nwk_lqi.nbor_tbl_cnt     = *data++;
        resp->type.nwk_lqi.start_index      = *data++;
        resp->type.nwk_lqi.nbor_list_cnt    = *data++;
        break;

    case NWK_DISC_RESP_CLUST:
        resp->type.nwk_disc.status          = *data++;
        resp->type.nwk_disc.nwk_cnt         = *data++;
        resp->type.nwk_disc.start_index     = *data++;
        resp->type.nwk_disc.nwk_list_cnt    = *data++;
        break;

    case NWK_LEAVE_RESP_CLUST:
        resp->type.leave.status             = *data++;
        break;

    case NWK_UPDATE_RESP_CLUST:
        resp->type.nwk_update.status        = *data++;
        resp->type.nwk_update.scan_channels = *(U32 *)data;
        data += sizeof(U32);
        resp->type.nwk_update.total_xmits   = *(U16 *)data;
        data += sizeof(U16);
        resp->type.nwk_update.xmit_failures = *(U16 *)data;
        data += sizeof(U16);
        resp->type.nwk_update.scan_channel_list_cnt = *data++;
        break;

    default:
        return AF_NOT_SUPPORTED;
    }
    debug_dump_zdo_resp(resp, data);
    return AF_SUCCESS;
}

