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
    \file sim/mac_hw.c
    \ingroup simdrvr
    \brief MAC hardware specific code for simulator

    MAC functions that are hardware specific. Any MAC functions that might
    be handled by hardware should go in here.
*/
/*******************************************************************/
#include "contiki.h"
#include "freakz.h"

extern process_event_t event_mac_rx;

/* Main process for MAC in simulator */
PROCESS(mac_process, "MAC Process");

void mac_set_pan_id(U16 pan_id)
{
	mac_pib_t *pib = mac_pib_get();
	pib->pan_id = pan_id;
}

/* Set the PHY channel to one of the 16 channels available for 802.15.4 */
void mac_set_channel(U8 channel)
{
	mac_pib_t *pib = mac_pib_get();

	if ((channel >= MAC_PHY_CHANNEL_OFFSET) &&
		(channel <= (MAC_PHY_CHANNEL_OFFSET + MAC_MAX_CHANNELS)))
		pib->curr_channel = channel;
}

void mac_rx_enb(bool enb, bool auto_ack)
{
}

void mac_set_short_addr(U16 addr)
{
	mac_pib_t *pib = mac_pib_get();
	pib->short_addr = addr;
}


/*
 * This function is an intermediate function that processes
 * any options before transmitting the actual frame. It parses
 * the frame's tx options and if its an indirect transfer, will
 * send it to the indirect queue. If the frame also requires an
 * ACK, it will buffer it in the retry queue so that the frame
 * can be re-sent in the case that the ack doesn't come.
 */
void mac_tx_handler(buffer_t *buf, address_t *addr, bool indir, bool ack_req, U8 dsn, U8 handle)
{
	if (indir) {
		mac_indir_add(buf, addr, ack_req, dsn, handle);
		return;
	}

	if (ack_req)
		mac_retry_add(buf, dsn, handle);

	mac_out(buf, ack_req, dsn, handle);
}

/*
 * This is the function that does the actual transmission of the
 * frame. It sends the data to the driver which will then send it
 * over the air. If the channel is busy, then it will back off for
 * a random delay and then retry the transfer. If it exceeds the
 * maximum backoffs, it will abort the transmission and send a data
 * confirm with a failure status. After transmission, if no ack is
 * needed, then a data confirm will immediately get issued to the
 * next higher layer that requested the transmission. If an ack is
 * required, the data confirm won't be sent until a proper ack is received.
 */
void mac_out(buffer_t *buf, bool ack_req, U8 dsn, U8 handle)
{
	U8 i;
	U16 csma_time;
	mac_pib_t *pib = mac_pib_get();
	mac_pcb_t *pcb = mac_pcb_get();

	for (i = 0; i < aMaxCsmaBackoffs; i++)
	{
		/* check if the channel is clear */
		if (drvr_get_cca())
		{
			/* send the frame if the CCA clears */
			drvr_tx(buf);

			/* collect a transmission stat here */
			pcb->total_xmit++;

			/*
			 * if there's no ack request,
			 * then we're finished. free the buf.
			 */
			if (!ack_req) {
				mac_data_conf(MAC_SUCCESS, handle);
				buf_free(buf);
			}
			return;
		} else {
			/*
			 * channel busy. do a busy wait and try again.
			 * Shift left of signed quantity (int) due to
			 * left shift of constant "1". its okay.
			 */
			csma_time = drvr_get_rand() % (U16)((1 << pib->min_be) - 1);
			busy_wait(csma_time);
		}
	}

	/*
	 * exceeded max csma backoffs. clean up and send a
	 * confirm with a fail message.
	 */
	if (ack_req)
	{
		/*
		 * if the ack request is set, then we need to
		 * check the retry queue and remove the entry.
		 */
		mac_retry_rem(dsn);

		/* collect a transmit fail stat here */
		pcb->total_fail++;
	} else
		buf_free(buf);

	mac_data_conf(MAC_CHANNEL_ACCESS_FAILURE, handle);
}

/*
 * Handle the rx events from the mac process. If the driver
 * receives a valid frame, it will send an event to the mac
 * process. The mac process will then call the event handler
 * which retrieves the frame from the rx queue and parses it.
 * Once parsed, it will be handled according to the frame type.
 * If its a command frame, it gets sent to the command handler,
 * a data frame gets sent to the next higher layer, etc...
 */
