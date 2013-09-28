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

    Title: test_zdo_check_req.c

    Description:
*******************************************************************/
#include "freakz.h"
#include "test_zdo.h"

static zdo_req_t req_exp;
static U8 req_data[100];

/**************************************************************************
Function Name: test_zdo_check_nwk_addr_req

Description:

**************************************************************************/
void test_zdo_check_nwk_addr_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                  == req_exp.seq);
    assert(req->nwk_addr.ext_addr    == req_exp.nwk_addr.ext_addr);
    assert(req->nwk_addr.req_type    == req_exp.nwk_addr.req_type);
    assert(req->nwk_addr.start_index == req_exp.nwk_addr.start_index);
    DBG_PRINT_RAW("ZDO_GEN_NWK_ADDR_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_ieee_addr_req

Description:

**************************************************************************/
void test_zdo_check_ieee_addr_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                      == req_exp.seq);
    assert(req->ieee_addr.nwk_addr       == req_exp.ieee_addr.nwk_addr);
    assert(req->ieee_addr.req_type       == req_exp.ieee_addr.req_type);
    assert(req->ieee_addr.start_index    == req_exp.ieee_addr.start_index);
    DBG_PRINT_RAW("ZDO_GEN_IEEE_ADDR_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_node_desc_req

Description:

**************************************************************************/
void test_zdo_check_node_desc_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                  == req_exp.seq);
    assert(req->node_desc.nwk_addr   == req_exp.node_desc.nwk_addr);
    DBG_PRINT_RAW("ZDO_GEN_NODE_DESC_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_pwr_desc_req

Description:

**************************************************************************/
void test_zdo_check_pwr_desc_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                  == req_exp.seq);
    assert(req->pwr_desc.nwk_addr    == req_exp.pwr_desc.nwk_addr);
    DBG_PRINT_RAW("ZDO_GEN_PWR_DESC_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_simple_desc_req

Description:

**************************************************************************/
void test_zdo_check_simple_desc_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                  == req_exp.seq);
    assert(req->simple_desc.nwk_addr == req_exp.simple_desc.nwk_addr);
    assert(req->simple_desc.ep       == req_exp.simple_desc.ep);
    DBG_PRINT_RAW("ZDO_GEN_SIMPLE_DESC_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_active_ep_req

Description:

**************************************************************************/
void test_zdo_check_active_ep_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                   == req_exp.seq);
    assert(req->active_ep.nwk_addr    == req_exp.active_ep.nwk_addr);
    DBG_PRINT_RAW("ZDO_GEN_ACTIVE_EP_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_match_desc_req

Description:

**************************************************************************/
void test_zdo_check_match_desc_req(buffer_t *buf)
{
    U8 i;
    zdo_req_t *req;
    clust_list_t *in_list, *out_list, *in_list_exp, *out_list_exp;

    req = (zdo_req_t *)buf->dptr;
    assert(req->seq                 == req_exp.seq);
    assert(req->match_desc.nwk_addr == req_exp.match_desc.nwk_addr);
    assert(req->match_desc.prof_id  == req_exp.match_desc.prof_id);

    // set up the in list pointer overlays
    in_list = (clust_list_t *)req->match_desc.clust_list;
    in_list_exp = (clust_list_t *)req_data;

    // comparison is done here
    assert(in_list->clust_cnt == in_list_exp->clust_cnt);
    for (i=0; i<in_list->clust_cnt; i++)
    {
        assert(in_list->clust_list[i] == in_list_exp->clust_list[i]);
    }

    // set up the out list pointer overlays
    out_list = (clust_list_t *)req->match_desc.clust_list + (1 + (in_list->clust_cnt * sizeof(U16)));
    out_list_exp = (clust_list_t *)req_data + (1 + (in_list_exp->clust_cnt * sizeof(U16)));

    // comparison is done here
    assert(out_list->clust_cnt == out_list_exp->clust_cnt);
    for (i=0; i<out_list->clust_cnt; i++)
    {
        assert(out_list->clust_list[i] == out_list_exp->clust_list[i]);
    }
    DBG_PRINT_RAW("ZDO_GEN_MATCH_DESC_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_dev_annce_req

Description:

**************************************************************************/
void test_zdo_check_dev_annce_req(buffer_t *buf)
{
    zdo_req_t *req;

    req = (zdo_req_t *)buf->dptr;

    assert(req->dev_annce.nwk_addr      == req_exp.dev_annce.nwk_addr);
    assert(req->dev_annce.ext_addr      == req_exp.dev_annce.ext_addr);
    assert(req->dev_annce.capab_info    == req_exp.dev_annce.capab_info);
    DBG_PRINT_RAW("ZDO_GEN_DEV_ANNCE_REQ_CHECK: \tOK\n");
}

/**************************************************************************
Function Name: test_zdo_check_load_req

Description:

**************************************************************************/
void test_zdo_check_load_req(zdo_req_t *req, U8 *data, U8 size)
{
    memcpy(&req_exp, req, sizeof(zdo_req_t));

    if (data)
    {
        memcpy(req_data, data, size);
    }
}

