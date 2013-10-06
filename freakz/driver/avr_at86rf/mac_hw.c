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
/*
 * \file avr_at86rf/mac_hw.c
 * \ingroup atdrvr
 * \brief MAC hardware specific code for AVR/AT86RF23x
 *
 * MAC functions that are hardware specific. Any MAC functions that might
 * be handled by hardware should go in here.
 *
 * Just a note: the MAC was split into two parts because many chips
 * incorporate certain MAC functions in hardware. Hence, I've kept
 * one portion of the MAC as hardware independent such as the MAC
 * management functions. For the MAC data functions, portions of it
 * that are handled in hardware were put into this file which needs
 * to be ported to each particular radio/mcu combo.
 */
#include "contiki.h"
#include "freakz.h"
#include "mac_hw.h"
#include "drvr_avr_at86.h"
#include "at86rf230_registermap.h"

extern process_event_t event_mac_rx;

PROCESS(mac_process, "MAC Process");

/*
 * Set the status in the driver control block. This is called when a new status
 * is available from the driver after a successful/unsuccessful transmission.
 */
void mac_set_status(U8 status)
{
	at86_dcb_t *dcb = drvr_dcb_get();

	dcb->status = status;
	dcb->status_avail = true;
}

/* Set the PHY channel to one of the 16 channels available for 802.15.4 */
void mac_set_channel(U8 channel)
{
	mac_pib_t *pib = mac_pib_get();

	if ((channel >= MAC_PHY_CHANNEL_OFFSET) &&
	    (channel <= (MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS)))
	{
		if (drvr_set_channel(channel) != RADIO_SUCCESS) {
			DBG_PRINT("MAC_HW - Channel not changed successfully.\n");
		} else {
			DBG_PRINT("MAC_HW - New channel set: %02X\n", channel);
			pib->curr_channel = channel;
		}
	}
}

/*
 * Set the short address. This is just a wrapper function for the one
 * found in the driver.
 */
void mac_set_short_addr(U16 addr)
{
	mac_pib_t *pib = mac_pib_get();

	pib->short_addr = addr;
	drvr_set_short_addr(addr);
}

/*
 * Set the PAN ID. This is just a wrapper function for the one
 * found in the driver.
 */
void mac_set_pan_id(U16 pan_id)
{
	mac_pib_t *pib = mac_pib_get();

	pib->pan_id = pan_id;
	drvr_set_pan_id(pan_id);
}

/*
 * Enable the MAC receiver. In case the receiver is ever turned off, ie:
 * the device decides to go to sleep to save power, this function needs
 * to be called to enable it. This function can also be called to disable
 * the receiver. You would set the enb parameter to false.
 */
U8 mac_rx_enb(bool enb, bool auto_ack)
{
	if (enb)
		return (auto_ack) ?
				drvr_set_trx_state(RX_AACK_ON) :
				drvr_set_trx_state(RX_ON);

	return drvr_set_trx_state(TRX_OFF);
}

/*
 * This function is an intermediate function that processes any options before
 * transmitting the actual frame. It parses the frame's tx options and if its an indirect
 * transfer, will send it to the indirect queue. If the frame also requires an ACK,
 * it will buffer it in the retry queue so that the frame can be re-sent in the case
 * that the ack doesn't come.
 */
void mac_tx_handler(buffer_t *buf, address_t *addr, bool indir, bool ack_req, U8 dsn, U8 handle)
{
	if (indir) {
		mac_indir_add(buf, addr, ack_req, dsn, handle);
		return;
	}
	mac_out(buf, ack_req, dsn, handle);
}

/*
 * This is the function that does the actual transmission of the
 * frame. It sends the data to the driver which will then send it over the air.
 * If the channel is busy, then it will back off for a random delay and then
 * retry the transfer. If it exceeds the maximum backoffs, it will abort
 * the transmission and send a data confirm with a failure status.
 * After transmission, if no ack is needed, then a data confirm will immediately
 * get issued to the next higher layer that requested the transmission. If an
 * ack is required, the data confirm won't be sent until a proper ack is received.
 */
void mac_out(buffer_t *buf, bool ack_req, U8 dsn, U8 handle)
{
    U8 status;
    mac_pcb_t *pcb = mac_pcb_get();

    // transmit the frame and record the status
    status = drvr_tx(buf);

    // collect a transmit stat here
    pcb->total_xmit++;

    switch (status)
    {
    case RADIO_SUCCESS:
        if (pcb->mac_state == MLME_ASSOC_REQ)
        {
            ctimer_set(&pcb->mlme_tmr, CLOCK_SECOND/*pib->resp_wait_time*/, mac_poll_req, NULL);
        }
        else
        {
            mac_data_conf(MAC_SUCCESS, handle);
        }
        break;
    case RADIO_NO_ACK:
        mac_data_conf(MAC_NO_ACK, handle);

        // collect a transmit fail stat here
        pcb->total_fail++;
        break;
    default:
        mac_data_conf(MAC_DENIED, handle);
        break;
    }

    buf_free(buf);
}

/*
 * Handle the rx events from the mac process. If the driver receives a valid frame,
 * it will send an event to the mac
 * process. The mac process will then call the event handler which retrieves
 * the frame from the rx queue and parses it. Once parsed, it will be handled
 * according to the frame type. If its a command frame, it gets sent to the
 * command handler, a data frame gets sent to the next higher layer, etc...
 */