static void mac_eventhandler(process_event_t event)
{
	buffer_t *buf, *buf_out;
	mac_hdr_t hdr;
	mac_cmd_t cmd;
	bool frm_pend;
	mac_pcb_t *pcb = mac_pcb_get();
	mac_pib_t *pib = mac_pib_get();

	if (event == event_mac_rx)
	{
		DBG_PRINT("MAC_EVENTHANDLER: Rx event occurred.\n");

		buf = mac_queue_buf_pop();
		if (buf) {
			DBG_PRINT_RAW("\n<INCOMING>");
			debug_dump_buf(buf->dptr, buf->len);

			/* decode the packet */
			mac_parse_hdr(buf, &hdr);
			debug_dump_mac_hdr(&hdr);

			/*
			 * check if an ack is needed. if so, then generate
			 * an ack and queue it for transmission. the frm
			 * pending bit will be set for any frame coming from
			 * an address that has an indirect frame for it. this
			 * is against the spec, but it will speed up the ack
			 * transmission.
			 * NOTE: the ack response section may change due to the
			 * tight ack timing requirements.
			 */
			if (hdr.mac_frm_ctrl.ack_req)
			{
				BUF_ALLOC(buf_out, TX);
				DBG_PRINT("MAC: ACK Required.\n");
				frm_pend = mac_indir_frm_pend(&hdr.src_addr);
				mac_gen_ack(buf_out, frm_pend, hdr.dsn);
				mac_out(buf_out, false, hdr.dsn, 0);
			}

			/*
			 * process accordingly. if a scan is in progress,
			 * all frames except for beacon frames will be
			 * discarded.
			 */
			switch(hdr.mac_frm_ctrl.frame_type)
			{
			case MAC_COMMAND:
				if (pcb->mac_state != MLME_SCAN)
				{
					/*
					 * need to handle the case that this is an indirect
					 * transfer, which means that we need to stop the
					 * poll timer and send a status to the poll confirm.
					 */
					if ((pcb->mac_state == MLME_DATA_REQ) &&
					(hdr.src_addr.mode == SHORT_ADDR) &&
					(hdr.src_addr.short_addr == pib->coord_addr.short_addr))
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
				if (pcb->mac_state == MLME_SCAN)
				{
					mac_parse_beacon(buf, &hdr);
					mac_beacon_notify_ind(buf, mac_scan_descr_find_addr(&hdr.src_addr));
				}
				buf_free(buf);
				break;
			case MAC_ACK:
				mac_retry_ack_handler(hdr.dsn);

				/*
				 * we need to do some special ops depending on the
				 * state we're in if we get an ACK.
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
				buf_free(buf);
				break;
			case MAC_DATA:
				if (pcb->mac_state != MLME_SCAN)
				{
					/*
					 * need to handle the case that this is an indirect
					 * transfer, which means that we need to stop the poll
					 * timer and send a status to the poll confirm.
					 */
					if ((pcb->mac_state == MLME_DATA_REQ)  &&
						(hdr.src_addr.mode == SHORT_ADDR) &&
						(hdr.src_addr.short_addr == pib->coord_addr.short_addr))
					{
						ctimer_stop(&pcb->mlme_tmr);
						mac_poll_conf(MAC_SUCCESS);
					}

					mac_data_ind(buf, &hdr);
				} else
					buf_free(buf);
				break;
			default:
				/* TODO: Add a statistic here to capture an error'd rx */
			break;
			}
		}
		/*
		 * there's a possibility that more than one frame is in the
		 * buffer. if they came in before this function gets executed.
		 * So process until the queue is empty.
		 */
		if (!mac_queue_is_empty())
		{
			while (process_post(&mac_process, event_mac_rx, NULL) != PROCESS_ERR_OK)
			{
				;
			}
		}
	}
}

/*
 * This is the main process for the mac layer. Actually, its more like
 * a gateway into the protocol stack. All rx data must post an  event
 * to this process.
 */
PROCESS_THREAD(mac_process, ev, data)
{
	PROCESS_BEGIN();

	while (1) {
		PROCESS_WAIT_EVENT();
		mac_eventhandler(ev);
	}

	/* due to contiki process macro. its okay */
	PROCESS_END();
}
