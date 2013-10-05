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
    \file zdo_gen.c
    \ingroup zdo
    \brief ZDO frame generator

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the generation of requests and responses for the
    ZDO endpoint.
*/
/**************************************************************************/
#include "freakz.h"

/*
 * This is where all the requests in the ZDO are generated. A request structure
 * and dest address is passed in. From there, the request is generated and
 * sent out.
 */
U8 zdo_gen_req(U16 dest_addr, zdo_req_t *params)
{
	U8 handle, size, *req, req_data[ZDO_REQ_MAX_BUF_SIZE];

	req = req_data;
	*req++ = params->seq;

	switch (params->clust)
	{
	/* ZDO Discovery Manager requests */
	case NWK_ADDR_REQ_CLUST:
		*(U64 *)req = params->type.nwk_addr.ext_addr;
		req += sizeof(U64);
		*req++ = params->type.nwk_addr.req_type;
		*req++ = params->type.nwk_addr.start_index;
		break;
	case IEEE_ADDR_REQ_CLUST:
		*(U16 *)req = params->type.ieee_addr.nwk_addr;
		req += sizeof(U16);
		*req++ = params->type.ieee_addr.req_type;
		*req++ = params->type.ieee_addr.start_index;
		break;
	case NODE_DESC_REQ_CLUST:
	case PWR_DESC_REQ_CLUST:
	case ACTIVE_EP_REQ_CLUST:
		*(U16 *)req = params->type.node_desc.nwk_addr;
		req += sizeof(U16);
		break;
	case SIMPLE_DESC_REQ_CLUST:
		*(U16 *)req = params->type.simple_desc.nwk_addr;
		req += sizeof(U16);
		*req++ = params->type.simple_desc.ep;
		break;
	case DEV_ANNCE_REQ_CLUST:
		*(U16 *)req = params->type.dev_annce.nwk_addr;
		req += sizeof(U16);
		*(U64 *)req = params->type.dev_annce.ext_addr;
		req += sizeof(U64);
		*req++ = params->type.dev_annce.capab_info;
		break;
	/* ZDO Network Manager requests */
	case NWK_DISC_REQ_CLUST:
		*(U32 *)req = params->type.nwk_disc.scan_channels;
		req += sizeof(U32);
		*req++ = params->type.nwk_disc.scan_duration;
		*req++ = params->type.nwk_disc.start_index;
		break;
	case NWK_LQI_REQ_CLUST:
		*req++ = params->type.nwk_lqi.start_index;
		break;
	case NWK_PERMIT_JOIN_REQ_CLUST:
		*req++ = params->type.permit_join.duration;
		*req++ = params->type.permit_join.tc_sig;
		break;
	case NWK_LEAVE_REQ_CLUST:
		*(U64 *)req = params->type.leave.addr;
		req += sizeof(U64);
		*req++ = ((params->type.leave.rem_children << 1) | params->type.leave.rejoin) & 0x3;
		break;
	case NWK_UPDATE_REQ_CLUST:
		/*
		 * Generate a NWK update request. A NWK update is used for various
		 * purposes.
		 * - Change the active channel for the device. This is used for
		 *   frequency agility so that all devices can be told to change
		 *   to the specified channel if one channel becomes overly noisy.
		 * - Change the channel mask. The requesting device can instruct
		 *   the destination device/s to add or remove channels to the mask.
		 * - Energy scan. Scan the network and report back the energy conditions
		 *   on each channel. The info can then be used to see if a channel
		 *   switch is necessary (freq agility) and which channel to switch to.
		*/

		*(U32 *)req = params->type.nwk_update.scan_channels;
		req += sizeof(U32);
		*req++ = params->type.nwk_update.scan_duration;

		if (params->type.nwk_update.scan_duration == 0xFE)
		{
			/* requesting a channel change */
			*req++ = params->type.nwk_update.nwk_update_id;
		} else if (params->type.nwk_update.scan_duration == 0xFF) {
			/* requesting a new channel mask and nwk manager address */
			*req++ = params->type.nwk_update.nwk_update_id;
			*(U16 *)req = params->type.nwk_update.nwk_mngr_addr;
			req += sizeof(U16);
		} else if (params->type.nwk_update.scan_duration <= 5) 	{
			*req++ = params->type.nwk_update.scan_cnt;
		}
		break;
	/* ZDO Binding Manager requests */
	case END_DEV_BIND_REQ_CLUST:
		*(U16 *)req = params->type.ed_bind.target;
		req += sizeof(U16);
		*(U64 *)req = params->type.ed_bind.src_ext_addr;
		req += sizeof(U64);
		*req++ = params->type.ed_bind.src_ep;
		*(U16 *)req = params->type.ed_bind.prof_id;
		req += sizeof(U16);
		*req++ = params->type.ed_bind.num_in_clust;
		memcpy(req, params->type.ed_bind.in_clust, params->type.ed_bind.num_in_clust * 2);
		req += 2 * params->type.ed_bind.num_in_clust;
		*req++ = params->type.ed_bind.num_out_clust;
		memcpy(req, params->type.ed_bind.out_clust, params->type.ed_bind.num_out_clust * 2);
		req += 2 * params->type.ed_bind.num_out_clust;
		break;
	case BIND_REQ_CLUST:
	case UNBIND_REQ_CLUST:
		/*
		 * bind and unbind requests have the same format.
		 * The only difference is the target cluster.
		 */
		*(U64 *)req = params->type.bind.src_addr;
		req += sizeof(U64);
		*req++ = params->type.bind.src_ep;
		*(U16 *)req = params->type.bind.clust;
		req += sizeof(U16);
		*req++ = params->type.bind.dest_addr.mode;

		/*
		 * in this case, the address modes are slightly
		 * different than those defined for the MAC
		 */
		if (params->type.bind.dest_addr.mode == BIND_GRP_ADDR)
		{
			/*
			 * if the dest addr is for group addr, then
			 * don't include the dest ep
			 */
			*(U16 *)req = params->type.bind.dest_addr.short_addr;
			req += sizeof(U16);
		} else if (params->type.bind.dest_addr.mode == BIND_EXT_ADDR) {
			/*
			 * if the dest addr is an extended one, then include
			 * the dest ep
			 */
			*(U64 *)req = params->type.bind.dest_addr.long_addr;
			req += sizeof(U64);
			*req++ = params->type.bind.dest_ep;
		}
		break;
	default:
		return AF_INV_REQUESTTYPE;
	}

	size = req - req_data;
	handle = af_handle_get();
	zdo_tx(req_data, size, dest_addr, params->clust, 0, handle);
	return AF_SUCCESS;
}
