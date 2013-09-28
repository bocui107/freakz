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

static U8 *test_zdo_gen_simple_desc(U8 *);

/**************************************************************************
Function Name: test_zdo_gen_req_resp

Description:

**************************************************************************/
void test_zdo_gen_req_resp(buffer_t *buf, void *req_resp, U8 size, U8 *list, U8 list_size, U16 dest_addr, U16 clust_id, U8 tx_opt)
{
    if (list && (list_size > 0))
    {
        buf->dptr -= list_size;
        buf->len += list_size;
        memcpy(buf->dptr, list, list_size);
    }

    buf->dptr -= size;
    buf->len += size;
    memcpy(buf->dptr, req_resp, size);
}

/**************************************************************************
Function Name: test_zdo_nwk_addr_req_handler

Description:

**************************************************************************/
void test_zdo_nwk_addr_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();

    req.seq                     = (U8)drvr_get_rand();
    req.nwk_addr.ext_addr       = 0x1122334455667788ULL;
    req.nwk_addr.req_type       = 0;
    req.nwk_addr.start_index    = 0;

    BUF_ALLOC(buf, TX);
    hdr.clust_id                = NWK_ADDR_REQ_CLUST;
    hdr.src_addr                = 0xfffe;

    resp.addr.nwk_addr_rmt      = pib->short_addr;
    resp.addr.ext_addr_rmt      = pib->ext_addr;

    test_zdo_check_load_resp(&resp, NULL, 0);
    test_zdo_gen_req_resp(buf, &req, NWK_ADDR_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);
}

/**************************************************************************
Function Name: test_zdo_node_desc_req_handler

Description:

**************************************************************************/
void test_zdo_node_desc_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();

    req.seq                 = (U8)drvr_get_rand();
    req.node_desc.nwk_addr  = pib->short_addr;

    resp.node_desc.status   = AF_SUCCESS;
    resp.node_desc.nwk_addr = pib->short_addr;
    memcpy(&resp.node_desc.desc, zdo_cfg_node_desc_get(), sizeof(node_desc_t));

    BUF_ALLOC(buf, TX);
    hdr.clust_id = NODE_DESC_REQ_CLUST;
    hdr.src_addr = 0xfffe;

    test_zdo_check_load_resp(&resp, NULL, 0);
    test_zdo_gen_req_resp(buf, &req, NODE_DESC_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);
}

/**************************************************************************
Function Name: test_zdo_pwr_desc_req_handler

Description:

**************************************************************************/
void test_zdo_pwr_desc_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();

    req.pwr_desc.nwk_addr = pib->short_addr;

    resp.pwr_desc.status = AF_SUCCESS;
    resp.pwr_desc.nwk_addr = pib->short_addr;
    memcpy(&resp.pwr_desc.pwr_desc, zdo_cfg_pwr_desc_get(), sizeof(U16));

    BUF_ALLOC(buf, TX);
    hdr.clust_id = PWR_DESC_REQ_CLUST;
    hdr.src_addr = 0xfffe;

    test_zdo_check_load_resp(&resp, NULL, 0);
    test_zdo_gen_req_resp(buf, &req, PWR_DESC_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);
}

/**************************************************************************
Function Name: test_zdo_simple_desc_req_handler

Description:

**************************************************************************/
void test_zdo_simple_desc_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();
    U8 *desc, desc_size;
    simple_desc_t *simple_desc;

    desc = test_zdo_gen_simple_desc(&desc_size);
    simple_desc = (simple_desc_t *)desc;
    assert(desc);

    af_ep_reg(simple_desc->ep, desc, desc_size, NULL, NULL);

    // make the simple desc request
    req.seq                     = (U8)drvr_get_rand();
    req.simple_desc.nwk_addr    = pib->short_addr;
    req.simple_desc.ep          = simple_desc->ep;

    resp.simple_desc.status     = AF_SUCCESS;
    resp.simple_desc.nwk_addr   = pib->short_addr;
    resp.simple_desc.len        = desc_size;

    BUF_ALLOC(buf, TX);
    hdr.clust_id = SIMPLE_DESC_REQ_CLUST;
    hdr.src_addr = 0xfffe;

    test_zdo_check_load_resp(&resp, desc, desc_size);
    test_zdo_gen_req_resp(buf, &req, SIMPLE_DESC_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);

    if (desc)
    {
        free(desc);
    }
}

