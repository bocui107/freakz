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
    \file test_avr.c
    \ingroup
*/
#include <string.h>
#include "freakz.h"
#include "test_avr_ravenusb.h"
#include "drvr_avr_at86.h"
#include "freakusb.h"
#include "test_app.h"

static U8 msg[MAX_MSG_SIZE];
static U8 *msg_ptr;

void test_avr_init()
{
	test_app_init();

	/* init the msg_ptr */
	msg_ptr = msg;
}

void test_avr_usb_rx_handler()
{
	U8 i, c, ep_num, len;
	usb_pcb_t *pcb = usb_pcb_get();

	/* get the ep number of any endpoint with pending rx data */
	ep_num = usb_buf_data_pending(DIR_OUT);
	if (ep_num != 0xFF)
	{
		/* get the length of data in the OUT buffer */
		len = pcb->fifo[ep_num].len;

		for (i = 0; i < len; i++)
		{
			c = usb_buf_read(ep_num);

			switch (c)
			{
			case '\r':
				/*
				 * terminate the msg and reset the msg ptr. then send
				 * it to the handler for processing.
				 */
				*msg_ptr = '\0';
				DBG_PRINT("\n\r");
				test_app_parse((char *)msg);
				msg_ptr = msg;
				break;
			case '\b':
				if (msg_ptr != msg) {
					usb_buf_write(EP_1, c);
					msg_ptr--;
				}
				break;

			default:
				usb_buf_write(EP_1, c);
				*msg_ptr++ = c;
				break;
			}
		}
		pcb->flags |= (1 << TX_DATA_AVAIL);
	}
}
