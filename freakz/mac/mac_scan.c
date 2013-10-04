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
    \file mac_scan.c
    \ingroup mac
    \brief MAC energy and network scan

    This file handles all the scan functions in the MAC. There are primarily
    four types of scans that are used in 802.15.4 (energy, active, passive, orphan)
    but only three that are used in Zigbee (energy, active, orphan). This
    file contains the code for the energy and active scans. The orphan scan
    is basically an active scan where the orphan is looking for the address
    of its parent.
    This file also maintains a list of scan descriptors that are dynamically
    allocated (via malloc...ugh) when an active scan is performed. The scan
    descriptor list is then passed to the upper layers for evaluation to see
    which network should be joined.
*/
#include "freakz.h"

/*
 * List head for the scan entries. When a network scan is performed, pan
 * descriptors that describe the discovered networks are stored in this list.
 * When the network scan is finished, these entries get sent to the network
 * layer for processing to choose a channel to use or a parent to join.
 */
LIST(scan_list);

/* Init the scan descriptor list */
void mac_scan_init()
{
	list_init(scan_list);
}

/*
 * Allocate a memory block for the PAN descriptor and add it to the scan list.
 * Also, return a memory handle to the alloc'd block.
 */
static mem_ptr_t *mac_scan_alloc()
{
	mem_ptr_t *mem_ptr;

	mem_ptr = mem_heap_alloc(sizeof(pan_descr_t));
	if (mem_ptr)
		list_add(scan_list, mem_ptr);

	return mem_ptr;
}

/* Insert a scan descriptor into the list */
void mac_scan_descr_add(address_t *src_addr, U16 src_pan_id, U8 channel, U16 superframe_spec)
{
	mem_ptr_t *mem_ptr;

	mem_ptr = mac_scan_alloc();
	if (mem_ptr) {
		memcpy(&SCAN_ENTRY(mem_ptr)->coord_addr, src_addr, sizeof(address_t));
		SCAN_ENTRY(mem_ptr)->superfrm_spec  = superframe_spec;
		SCAN_ENTRY(mem_ptr)->coord_pan_id   = src_pan_id;
		SCAN_ENTRY(mem_ptr)->channel        = channel;
	}
}

/* Remove a scan descriptor from the list */
void mac_scan_descr_free(mem_ptr_t *mem_ptr)
{
	list_remove(scan_list, mem_ptr);
	mem_heap_free(mem_ptr);
}

/*
 * Clear all scan descriptors from the list. This is normally used after the
 * upper layers have made their decision on which network to join since they
 * don't need the descriptors any more.
 */
void mac_scan_descr_clear()
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_chop(scan_list); mem_ptr != NULL; mem_ptr = list_chop(scan_list))
	{
		mac_scan_descr_free(mem_ptr);
	}
}

/* Find the specified address and return the memory pointer to it */
mem_ptr_t *mac_scan_descr_find_addr(address_t *addr)
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(scan_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if ((SCAN_ENTRY(mem_ptr)->coord_addr.mode == SHORT_ADDR) &&
			(SCAN_ENTRY(mem_ptr)->coord_addr.short_addr == addr->short_addr))
			break;
		else if ((SCAN_ENTRY(mem_ptr)->coord_addr.mode == LONG_ADDR) &&
			(SCAN_ENTRY(mem_ptr)->coord_addr.long_addr == addr->long_addr))
			break;
	}
	return mem_ptr;
}

/* Return a pointer to the scan descriptor list head */
mem_ptr_t *mac_scan_descr_get_head()
{
	return list_head(scan_list);
}

