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
    \file nwk_form.c
    \ingroup nwk
    \brief NWK formation

    This file handles how the Zigbee network is formed. When a device
    is the first node on the network, it must create the network and
    appoint itself the coordinator. To form the network, an energy and
    active scan is performed to see if there are any networks that can
    be joined. If there are none, then the energy scan and scan descriptor
    data is used to decide which channel to start the network on.
*/
#include "freakz.h"

/*
 * This function is called when a network needs to be formed.
 * It starts off by doing an energy scan followed by an active
 * scan. After the scans, a channel is chosen based on energy
 * value and number of PANs in the given channel. Then the nwk
 * address is changed to 0x0 and the pan id is set to the
 * extended mac address.
 */
void nwk_form_req(U32 channel_mask, U8 scan_duration)
{
	nwk_pcb_t *nwk_pcb = nwk_pcb_get();
	mac_pcb_t *mac_pcb = mac_pcb_get();
	nwk_nib_t *nib = nwk_nib_get();

	if (nib->joined) {
		nwk_form_conf(NWK_INVALID_REQUEST);
		return;
	}

	/* we need to fill out the parameters inside the mac and nwk pcb */
	mac_pcb->channel_mask   = nwk_pcb->channel_mask = channel_mask;
	mac_pcb->duration       = nwk_pcb->duration     = scan_duration;
	mac_pcb->scan_type      = MAC_ACTIVE_SCAN;
	nwk_pcb->nlme_state     = NLME_NWK_FORMATION;

	/*
	 * start off the nwk formation by doing an energy scan.
	 * the energy scan uses a busy wait so we don't need to
	 * break out of the thread.
	 */
	mac_scan_energy();

	/* clear the pan descriptor list and do the active scan. */
	mac_scan_descr_clear();
	mac_scan(NULL);
}

/*
 * Select the appropriate channel to start the new network on. It will use the
 * data found in the energy list and the pan descriptor list to select a channel.
 * Then it will call mac start request and try to start up a PAN.
 *
 * \note Preference is given to the lowest number of networks first. Then we check
 * for energy value.
 */
void nwk_form_select_channel()
{
	U8 min, i, channel;
	U8 nwk_cnt[MAC_MAX_CHANNELS];
	U16 pan_id;
	U32 mask = 0;
	nwk_pcb_t *pcb = nwk_pcb_get();
	nwk_nib_t *nib = nwk_nib_get();
	mac_pib_t *pib = mac_pib_get();
	mem_ptr_t *mem_ptr;
	mac_start_req_t req;

	/* clear the nwk cnt */
	memset(nwk_cnt, 0, MAC_MAX_CHANNELS * sizeof(U8));

	/* count the number of networks in each channel */
	for (mem_ptr = mac_scan_descr_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
		nwk_cnt[SCAN_ENTRY(mem_ptr)->channel]++;

	/* find the min nwk cnt in the channels */
	min = 0xFF;
	for (i=0; i<MAC_MAX_CHANNELS; i++)
	{
		/* we only care about channels that are in our channel mask */
		if (pcb->channel_mask & (1 << (MAC_PHY_CHANNEL_OFFSET + i)))
		{
			/*
			 * if the nwk cnt value is less than the min
			 * value we have, then replace min with the
			 * lower network count value
			 */
			min = (nwk_cnt[i] < min) ? nwk_cnt[i] : min;
		}
	}

	/*
	 * now that we have our min value, generate a mask of the
	 * channels with the lowest nwk count
	 */
	for (i=0; i<MAC_MAX_CHANNELS; i++)
	{
		/* we only care about channels that are in our channel mask */
		if (pcb->channel_mask & (1 << (MAC_PHY_CHANNEL_OFFSET + i)))
		{
			if (nwk_cnt[i] == min)
				mask |= 1<<(MAC_PHY_CHANNEL_OFFSET + i);
		}
	}

	/*
	 * go through the energy list and choose the channel
	 * that is in our lowest nwk count mask and also has
	 * the lowest energy value.
	 */
	min = 0xFF;
	channel = 0;
	for (i=0; i<MAC_MAX_CHANNELS; i++)
	{
		/* we only care about channels that are in our mask */
		if (mask & (1 << (MAC_PHY_CHANNEL_OFFSET + i)))
		{
			if (pcb->energy_list[i] < min) {
				min = pcb->energy_list[i];
				channel = i;
			}
		}
	}
	DBG_PRINT("NWK_FORM_SELECT_CHANNEL: Selected channel is %d.\n",
		  channel + MAC_PHY_CHANNEL_OFFSET);

	/*
	 * now choose a pan id and check to make sure that it does
	 * not conflict with any other pan id on this network
	 */
	do
	{
		pan_id = drvr_get_rand();
		for (mem_ptr = mac_scan_descr_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
		{
			/*
			 * check for a match of an identical pan id
			 * on the chosen channel
			 */
			if ((SCAN_ENTRY(mem_ptr)->channel == channel) &&
			    (SCAN_ENTRY(mem_ptr)->coord_pan_id == pan_id))
			{
				/* choose another pan id and restart the check */
				continue;
			}
		}
		/*
		 * if we made it this far, then there is no pan id conflict.
		 * Just check to make sure the pan id is less than 0xFFFF and
		 * we're home free.
		 */
	} while (pan_id == 0xFFFF);

	DBG_PRINT("NWK_FORM_SELECT_CHANNEL: Selected PAN ID is %d.\n",
		  channel + MAC_MAX_CHANNELS);

	/*
	 * we now have our channel and pan id. Set the nwk address
	 * to 0x0000 (for coordinators), set the ext pan id to the
	 * ext address if its current value is 0, and then its time
	 * to start the network.
	 */
	nib->joined         = true;
	nib->short_addr     = 0x0000;
	nib->ext_pan_ID     = (nib->ext_pan_ID == 0) ? pib->ext_addr : nib->ext_pan_ID;
	nib->depth          = 0;
	nib->dev_type       = NWK_COORDINATOR;
	nib->cskip          = nwk_rte_tree_calc_cskip(nib->depth);

	req.channel         = channel + MAC_PHY_CHANNEL_OFFSET;
	req.pan_id          = pan_id;
	mac_start_req(&req);
	nwk_form_conf(NWK_SUCCESS);
}