static void mac_eventhandler(process_event_t event)
{
	buffer_t *buf;
	mac_hdr_t hdr;
	mac_cmd_t cmd;
	//*buf_out;
	//bool frm_pend;
	mac_pcb_t *pcb = mac_pcb_get();
	mac_pib_t *pib = mac_pib_get();

	if (event == event_mac_rx)
	{
		DBG_PRINT("MAC_EVENTHANDLER: Rx event occurred.\n");

		if ((buf = mac_queue_buf_pop()) != NULL)
		{
			DBG_PRINT_RAW("\n<INCOMING>");

			/* decode the packet */
			mac_parse_hdr(buf, &hdr);
			debug_dump_mac_hdr(&hdr);

			/*
			 * NOTE: Using AUTO-ACK so this portion of the ACK
			 * generation is not needed check if an ack is needed.
			 * if so, then generate an ack and queue it for
			 * transmission. the frm pending bit will be set for
			 * any frame coming from an address that has an indirect
			 * frame for it. this is against the spec, but it will
			 * speed up the ack transmission.
			 *
			 * NOTE: the ack response section may change due to the
			 * tight ack timing requirements.
			 */
#if 0
			if (hdr.mac_frm_ctrl.ack_req)
			{
				BUF_ALLOC(buf_out, TX);
				DBG_PRINT("MAC: ACK Required.\n");
				frm_pend = mac_indir_frm_pend(&hdr.src_addr);
				mac_gen_ack(buf_out, frm_pend, hdr.dsn);
				mac_out(buf_out, false, hdr.dsn, 0);
			}
#endif
			/*
			 * process accordingly. if a scan is in progress, all
			 * frames except for beacon frames will be discarded.
			 */
			switch(hdr.mac_frm_ctrl.frame_type)
			{
			case MAC_COMMAND:
				if (pcb->mac_state != MLME_SCAN)
				{
					/*
					 * need to handle the case that this is
					 * an indirect transfer, which means that
					 * we need to stop the poll timer and
					 * send a status to the poll confirm.
					 */
					if ((pcb->mac_state == MLME_DATA_REQ) &&
					    (hdr.src_addr.mode == SHORT_ADDR) &&
					    (hdr.src_addr.short_addr ==
						pib->coord_addr.short_addr))
					{
						ctimer_stop(&pcb->mlme_tmr);
						mac_poll_conf(MAC_SUCCESS);
					}

					mac_parse_cmd(buf, &cmd);
					mac_cmd_handler(&cmd, &hdr);
				}
				buf_free(buf);
				break;
			case MAC_BEACON:
				/* discard the beacon if we're not doing a scan */
				if (pcb->mac_state == MLME_SCAN) {
					mac_parse_beacon(buf, &hdr);
					mac_beacon_notify_ind(buf,
						mac_scan_descr_find_addr(&hdr.src_addr));
				}
				buf_free(buf);
				break;
			case MAC_ACK:
#if 0
				mac_retry_ack_handler(hdr.dsn);

				/*
				 * we need to do some special ops depending
				 * on the state we're in if we get an ACK.
				 */
				if (pcb->mac_state == MLME_ASSOC_REQ)
				{
					if (pcb->assoc_req_dsn == hdr.dsn)
						ctimer_set(&pcb->mlme_tmr,
							   pib->resp_wait_time,
							   mac_poll_req,
							   NULL);
				} else if (pcb->mac_state == MLME_DATA_REQ) {
					if (hdr.mac_frm_ctrl.frame_pending)
						ctimer_set(&pcb->mlme_tmr,
							   aMacMaxFrameTotalWaitTime,
							   mac_poll_timeout,
							   NULL);
				}
#endif
				buf_free(buf);
				break;
			case MAC_DATA:
				if (pcb->mac_state != MLME_SCAN)
				{
					/*
					 * need to handle the case that this
					 * is an indirect transfer, which means
					 * that we need to stop the poll timer
					 * and send a status to the poll confirm
					 */
					if ((pcb->mac_state == MLME_DATA_REQ) &&
					(hdr.src_addr.mode == SHORT_ADDR) &&
					(hdr.src_addr.short_addr == pib->coord_addr.short_addr))
					{
						ctimer_stop(&pcb->mlme_tmr);
						mac_poll_conf(MAC_SUCCESS);
					}
					mac_data_ind(buf, &hdr);
				} else {
					buf_free(buf);
				}
				break;
			default:
				/* TODO: Add a statistic here to capture an error'd rx */
				break;
			}
		}

		/*
		 * there's a possibility that more than one frame is in the
		 * buffer if they came in before this function gets executed.
		 * So process until the queue is empty.
		 */
		if (!mac_queue_is_empty())
		{
			while (process_post(&mac_process, event_mac_rx, NULL) != PROCESS_ERR_OK)
				;
		}
	}
}

/*
 * This is the main process for the mac layer. Actually, its more like
 * a gateway into the protocol stack. All rx data must post an  event to this process.
 */
PROCESS_THREAD(mac_process, ev, data)
{
	PROCESS_BEGIN();

	while (1)
	{
		PROCESS_WAIT_EVENT();
		mac_eventhandler(ev);
	}

	PROCESS_END();
}