/* Find a scan descriptor with the specified PAN ID */
mem_ptr_t *mac_scan_descr_find_pan(U16 pan_id)
{
	mem_ptr_t *mem_ptr;

	for (mem_ptr = list_head(scan_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
	{
		if (SCAN_ENTRY(mem_ptr)->coord_pan_id == pan_id)
			break;
	}
	return mem_ptr;
}

/* Return the number of scan descriptors in the list */
U8 mac_scan_get_entry_cnt()
{
	mem_ptr_t *mem_ptr;
	U8 i = 0;

	for (mem_ptr = list_head(scan_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
		i++;

	return i;
}

/*
 * Return a pointer to the energy scan list. The list shows the energy values
 * on each channel.
 */
U8 *mac_scan_get_energy_list()
{
	mac_pcb_t *pcb = mac_pcb_get();

	return pcb->energy_list;
}

/*
 * This is the energy scan function. The channel mask is stored in the MAC
 * protocol control block. The scan function will step through each channel,
 * and if its in the channel mask, then it will take energy measurements on
 * the channel.
 * The energy measurements are taken for a specified interval, which is counted
 * by a timer. The maximum value of the energy measurements is kept. The energy
 * scan measurements are blocking functions, meaning that we stay in this
 * function until all channels are scanned.
 */
void mac_scan_energy()
{
	U8 i, curr_ed;
	mac_scan_conf_t scan_conf;
	mac_pcb_t *pcb = mac_pcb_get();

	/* reset the energy list */
	memset(pcb->energy_list, 0, sizeof(pcb->energy_list));

	/*
	 * Check all the channels for the ED scan. First see if its in our
	 * allowed channels list. If it is, then set a timer, set the
	 * channel, and then poll the RSSI until the timer expires. Keep
	 * the max value that we get.
	 */
	for (i = 0; i < MAC_MAX_CHANNELS; i++)
	{
		/*
		 * check to see if we found a set bit in the channel mask, and
		 * also that the bit is between 11 and 26
		 */
		pcb->curr_scan_channel = i + MAC_PHY_CHANNEL_OFFSET;

		/*
		 * this is done on purpose to shift the bitmask to
		 * the corresponding channel in the channel mask
		 */
		if (pcb->channel_mask & (1 << pcb->curr_scan_channel))
		{
			/* inform everyone that we are currently scanning */
			pcb->mac_state = MLME_SCAN;

			/* set the channel to the current scan channel */
			mac_set_channel(pcb->curr_scan_channel);

			/*
			 * enable transceiver in receive mode so we can get
			 * ED measurements
			 */
			mac_rx_enb(true, false);

			/*
			 * set the timer so that we know when to stop
			 * the energy scan
			 */
			timer_set(&pcb->mlme_tmr.etimer.timer, MAC_SCAN_TIME(pcb->duration));

			/*
			 * poll the etimer in a busy wait as we constantly get
			 * the rssi values. keep the max value that we find.
			 */
			while (!timer_expired(&pcb->mlme_tmr.etimer.timer))
			{
				curr_ed = drvr_get_ed();
				if (curr_ed > pcb->energy_list[i])
					pcb->energy_list[i] = curr_ed;
			}

			pcb->mac_state = MLME_IDLE;
		}
	}

	/* send scan confirm for energy detect */
	scan_conf.scan_type	= MAC_ENERGY_SCAN;
	scan_conf.energy_list	= pcb->energy_list;
	scan_conf.status	= MAC_SUCCESS;
	/* set the trx in auto ack mode */
	mac_rx_enb(true, true);
	mac_scan_conf(&scan_conf);
}

/*
 * This the main function for the active scan. The channel mask is stored
 * in the MAC protocol control block. The scan function will step through
 * each channel, and if its in the channel mask, an active scan will be
 * performed. An active scan basically consists of broadcasting a beacon
 * request command frame and collecting all the incoming beacons from the
 * routers. The beacons get put into the scan descriptor list and the list
 * will be used by the higher layers to choose the parent and the network
 * to join. The network that the device should join can also be specified
 * in the application layer if this is known beforehand (I'm sure it
 * usually is).
 *
 * Unlike the energy scan, the active scan needs to be non-blocking. That
 * means that after the beacon request is sent out, we need to exit the
 * function so that we can process the incoming frames. Hence after the frame
 * is sent out, a callback timer is set to call the mac_scan function again
 * after the scan timeout. That way, we can send out the scan, exit the
 * function, wait for incoming beacons, process them, and after the scan
 * timeout, we can come back to this function and scan the next channel.
 */
void mac_scan(void *ptr)
{
	mac_cmd_t cmd;
	mac_hdr_t hdr;
	buffer_t *buf;
	address_t src_addr, dest_addr;
	mac_scan_conf_t scan_conf;
	U32 duration;
	mac_pcb_t *pcb = mac_pcb_get();
	mac_pib_t *pib = mac_pib_get();

	/* increment the current scan channel on entry into this function */
	pcb->curr_scan_channel++;

	/*
	 * check if we are initializing the scan. if so, then start the
	 * init procedure
	 */
	if (pcb->mac_state != MLME_SCAN)
	{
		/* on a new scan, first save the original pan id */
		pcb->original_pan_id = pib->pan_id;

		/*
		 * then set the pan id to the broadcast pan id.
		 * NOTE: the broadcast addr is same as broadcast pan id
		 */
		pib->pan_id = MAC_BROADCAST_ADDR;

		/* init the curr scan channel to the first one */
		pcb->curr_scan_channel  = MAC_PHY_CHANNEL_OFFSET;
		pcb->mac_state          = MLME_SCAN;
		pcb->nwk_cnt            = 0;
	}

	/*
	 * search the channel mask to find the next allowable channel.
	 * if the curr scan channel is not in the mask, then increment
	 * the channel and check again. keep doing it until we either
	 * find an allowable channel or we exceed the max channels that
	 * are supported.
	 */
	for (; pcb->curr_scan_channel < (MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS);
	       pcb->curr_scan_channel++)
	{
		/* shift the bitmask and compare to the channel mask */
		if (pcb->channel_mask & (1UL << pcb->curr_scan_channel))
			break;
	}

	/*
	 * we may get here if the curr scan channel exceeds the max channels.
	 * if thats the case, then we will automatically end the active scan.
	 */
	if (pcb->curr_scan_channel < (MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS))
	{
		/* set the channel on the radio */
		mac_set_channel(pcb->curr_scan_channel);

		/*
		 * generate and send the beacon request get a free buffer, build the
		 * beacon request command, and then send it using the mac_data_req
		 * service.
		 */
		BUF_ALLOC(buf, TX);

		dest_addr.mode		= SHORT_ADDR;
		dest_addr.short_addr	= MAC_BROADCAST_ADDR;

		if (pcb->scan_type == MAC_ACTIVE_SCAN) {
			cmd.cmd_id	= MAC_BEACON_REQ;
			src_addr.mode	= NO_PAN_ID_ADDR;
		} else if (pcb->scan_type == MAC_ORPHAN_SCAN) {
			cmd.cmd_id	= MAC_ORPHAN_NOT;
			src_addr.mode	= LONG_ADDR;
			src_addr.long_addr = pib->ext_addr;
		}

		mac_gen_cmd(buf, &cmd);
		mac_gen_cmd_header(buf, &hdr, false, &src_addr, &dest_addr);
		mac_tx_handler(buf, &hdr.dest_addr, false, false, hdr.dsn, ZIGBEE_INVALID_HANDLE);

		/* set the callback timer */
		duration = (pcb->scan_type == MAC_ACTIVE_SCAN) ?
				MAC_SCAN_TIME(pcb->duration) :
				aMacResponseWaitTime;
		ctimer_set(&pcb->mlme_tmr, duration, mac_scan, NULL);
	} else {
		pcb->mac_state = MLME_IDLE;

		/* Send the nwk scan confirm */
		scan_conf.scan_type     = pcb->scan_type;
		scan_conf.energy_list   = NULL;

		if (pcb->scan_type == MAC_ACTIVE_SCAN) {
			scan_conf.status = MAC_SUCCESS;
		} else if (pcb->scan_type == MAC_ORPHAN_SCAN) {
			scan_conf.status = (pcb->coor_realign_rcvd) ?
						MAC_SUCCESS :
						MAC_NO_BEACON;
			pcb->coor_realign_rcvd = false;
		}

		/* restore the original pan ID */
		pib->pan_id = pcb->original_pan_id;
		mac_scan_conf(&scan_conf);
	}
}
