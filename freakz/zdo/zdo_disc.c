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
    \file zdo_disc.c
    \ingroup zdo
    \brief ZDO device and service discovery manager

    This file contains functions that perform the client and server
    side of device discovery. Device discovery is the ability for a
    Zigbee device to request information about other devices on the network
    and to identify devices that have compatible profiles.

    /note The client-side request generation is handled in zdo_gen.c
*/
#include "freakz.h"

static U8 zdo_disc_addr_resp(const zdo_req_t *req, U8 *resp_data, bool nwk_addr_req);

/* ZDO DEVICE AND SERVICE DISCOVERY - SERVER FUNCTIONS */

/*
 * Handler function for the nwk addr and IEEE addr requests. When an address
 * request arrives, it will be routed to this function which will assemble
 * the response and send it back out.
 *
 * /note The request handler only handles the address for this device. It
 * doesn't support sending back a list of all associated devices yet.
 */
void zdo_disc_addr_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, size, resp_data[MAX_APS_PAYLOAD];
	mac_pib_t *pib = mac_pib_get();
	bool nwk_addr_req, match;
	U16 resp_clust_id;
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);

	/* figure out which addr request it wants, then parse the buffer */
	nwk_addr_req = (clust == NWK_ADDR_REQ_CLUST) ? true : false;

	/*
	 * if there's no address match, then exit this function.
	 * we don't need to send a response.
	 */
	match = (nwk_addr_req) ?
		(req.type.nwk_addr.ext_addr == pib->ext_addr) :
		(req.type.ieee_addr.nwk_addr == pib->short_addr);
	if (!match)
		return;

	/* generate the response */
	resp_clust_id = (nwk_addr_req) ? NWK_ADDR_RESP_CLUST : IEEE_ADDR_RESP_CLUST;
	size = zdo_disc_addr_resp(&req, resp_data, nwk_addr_req);
	handle = af_handle_get();

	/* figure out which cluster to send it to and then send it out */
	zdo_tx(resp_data, size, src_addr, resp_clust_id, 0, handle);
}

/* Handler function for the node descriptor request */
void zdo_disc_node_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, size, *resp, resp_data[MAX_APS_PAYLOAD];
	node_desc_t *desc = zdo_cfg_node_desc_get();
	nwk_nib_t *nib = nwk_nib_get();
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);

	resp            = resp_data;
	*resp++         = req.seq;
	*(U16 *)resp    = nib->short_addr;
	resp            += sizeof(U16);

	if (req.type.node_desc.nwk_addr == nib->short_addr)
	{
		*resp++ = AF_SUCCESS;
		memcpy(resp, desc, sizeof(node_desc_t));
		resp += sizeof(node_desc_t);
	} else {
		/* address does not match */
		*resp++ = AF_INV_REQUESTTYPE;
	}

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, NODE_DESC_RESP_CLUST, 0, handle);
}

/* Handler for the power descriptor request */
void zdo_disc_pwr_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, size, *resp, resp_data[MAX_APS_PAYLOAD];
	U16 *pwr_desc = zdo_cfg_pwr_desc_get();
	nwk_nib_t *nib = nwk_nib_get();
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);

	resp            = resp_data;
	*resp++         = req.seq;
	*(U16 *)resp    = req.type.pwr_desc.nwk_addr;
	resp            += sizeof(U16);

	if (req.type.pwr_desc.nwk_addr == nib->short_addr)
	{
		*resp++ = AF_SUCCESS;
		memcpy(resp, pwr_desc, sizeof(U16));
		resp += sizeof(U16);
	} else {
		/* address does not match */
		*resp++ = AF_INV_REQUESTTYPE;
	}

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, PWR_DESC_RESP_CLUST, 0, handle);
}

/*
 * Handle the simple descriptor request and generate the response. The simple
 * descriptor will be based on the endpoint that came in the request argument.
 * If the endpoint does not exist, we will return an error status.
 */
