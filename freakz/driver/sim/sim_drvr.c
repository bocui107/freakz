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
    \defgroup simdrvr Driver - SIM
    \file sim_drvr.c
    \ingroup simdrvr
    \brief Simulator driver

    This is the test driver that will simulate the actions of an
    actual radio chip.
*/
/*******************************************************************/
#include "contiki.h"
#include "freakz.h"
#include <sys/time.h>

/* Main process for simulation driver */
PROCESS(drvr_process, "Test Driver Process");

extern process_event_t event_mac_rx;

static U8 rx_buf[SIM_DRVR_MAX_BUF_SIZE]; /* Receive buffer for incoming frames */
static U8 tx_buf[SIM_DRVR_MAX_BUF_SIZE]; /* Tx buffer for outgoing frames */
static U8 rx_len;                        /* Received frame length */
static U8 tx_len;                        /* Transmitted frame length */
static U8 channel;                       /* Current channel we are using */

/*
 * Init the simulated driver. We need to initialize the variables,
 * start the driver process, and also seed the random number generator.
 */
void drvr_init()
{
	struct timeval tv;

	rx_len = 0;
	tx_len = 0;

	process_start(&drvr_process, NULL);

	/*
	 * this is a method of generating a random number based on the time
	 * of day. the micro-seconds portion of time of day is used to seed
	 * the random number generator. After that, all random numbers are
	 * accessed by a call to drvr_get_rand().
	 */
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
}

void drvr_set_pan_id(U16 pan_id)
{
}

void drvr_set_short_addr(U16 addr)
{
}

void drvr_set_ext_addr(U64 addr)
{
}

void drvr_set_coord(bool coord)
{
}

void drvr_set_frm_pend(bool pend)
{
}

/*
 * Get the simulated clear channel assessment. Return true if the CCA is
 * below the clear channel assessment threshold. Otherwise return false.
 */
bool drvr_get_cca()
{
	/* always return true for the simulation. */
	return true;
}

/* Get a random number from the driver. */
U16 drvr_get_rand()
{
	U16 tmp;

	/*
	 * we don't want to have a zero value for our random number because
	 * its used too much in the stack.
	 */
	do {
		tmp = (U16)rand();
	} while (tmp == 0);

	return tmp;
}

/*
 * Set the channel in the simulated driver. This function doesn't really
 * do anything other than track what channel we're on.
 */
U8 drvr_set_channel(U8 chan)
{
	DBG_PRINT("TEST_DRIVER: Channel set to %02d.\n", chan);
	channel = chan;
	return 1;
}

/* Get the current channel setting */
U8 drvr_get_channel()
{
	return channel;
}

/* Emulate sampling a channel's RSSI */
U8 drvr_get_ed()
{
	U8 ed;

	ed = drvr_get_rand() % 51;

	return (ed);
}

/*
 * Write into the rx buffer. This will be used by the sim interface to
 * simulate data arriving into the rx buffer.
 */
void drvr_write_rx_buf(U8 *buf, U8 len)
{
	rx_len = len - 2;
	memcpy(rx_buf, buf + 1, rx_len);
}

/*
 * Read data out of the tx buffer into a test buffer. This will be used by the
 * sim interface to simulate data arriving from the sim driver.
 */
U8 drvr_read_tx_buf(U8 *buf)
{
	U8 tmp;

	if (tx_len) {
		/* remove 1 byte since we don't want the length byte */
		memcpy(buf, tx_buf + 1, tx_len - 1);
		tmp = tx_len;
		tx_len = 0;
		return tmp;
	} else
		return 0;
}

/*
 * Simulates the sending of data. For the sim interface, the transmission of
 * data means that the data gets sent into the pipe whose destination is
 * the simulator.
 */
U8 drvr_tx(const buffer_t *buf)
{
	DBG_PRINT_RAW("DRVR_TX: ");
	debug_dump_buf(buf->dptr, buf->len);
	tx_len = buf->len;
	sim_pipe_data_out(buf->dptr, buf->len);

	DBG_PRINT("TEST_DRIVER: Frame transmitted.\n");
	return 1;
}

/*
 * Check that the frame's dest addr is for us or a broadcast address. Otherwise
 * every frame sent by the simulator to us will trigger an interrupt. This
 * function will allow us to discard frames not meant for us.
 */
static bool drvr_check_addr(buffer_t *buf)
{
	buffer_t *tmp;
	mac_hdr_t hdr;
	U8 index;
	mac_pib_t *pib = mac_pib_get();

	BUF_ALLOC(tmp, TX);
	index = tmp->index;
	memcpy(tmp, buf, sizeof(buffer_t));
	tmp->index = index;
	mac_parse_hdr(tmp, &hdr);
	buf_free(tmp);


	if ((hdr.mac_frm_ctrl.frame_type == MAC_BEACON) ||
	    (hdr.mac_frm_ctrl.frame_type == MAC_ACK)) {
		return true;
	} else if (hdr.dest_addr.mode == SHORT_ADDR) {
		if ((hdr.dest_addr.short_addr == pib->short_addr) ||
		    (hdr.dest_addr.short_addr == MAC_BROADCAST_ADDR))
			return true;
	} else if (hdr.dest_addr.mode == LONG_ADDR) {
		if ((hdr.dest_addr.long_addr == pib->ext_addr))
			return true;
	}

	return false;
}

/* This is the receive interrupt service routine for the sim driver */
void drvr_rx_isr()
{
	buffer_t *buf;

	if (rx_len > aMaxPHYPacketSize)
	{
		/*
		 * don't need to worry about buffer overflow attacks in
		 * the sim, but hey.. why not add a check just in case I
		 * try to hack myself.
		 */
		return;
	}

	BUF_ALLOC(buf, RX);

	/*
	 * copy data into the buffer starting from the back. it will be easier
	 * to forward in case we're not the final destination.
	 */
	buf->dptr = &buf->buf[aMaxPHYPacketSize - rx_len];
	buf->len = rx_len;

	/* void *memcpy(void *dest, const void *src, size_t n) */
	memcpy(buf->dptr, rx_buf, rx_len);

	if (drvr_check_addr(buf)) {
		DBG_PRINT("TEST_DRIVER: Rx Intterupt Received.\n");
		mac_queue_buf_insert(buf);

		rx_len = 0;
		process_poll(&drvr_process);
	} else
		buf_free(buf);
}

/*
 * This is the process thread that will handle the receive event. It will
 * just post an event to the mac process when a receive event occurs.s
 */
PROCESS_THREAD(drvr_process, ev, data)
{
	PROCESS_BEGIN();

	while (1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		process_post(&mac_process, event_mac_rx, NULL);
	}

	PROCESS_END();
}
