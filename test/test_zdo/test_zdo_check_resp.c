/*******************************************************************
    Copyright (C) 2008 FreakLabs

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Originally written by Christopher Wang.
    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title: test_zdo_check_resp.c

    Description:
*******************************************************************/
#include "freakz.h"
#include "test_zdo.h"

static zdo_resp_t resp_exp;
static U8 resp_data[MAX_RESP_DATA_SIZE];
static U8 resp_data_size;

/**************************************************************************
Function Name: test_zdo_check_addr_resp

Description:

**************************************************************************/
void test_zdo_check_addr_resp(buffer_t *buf)
{
    zdo_resp_t *resp;

    resp = (zdo_resp_t *)buf->dptr;
    assert(resp->addr.nwk_addr_rmt == resp_exp.addr.nwk_addr_rmt);
    assert(resp->addr.ext_addr_rmt == resp_exp.addr.ext_addr_rmt);
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_ADDR_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_check_node_resp

Description:

**************************************************************************/
void test_zdo_check_node_desc_resp(buffer_t *buf)
{
    zdo_resp_t *resp;

    resp = (zdo_resp_t *)buf->dptr;
    assert(resp->node_desc.status == resp_exp.node_desc.status);
    assert(resp->node_desc.nwk_addr == resp_exp.node_desc.nwk_addr);
    assert(memcmp(&resp->node_desc.desc, &resp_exp.node_desc.desc, sizeof(node_desc_t)) == 0);
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_NODE_DESC_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_check_pwr_desc

Description:

**************************************************************************/
void test_zdo_check_pwr_desc_resp(buffer_t *buf)
{
    zdo_resp_t *resp;

    resp = (zdo_resp_t *)buf->dptr;

    assert(resp->pwr_desc.status == resp_exp.pwr_desc.status);
    assert(resp->pwr_desc.nwk_addr == resp_exp.pwr_desc.nwk_addr);
    assert(resp->pwr_desc.pwr_desc == resp_exp.pwr_desc.pwr_desc);
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_PWR_DESC_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_check_simple_desc_resp

Description:

**************************************************************************/
void test_zdo_check_simple_desc_resp(buffer_t *buf)
{
    zdo_resp_t *resp;
    simple_desc_t *desc;

    resp = (zdo_resp_t *)buf->dptr;
    desc = (simple_desc_t *)resp->simple_desc.desc;

    assert(resp->simple_desc.status == resp_exp.simple_desc.status);
    assert(resp->simple_desc.nwk_addr == resp_exp.simple_desc.nwk_addr);
    assert(resp->simple_desc.len == resp_exp.simple_desc.len);
    assert(memcmp(desc, resp_data, resp_exp.simple_desc.len) == 0);
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_SIMPLE_DESC_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_active_ep_resp

Description:

**************************************************************************/
void test_zdo_active_ep_resp(buffer_t *buf)
{
    zdo_resp_t *resp;
    U8 i, *ep_list;

    resp = (zdo_resp_t *)buf->dptr;

    if ((ep_list = (U8 *)malloc(resp->active_ep.ep_cnt * sizeof(U8))) != NULL)
    {
        ep_list = resp_data;
    }

    assert(resp->active_ep.status == resp_exp.active_ep.status);
    assert(resp->active_ep.nwk_addr == resp_exp.active_ep.nwk_addr);
    assert(resp->active_ep.ep_cnt == resp_exp.active_ep.ep_cnt);
    for (i=0; i<resp->active_ep.ep_cnt; i++)
    {
        assert(resp->active_ep.eps[i] == ep_list[i]);
    }
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_ACTIVE_EP_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_check_match_desc_resp

Description:

**************************************************************************/
void test_zdo_check_match_desc_resp(buffer_t *buf)
{
    zdo_resp_t *resp;

    resp = (zdo_resp_t *)buf->dptr;
    assert(resp->match_desc.status      == resp_exp.match_desc.status);
    assert(resp->match_desc.nwk_addr    == resp_exp.match_desc.nwk_addr);
    assert(resp->match_desc.match_len   == resp_exp.match_desc.match_len);
    assert(memcmp(resp->match_desc.ep_list, resp_data, resp_data_size) == 0);
    buf_free(buf);

    DBG_PRINT_RAW("ZDO_GEN_MATCH_DESC_REQ_HANDLER_CHECK: \tOK\n");
    process_post(&test_process, event_test_proc, NULL);
}

/**************************************************************************
Function Name: test_zdo_check_load_resp

Description:

**************************************************************************/
void test_zdo_check_load_resp(zdo_resp_t *resp, U8 *data, U8 size)
{
    memcpy(&resp_exp, resp, sizeof(zdo_resp_t));

    if (data)
    {
        assert(size < MAX_RESP_DATA_SIZE);
        memcpy(resp_data, data, size);
        resp_data_size = size;
    }
}