/**************************************************************************
Function Name: test_zdo_active_ep_req_handler

Description:

**************************************************************************/
void test_zdo_active_ep_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();
    U8 *desc, desc_size;
    simple_desc_t *simple_desc;

    // clear the AF
    af_ep_clear_all();

    // gen the request
    req.seq                 = (U8)drvr_get_rand();
    req.active_ep.nwk_addr  = pib->short_addr;

    // gen the simple descriptor and register the ep with the af
    desc = test_zdo_gen_simple_desc(&desc_size);
    simple_desc = (simple_desc_t *)desc;
    assert(desc);

    af_ep_reg(simple_desc->ep, desc, desc_size, NULL, NULL);

    // gen the expected response
    resp.active_ep.status   = AF_SUCCESS;
    resp.active_ep.nwk_addr = pib->short_addr;
    resp.active_ep.ep_cnt   = 1;

    BUF_ALLOC(buf, TX);
    hdr.clust_id = ACTIVE_EP_REQ_CLUST;
    hdr.src_addr = 0xfffe;

    test_zdo_check_load_resp(&resp, desc, desc_size);
    test_zdo_gen_req_resp(buf, &req, ACTIVE_EP_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);
}

/**************************************************************************
Function Name: test_zdo_match_desc_req_handler

Description:

**************************************************************************/
void test_zdo_match_desc_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();
    U8 *desc = NULL, *in_start, *out_start, *clust_list = NULL, desc_size, in_size, out_size, clust_list_size, ep_list;
    simple_desc_t *simple_desc;
    clust_list_t *in_list, *out_list;

    desc = test_zdo_gen_simple_desc(&desc_size);
    simple_desc = (simple_desc_t *)desc;
    assert(desc_size);

    af_ep_reg(simple_desc->ep, desc, desc_size, NULL, NULL);

    // pointer overlays for the simple descriptor and the cluster lists
    in_start = simple_desc->clust_info;
    in_list = (clust_list_t *)in_start;
    in_size = 1 + in_list->clust_cnt * sizeof(U16);

    out_start = in_start + in_size;
    out_list = (clust_list_t *)out_start;
    out_size = 1 + out_list->clust_cnt * sizeof(U16);

    clust_list_size = in_size + out_size;
    if ((clust_list = (U8 *)malloc(clust_list_size)) != NULL)
    {
        memcpy(clust_list, in_list, in_size);
        memcpy(clust_list + in_size, out_list, out_size);
    }

    req.seq                     = (U8)drvr_get_rand();
    req.match_desc.nwk_addr     = pib->short_addr;
    req.match_desc.prof_id      = simple_desc->prof_id;

    resp.match_desc.status      = AF_SUCCESS;
    resp.match_desc.nwk_addr    = pib->short_addr;
    resp.match_desc.match_len   = 1;
    ep_list                     = simple_desc->ep;

    BUF_ALLOC(buf, TX);
    hdr.clust_id                = MATCH_DESC_REQ_CLUST;
    hdr.src_addr                = 0xfffe;

    test_zdo_check_load_resp(&resp, &ep_list, 1);
    test_zdo_gen_req_resp(buf, &req, MATCH_DESC_REQ_BASE_SIZE, clust_list, clust_list_size, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);

    if (desc)
    {
        free(desc);
    }
}