void zdo_disc_simple_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, size, *resp, resp_data[MAX_APS_PAYLOAD];
	nwk_nib_t *nib = nwk_nib_get();
	mem_ptr_t *mem_ptr;
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);

	resp            = resp_data;
	*resp++         = req.seq;
	*(U16 *)resp    = req.type.simple_desc.nwk_addr;
	resp            += sizeof(U16);

	if (req.type.simple_desc.nwk_addr == nib->short_addr)
	{
		if ((req.type.simple_desc.ep == 0) || (req.type.simple_desc.ep > AF_EP_RANGE_MAX))
		{
			*resp++ = AF_INVALID_EP;    // simple desc req status
			*resp++ = 0;                // simple desc len
			size    = resp - resp_data;
			zdo_tx(resp_data, size, src_addr, SIMPLE_DESC_RESP_CLUST, 0, req.seq);
			return;
		}

		if ((mem_ptr = af_ep_find(req.type.simple_desc.ep)) != NULL)
		{
			*resp++ = AF_SUCCESS;
			*resp++ = EP_ENTRY(mem_ptr)->simple_desc_size;
			memcpy(resp, EP_ENTRY(mem_ptr)->simple_desc, EP_ENTRY(mem_ptr)->simple_desc_size);
			resp    += EP_ENTRY(mem_ptr)->simple_desc_size;
		} else {
			/* endpoint does not exist on this device */
			*resp++ = AF_NOT_ACTIVE;
			*resp++ = 0;
		}
	} else {
		/* nwk addr does not match */
		*resp++ = AF_INV_REQUESTTYPE;
		*resp++ = 0;
	}

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, SIMPLE_DESC_RESP_CLUST, 0, handle);
}

/*
 * Handle the active endpoint request and generate the response. This response
 * contains a list of the active endpoints for this device. If the address does
 * not match, then return an error status.
 */
void zdo_disc_active_ep_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, size, tmp, *resp, resp_data[MAX_APS_PAYLOAD];
	nwk_nib_t *nib = nwk_nib_get();
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);

	resp            = resp_data;
	*resp++         = req.seq;

	if (req.type.active_ep.nwk_addr == nib->short_addr)
	{
		/* start filling out the response and calc the size */
		*resp++         = AF_SUCCESS;
		*(U16 *)resp    = req.type.active_ep.nwk_addr;
		resp            += sizeof(U16);
		tmp = af_ep_get_active(resp);
		resp += tmp;
	} else {
		/* addr does not match */
		*resp++         = AF_INV_REQUESTTYPE;
		*(U16 *)resp    = req.type.active_ep.nwk_addr;
		resp            += sizeof(U16);
		*resp++         = 0;
	}

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, ACTIVE_EP_RESP_CLUST, 0, handle);
}

/*  Handler function for the match request */
void zdo_disc_match_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	bool addr_ok;
	U8 handle, size, ep_cnt, *resp;
	U8 resp_data[MAX_APS_PAYLOAD], eps[ZIGBEE_MAX_ENDPOINTS];
	nwk_nib_t *nib = nwk_nib_get();
	clust_list_t *in_list, *out_list;
	zdo_req_t req;

	// parse the request
	zdo_parse_req(src_addr, data, clust, &req);

	in_list = (clust_list_t *)&req.type.match_desc.clust_list;
	out_list = in_list + 1 + (in_list->clust_cnt * sizeof(U16));

	/* check to see if the address is okay */
	if ((req.type.match_desc.nwk_addr & NWK_BROADCAST_MASK) == NWK_BROADCAST_MASK)
		addr_ok = nwk_brc_check_dev_match(req.type.match_desc.nwk_addr);
	else
		addr_ok = (req.type.match_desc.nwk_addr == nib->short_addr);

	resp            = resp_data;
	*resp++         = req.seq;
	*(U16 *)resp    = req.type.match_desc.nwk_addr;

	if (addr_ok)
	{
		ep_cnt = af_ep_find_matches(req.type.match_desc.prof_id, in_list, out_list, eps);
		if (ep_cnt)
		{
			*resp++ = AF_SUCCESS;
			*resp++ = ep_cnt;
			memcpy(resp, eps, ep_cnt);
			resp    += ep_cnt;
		} else {
			*resp++ = AF_NO_DESCRIPTOR;
			*resp++ = 0;
		}
	} else {
		*resp++ = (nib->dev_type == NWK_END_DEVICE) ? AF_INV_REQUESTTYPE : AF_DEVICE_NOT_FOUND;
		*resp++ = 0;
	}

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, MATCH_DESC_RESP_CLUST, 0, handle);
}

