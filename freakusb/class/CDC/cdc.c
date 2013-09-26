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
    \defgroup cdc_class USB CDC Class
    \file cdc.c
    \ingroup cdc_class
*/
/*******************************************************************/
#include <stdio.h>
#include "freakusb.h"
#include "cdc.h"

// This is the line coding array that sets the baud rate and other modem
// communications properties. This will be used if you are implementing
// an actual USB to serial bridge where you connect to the MCU UART. However
// if you are just transferring data back and forth to the PC from the MCU
// without going through a UART, then the line coding doesn't have much meaning
// since the flow control is handled by the USB protocol.
static U8 line_code[] =
{
    0x00,               // baud rate: 0x0001C200 (115200)
    0xC2,
    0x01,
    0x00,
    FMT_1_STOP_BIT,     // format
    PARITY_NONE,        // parity
    8                   // num bits
};

// this is the rx handler callback function. it gets registered by the application program
// and will handle any incoming data.
static void (*rx_handler)();

/**************************************************************************/
/*!
    This is the class specific request handler for the USB Communications Device
    Class (CDC). Currently, this class driver only support the Virtual COM Port
    feature of the CDC.
*/
/**************************************************************************/
void cdc_req_handler(req_t *req)
{
    U8 i;
    usb_pcb_t *pcb = usb_pcb_get();

    switch (req->req)
    {
    case GET_LINE_CODING:
        if (req->type & (DEVICE_TO_HOST | TYPE_CLASS | RECIPIENT_INTF))
        {
            // send the line coding to the host
            for (i=0; i<LINE_CODE_SZ; i++)
            {
                usb_buf_write(EP_CTRL, line_code[i]);
            }
            ep_write(EP_CTRL);
        }
        break;

    case SET_LINE_CODING:
        if (req->type & (HOST_TO_DEVICE | TYPE_CLASS | RECIPIENT_INTF))
        {
            // wait for the setup data to be sent to the control endpoint
            while (pcb->fifo[EP_CTRL].len == 0)
            {
                // keep the nop for a place to set a breakpoint on and to make it obvious we're
                // waiting for something.
                asm("nop");
            }

            // clear the setup flag if needed
            pcb->flags &= ~(1<<SETUP_DATA_AVAIL);

            // send out a zero-length packet to ack to the host that we received
            // the new line coding
            ep_send_zlp(EP_CTRL);

            // set the new line code. the first 8 bytes in the fifo are just
            // for the setup packet so we want to write the next 7 bytes for the
            // line code.
            for (i=0; i<LINE_CODE_SZ; i++)
            {
                line_code[i] = usb_buf_read(EP_CTRL);
            }
        }
        break;

    case SET_CTRL_LINE_STATE:
        if (req->type & (HOST_TO_DEVICE | TYPE_CLASS | RECIPIENT_INTF))
        {
            ep_send_zlp(EP_CTRL);
        }
        break;

    default:
        ep_set_stall(EP_CTRL);
        break;
    }
}

/**************************************************************************/
/*!
    This is the rx data handler for the CDC class driver. This should be
    implemented by the user depending on what they want to do with the incoming
    virtual COM data.
*/
/**************************************************************************/
void cdc_rx_handler()
{
    if (rx_handler)
    {
        rx_handler();
    }
}

/**************************************************************************/
/*!
    Initialize the endpoints according to the CDC class driver. The class
    driver specifies a BULK IN, BULK OUT, and INTERRUPT IN endpoint. We will
    usually set this after the host issues the set_configuration request.
*/
/**************************************************************************/
void cdc_ep_init()
{
    // setup the endpoints
    ep_config(EP_1, BULK, DIR_IN, PKTSZ_64);
    ep_config(EP_2, INTP, DIR_IN, PKTSZ_64);
    ep_config(EP_3, BULK, DIR_OUT, PKTSZ_64);
}

/**************************************************************************/
/*!
    This is the CDC's rx handler. You need to register your application's rx
    function here since the CDC doesn't know what to do with received data.
*/
/**************************************************************************/
void cdc_reg_rx_handler(void (*rx)())
{
    if (rx)
    {
        rx_handler = rx;
    }
}

/**************************************************************************/
/*!
    Initialize the CDC class driver. We basically register our init, request handler,
    and rx data handler with the USB core.
*/
/**************************************************************************/
void cdc_init()
{
    usb_reg_class_drvr(cdc_ep_init, cdc_req_handler, cdc_rx_handler);
}