/**************************************************************************
Function Name: test_zdo_dev_annce_req_handler

Description:

**************************************************************************/
void test_zdo_dev_annce_req_handler()
{
    zdo_req_t req;
    aps_hdr_t hdr;
    buffer_t *buf;
    mac_pib_t *pib = mac_pib_get();

    req.seq                     = (U8)drvr_get_rand();
    req.dev_annce.nwk_addr      = pib->short_addr;
    req.dev_annce.ext_addr      = pib->ext_addr;
    req.dev_annce.capab_info    = 0xff;

    BUF_ALLOC(buf, TX);
    hdr.clust_id                = DEV_ANNCE_REQ_CLUST;
    hdr.src_addr                = 0xfffe;

    test_zdo_gen_req_resp(buf, &req, DEV_ANNCE_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx_handler(buf->dptr, buf->len, hdr.src_addr, hdr.src_ep, hdr.clust_id);
    buf_free(buf);
}

/**************************************************************************
Function Name: test_zdo_nwk_disc_req_handler

Description:
TODO: NOT FINISHED!!!
**************************************************************************/
/*void test_zdo_nwk_disc_req_handler()
{
    zdo_req_t req;
    zdo_resp_t resp;
    aps_hdr_t hdr;
    buffer_t *buf;
    aps_aib_t *aib = aps_aib_get();

    req.seq                     = (U8)drvr_get_rand();
    req.nwk_disc.scan_channels  = aib->chan_mask;
    req.nwk_disc.scan_duration  = 2;

    resp.nwk_disc.status = AF_SUCCESS;
    //resp.nwk_disc.nwk_cnt = ;
    //resp.nwk_disc.nwk_list_cnt = ;

    BUF_ALLOC(buf, TX);
    hdr.clust_id                = NWK_DISC_REQ_CLUST;
    hdr.src_addr                = 0xfffe;

    test_zdo_check_load_resp(&resp, &ep_list, 1);
    test_zdo_gen_req_resp(buf, &req, NWK_DISC_REQ_SIZE, NULL, 0, 0xffff, hdr.clust_id, 0);
    zdo_rx(buf, &hdr);
}*/

/**************************************************************************
Function Name: test_zdo_gen_simple_desc

Description:

**************************************************************************/
static U8 *test_zdo_gen_simple_desc(U8 *size)
{
    simple_desc_t tmp_desc;
    clust_list_t *in_list, *out_list;
    U8 in_cnt, out_cnt, in_size, out_size, i, simple_desc_size, *desc = NULL;

    // generate simple descriptor
    tmp_desc.ep         = drvr_get_rand();
    tmp_desc.prof_id    = drvr_get_rand();
    tmp_desc.dev_id     = drvr_get_rand();
    tmp_desc.dev_ver    = (U8)drvr_get_rand();

    in_cnt      = 3;
    out_cnt     = 4;
    in_size     = 1 + in_cnt * sizeof(U16);
    out_size    = 1 + out_cnt * sizeof(U16);

    if ((in_list = (clust_list_t *)malloc(in_size)) != NULL)
    {
        in_list->clust_cnt = in_cnt;

        for (i=0; i<in_cnt; i++)
        {
            in_list->clust_list[i] = drvr_get_rand();
        }
    }

    if ((out_list = (clust_list_t *)malloc(sizeof(out_size))) != NULL)
    {
        out_list->clust_cnt = out_cnt;

        for (i=0; i<out_cnt; i++)
        {
            out_list->clust_list[i] = drvr_get_rand();
        }
    }

    // glob the simple descriptor into one big chunk of ram and register with AF
    simple_desc_size = sizeof(simple_desc_t) + in_size + out_size;
    if ((desc = (U8 *)malloc(simple_desc_size)) != NULL)
    {
        simple_desc_t *simple_desc_ptr = (simple_desc_t *)desc;
        U8 *in_start, *out_start;

        in_start = simple_desc_ptr->clust_info;
        out_start = simple_desc_ptr->clust_info + in_size;

        memcpy(simple_desc_ptr, &tmp_desc, sizeof(simple_desc_t));

        memcpy(in_start, in_list, in_size);
        memcpy(out_start, out_list, out_size);
        *size = simple_desc_size;

        free(in_list);
        free(out_list);
        return desc;
    }

    free(in_list);
    free(out_list);
    return 0;
}