/*
 * Handler function for the end device announce request. This request is a bit
 * different from other requests in that it doesn't expect a response. The end
 * device announce request just announces to all devices on the network that
 * a new end device has joined the network.
 *
 * The current handling is just to add it to the address map. However depending on
 * the application, we may want to query the descriptors of the device and see
 * what profiles it supports.
 */
void zdo_disc_dev_annce_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	zdo_req_t req;

	/* parse the request */
	zdo_parse_req(src_addr, data, clust, &req);
	nwk_addr_map_add(req.type.dev_annce.nwk_addr,
			 req.type.dev_annce.ext_addr,
			 req.type.dev_annce.capab_info);
}

/*
 * Generate the address response and return the size. This response is used
 * for both the IEEE address request and the nwk address request. If the
 * extended device response is requested (ie the associated device list) then
 * a status of INVALID REQUEST TYPE is returned.
 */
static U8 zdo_disc_addr_resp(const zdo_req_t *req, U8 *resp_data, bool nwk_addr_req)
{
	mac_pib_t *pib = mac_pib_get();
	U8 req_type, *resp;

	req_type = (nwk_addr_req) ? req->type.nwk_addr.req_type : req->type.ieee_addr.req_type;

	/*
	 * we will use the nwk_addr_resp structure, but the layout
	 * is the same as ieee_addr_resp
	 */
	resp            = resp_data;
	*resp++         = req->seq;

	/* select the status */
	switch (req_type)
	{
	case ZDO_EXT_DEV_RESP:
		/* not supported yet */
		*resp++ = AF_INV_REQUESTTYPE;
		break;
	case ZDO_SINGLE_DEV_RESP:
		*resp++ = AF_SUCCESS;
		break;
	default:
		*resp++ = AF_INV_REQUESTTYPE;
		break;
	}

	/* fill out the rest of the fields */
	*(U64 *)resp    = pib->ext_addr;
	resp            += sizeof(U64);
	*(U16 *)resp    = pib->short_addr;
	resp            += sizeof(U16);

	return resp - resp_data;
}

/* ZDO DEVICE AND SERVICE DISCOVERY - CLIENT FUNCTIONS */

/*
 * This is the handler for the nwk and IEEE address response. This is a client
 * function where a request was sent and the response was returned. Its now
 * up to us to handle the response. The handler may vary depending on the
 * application.
 */
void zdo_disc_addr_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	zdo_resp_t *resp = (zdo_resp_t *)data;

	resp->clust = clust;
	debug_dump_zdo_resp(resp, data);

	/* if its not successful, then bail */
	if (resp->type.addr.status != AF_SUCCESS)
		return;

	/*
	 * Add an address map entry for this device. If it already exists,
	 * replace the existing info with this info.
	 */
	nwk_addr_map_add(resp->type.addr.nwk_addr_rmt, resp->type.addr.ext_addr_rmt, 0xff);

	/* TODO: Should I add support for multiple addresses that are returned? */
}

/*
 * Handle the node descriptor response. This will need to be defined by
 * this device's application since only it knows what to do with the info
 * in the node descriptor.
 */
void zdo_disc_node_desc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
}

/*
 * Handle the power descriptor response. This will need to be defined by
 * this device's application since only it knows what to do with the info
 * in the power descriptor.
 */
void zdo_disc_pwr_desc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
}

/*
 * Handle the simple descriptor response. This will need to be defined by
 * this device's application since only it knows what to do with the info
 * in the requested endpoint's simple descriptor.
 */
void zdo_disc_simple_desc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
}

/*
 * Handle the match descriptor response. This will need to be defined by
 * this device's application since only it knows what to do with the info
 * in the match response.
 */
void zdo_disc_match_desc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
}
