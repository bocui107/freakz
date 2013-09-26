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
    \defgroup usb USB
    \file usb.c
    \ingroup usb
*/
/*******************************************************************/
#include "freakusb.h"

static usb_pcb_t pcb;

/**************************************************************************/
/*!
    Initialize the USB stack. Actually, we just clear out the protocol control
    block for now.
*/
/**************************************************************************/
void usb_init()
{
    memset(&pcb, 0, sizeof(usb_pcb_t));
}

/**************************************************************************/
/*!
    Get a pointer to the USB stack's protocol control block.
*/
/**************************************************************************/
usb_pcb_t *usb_pcb_get()
{
    return &pcb;
}

/**************************************************************************/
/*!
    Register the class driver
*/
/**************************************************************************/
void usb_reg_class_drvr(void (*class_init)(),
                        void (*class_req_handler)(req_t *req),
                        void (*class_rx_handler)())
{
    pcb.class_req_handler   = class_req_handler;
    pcb.class_init          = class_init;
    pcb.class_rx_handler    = class_rx_handler;
}

/**************************************************************************/
/*!
    This function needs to be polled in the main loop. It will check if there
    are any transfers pending in either the IN or OUT direction as well as if
    there are any pending CONTROL transfers.
*/
/**************************************************************************/
void usb_poll()
{
    U8 ep_num;

    if (pcb.connected)
    {
        // if any setup data is pending, send it to the ctrl handler. we operate
        // the ctrl fifo like a standard buffer instead of a fifo. it makes the
        // operations easier because we can just do a pointer overlay on the buffer.
        if (pcb.flags & (1<<SETUP_DATA_AVAIL))
        {
            // clear the setup flag at the very beginning. later on, we can wait
            // on this flag if there are data stages to the setup transaction.
            pcb.flags &= ~(1<<SETUP_DATA_AVAIL);

            // handle the request
            ctrl_handler();
            usb_buf_clear_fifo(EP_CTRL);
        }

        // don't handle any data transfers on endpoints other than the control endpoint
        // unless the USB is properly enumerated.
        if (pcb.flags & (1<<ENUMERATED))
        {
            // if any rx data is pending, send it to the rx handler
            if (pcb.flags & (1<<RX_DATA_AVAIL))
            {
                if (pcb.class_rx_handler)
                {
                    pcb.class_rx_handler();
                }

                // clear the rx data avail flag now that we're done processing the rx data.
                pcb.flags &= ~(1 << RX_DATA_AVAIL);
            }

            // if any tx data is pending, send it to the endpoint fifo
            if (pcb.flags & (1<<TX_DATA_AVAIL))
            {
                // get the ep number of any fifo with pending tx data
                if ((ep_num = usb_buf_data_pending(DIR_IN)) != 0xFF)
                {
                    // disable the interrupts while we're handling endpoint data
                    // or else we might get data coming into the fifo from a non-USB related interrupt
                    // as we're reading it out of the fifo.
                    hw_intp_disable();
                    ep_write(ep_num);
                    hw_intp_enable();
                }
                pcb.flags &= ~(1<<TX_DATA_AVAIL);
            }
        }
    }
}

