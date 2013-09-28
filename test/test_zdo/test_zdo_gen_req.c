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

    Title: test_zdo_gen_req.c

    Description:
*******************************************************************/
#include "freakz.h"
#include "test_zdo.h"

/**************************************************************************
Function Name: test_zdo_nwk_addr_req

Description:

**************************************************************************/
void test_zdo_gen_nwk_addr_req()
{
    zdo_req_t req;

    req.seq                     = (U8)drvr_get_rand();
    req.nwk_addr.ext_addr       = 0x1234123412341234ULL;
    req.nwk_addr.req_type       = 0;
    req.nwk_addr.start_index    = 0;

    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_nwk_addr_req(0xffff, req.seq, req.nwk_addr.ext_addr, req.nwk_addr.req_type, req.nwk_addr.start_index);
}

/**************************************************************************
Function Name: test_zdo_ieee_addr_req

Description:

**************************************************************************/
void test_zdo_gen_ieee_addr_req()
{
    zdo_req_t req;

    req.seq                     = (U8)drvr_get_rand();
    req.ieee_addr.nwk_addr      = drvr_get_rand();
    req.ieee_addr.req_type      = 0;
    req.ieee_addr.start_index   = 0;

    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_ieee_addr_req(0xffff, req.seq, req.ieee_addr.nwk_addr, req.ieee_addr.req_type, req.ieee_addr.start_index);
}

/**************************************************************************
Function Name: test_zdo_node_desc_req

Description:

**************************************************************************/
void test_zdo_gen_node_desc_req()
{
    zdo_req_t req;

    req.seq                 = drvr_get_rand();
    req.node_desc.nwk_addr  = drvr_get_rand();

    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_node_desc_req(0xffff, req.seq, req.node_desc.nwk_addr);
}

/**************************************************************************
Function Name: test_zdo_pwr_desc_req

Description:

**************************************************************************/
void test_zdo_gen_pwr_desc_req()
{
    zdo_req_t req;

    req.seq                 = drvr_get_rand();
    req.pwr_desc.nwk_addr   = drvr_get_rand();
    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_pwr_desc_req(0xffff, req.seq, req.pwr_desc.nwk_addr);
}

/**************************************************************************
Function Name: test_zdo_simple_desc_req

Description:

**************************************************************************/
void test_zdo_gen_simple_desc_req()
{
    zdo_req_t req;

    req.seq                     = drvr_get_rand();
    req.simple_desc.nwk_addr    = drvr_get_rand();
    req.simple_desc.ep          = (U8)drvr_get_rand();

    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_simple_desc_req(0xffff, req.seq, req.simple_desc.nwk_addr, req.simple_desc.ep);
}

/**************************************************************************
Function Name: test_zdo_active_ep_req

Description:

**************************************************************************/
void test_zdo_gen_active_ep_req()
{
    zdo_req_t req;

    req.seq                 = drvr_get_rand();
    req.active_ep.nwk_addr  = drvr_get_rand();
    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_active_ep_req(0xffff, req.seq, req.active_ep.nwk_addr);
}

/**************************************************************************
Function Name: test_zdo_match_desc_req

Description:

**************************************************************************/
void test_zdo_gen_match_desc_req()
{
    zdo_req_t req;
    clust_list_t *in_list, *out_list;
    U8 in_cnt, out_cnt, in_size, out_size, i, *clust_list;

    req.seq = drvr_get_rand();
    req.match_desc.nwk_addr = drvr_get_rand();
    req.match_desc.prof_id = (U8)drvr_get_rand();

    in_cnt  = 2;
    out_cnt = 3;
    in_size = 1 + in_cnt * sizeof(U16);
    out_size = 1 + out_cnt * sizeof(U16);

    in_list = (clust_list_t *)malloc(in_size);
    out_list = (clust_list_t *)malloc(out_size);
    clust_list = (U8 *)malloc(in_size + out_size);
    assert(in_list);
    assert(out_list);
    assert(clust_list);

    in_list->clust_cnt = in_cnt;
    for (i=0; i<in_list->clust_cnt; i++)
    {
        in_list->clust_list[i] = drvr_get_rand();
    }

    out_list->clust_cnt = out_cnt;
    for (i=0; i<out_list->clust_cnt; i++)
    {
        out_list->clust_list[i] = drvr_get_rand();
    }

    memcpy(clust_list, in_list, in_size);
    memcpy(clust_list + in_size, out_list, out_size);

    test_zdo_check_load_req(&req, clust_list, in_size + out_size);
    zdo_gen_match_desc_req(0xffff, req.seq, req.match_desc.nwk_addr, req.match_desc.prof_id, in_list, out_list);

    free(in_list);
    free(out_list);
    free(clust_list);
}

/**************************************************************************
Function Name: test_zdo_dev_annce_req

Description:

**************************************************************************/
void test_zdo_gen_dev_annce_req()
{
    zdo_req_t req;

    req.seq                     = drvr_get_rand();
    req.dev_annce.nwk_addr      = drvr_get_rand();
    req.dev_annce.ext_addr      = 0x1122334455667788ULL;
    req.dev_annce.capab_info    = 0xFF;

    test_zdo_check_load_req(&req, NULL, 0);
    zdo_gen_dev_annce_req(0xffff, req.seq, req.dev_annce.nwk_addr, req.dev_annce.ext_addr, req.dev_annce.capab_info);
}



