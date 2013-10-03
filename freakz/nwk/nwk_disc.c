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
    \file nwk_disc.c
    \ingroup nwk
    \brief NWK discovery

    This file handles the network discovery functions for the NWK layer.
    It will basically initiate an active scan in the mac layer, and the
    received beacons will be sent here to be processed. When the network
    discovery is complete, it will issue the confirm to the ZDO layer.
*/
#include "freakz.h"

/*
 * This function will start a network discovery operation. It will start an active scan
 * which records all the networks on the given channels. Once the scan is finished,
 * it will call mlme_scan_confirm which will then send the results up to the ZDO.
 */
void nwk_disc_req(U32 channel_mask, U8 duration)
{
	nwk_pcb_t *nwk_pcb = nwk_pcb_get();
	mac_pcb_t *mac_pcb = mac_pcb_get();

	/*
	 * set the state. we will need this later when we do
	 * the confirm to know what function we are confirming.
	 */
	nwk_pcb->nlme_state = NLME_NWK_DISC;

	/*
	 * need to fill out the mac pcb with the channel mask
	 * and duration because the function uses a callback
	 * timer so the argument needs to be void
	 */
	mac_pcb->channel_mask   = nwk_pcb->channel_mask = channel_mask;
	mac_pcb->duration       = nwk_pcb->duration     = duration;
	mac_pcb->scan_type      = MAC_ACTIVE_SCAN;

	/* clear the pan descriptor list and do the active scan. */
	mac_scan_descr_clear();
	mac_scan(NULL);
}

/*
 * This function is called after the specified scan is finished. The results
 * of the scan are also sent to this function to be forwarded to the appropriate
 * function.
 */
void mac_scan_conf(const mac_scan_conf_t *args)
{
	nwk_pcb_t *pcb = nwk_pcb_get();

	if (args->scan_type == MAC_ENERGY_SCAN) {
		/*
		 * as long as the operation is successful, add the energy
		 * list to the pcb. otherwise, trash it.
		 */
		pcb->energy_list = (args->status == MAC_SUCCESS) ?
					args->energy_list : NULL;
	} else if (args->scan_type == MAC_ACTIVE_SCAN) {
		/*
		 * after the active scan, then we should be done with
		 * either the nwk formation or the nwk disc operations.
		 *
		 * send the confirm to the ZDO based on the operation
		 * that was requested.
		 */
		if (pcb->nlme_state == NLME_NWK_DISC) {
			/* Send the nwk discovery confirm */
			nwk_disc_conf(NWK_SUCCESS, mac_scan_descr_get_head());
			pcb->nlme_state = NLME_IDLE;
		} else if (pcb->nlme_state == NLME_NWK_FORMATION) {
			/*
			 * if we're doing a nwk formation, then no need
			 * to send a scan confirm. call nwk_form_select_channel.
			 */
			pcb->nlme_state = NLME_IDLE;
			nwk_form_select_channel();
		}
	} else if (args->scan_type == MAC_ORPHAN_SCAN) {
		/*
		 * if the orphan scan was successful, then we can just
		 * continue how we were before. otherwise, if we can't
		 * locate the parent, then we need to take some drastic
		 * measures. start by sending the join confirm.
		 */
		if (args->status != MAC_SUCCESS)
			nwk_join_conf(NWK_NO_NETWORKS, 0, (U64)0, 0);
	}
}

/*
 * This function indicates that a beacon frame has arrived. We will then need to
 * parse the contents and add it to the pan descriptor. If the protocol ID
 * is incorrect, then we trash the descriptor by removing it from the list and
 * freeing it.
 */
void mac_beacon_notify_ind(buffer_t *buf, mem_ptr_t *mem_ptr)
{
	/* parse the beacon contents and add it to the descriptor */
	nwk_parse_beacon(buf, mem_ptr);

	/*
	 * if the protocol id is not for zigbee, then remove the
	 * descr from the scan list and free the block of memory.
	 */
	if (SCAN_ENTRY(mem_ptr)->prot_id != ZIGBEE_PROTOCOL_ID)
		mac_scan_descr_free(mem_ptr);
}
