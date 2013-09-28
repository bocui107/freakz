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
    \file ep.c
    \ingroup hw_at90usb
*/
/*******************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "freakusb.h"
#include "at90usb.h"

/**************************************************************************/
/*!
    Select the endpoint number so that we can see the endpoint's associated
    registers.
*/
/**************************************************************************/
void ep_select(U8 ep_num)
{
    UENUM = ep_num;
}

/**************************************************************************/
/*!
    Get the max packet size of the endpoint.
*/
/**************************************************************************/
U8 ep_size_get()
{
    U8 tmp = (UECFG1X &= (7<<EPSIZE0));
    tmp >>= EPSIZE0;
    return (U8)(1 << (tmp + 3));
}

/**************************************************************************/
/*!
    Get the direction of the endpoint.
*/
/**************************************************************************/
U8 ep_dir_get()
{
    return (UECFG0X & 0x1);
}

/**************************************************************************/
/*!
    Get the endpoint type: BULK, CONTROL, INTERRUPT, ISOCHRONOUS
*/
/**************************************************************************/
U8 ep_type_get()
{
    return ((UECFG0X & (0x3 << EPTYPE0)) >> EPTYPE0);
}

/**************************************************************************/
/*!
    Clear the endpoint configuration registers
*/
/**************************************************************************/
void ep_cfg_clear()
{
    UECFG0X = 0;
    UECFG1X = 0;
}

/**************************************************************************/
/*!
    Clear the specified endpoint's enable bit.
*/
/**************************************************************************/
void ep_disable()
{
    UECONX &= ~(1 << EPEN);
}

/**************************************************************************/
/*!
    Configure the endpoint with the specified parameters.
*/
/**************************************************************************/
void ep_config(U8 ep_num, U8 type, U8 dir, U8 size)
{
    // init the direction the fifo
    usb_buf_init(ep_num,  dir);

    // select the endpoint and reset it
    ep_select(ep_num);
    UECONX      &= ~(1 << EPEN);
    UECONX  |= (1 << EPEN);

    // set the type, direction, and size
    UECFG0X |= (type & 0x3) << EPTYPE0;
    UECFG0X |= dir;
    UECFG1X |= (size & 0x7) << EPSIZE0;

    // we're only using single banks for now. double banks can be used on certain EPs only.
    UECFG1X |= (SINGLE & 0x3) << EPBK0;

    // alloc memory for the endpoint
    UECFG1X |= (1 << ALLOC);

    // spin here until the config is okay
    while (!(UESTA0X & (1<<CFGOK)));

    UERST |= (1<<ep_num);
    UERST &= ~(1<<ep_num);

    if (ep_num == EP_CTRL)
    {
        RX_SETUP_INT_ENB();
        RX_OUT_INT_ENB();
    }
    else if (dir == DIR_OUT)
    {
        RX_OUT_INT_ENB();
    }
}

/**************************************************************************/
/*!
    Write into the endpoint's FIFOs. These will be used to transfer data out
    of that particular endpoint to the host.
*/
/**************************************************************************/
void ep_write(U8 ep_num)
{
    U8 i, ep_size, len;
    usb_pcb_t *pcb = usb_pcb_get();

    ep_select(ep_num);
    ep_size = ep_size_get();
    len = pcb->fifo[ep_num].len;

    // make sure that the tx fifo is ready to receive the out data
    if (ep_num == EP_CTRL)
    {
        while (!TX_FIFO_READY);
    }
    else
    {
        while (!RWAL_INT);
    }

    for (i=0; i<len; i++)
    {
        // check if we've reached the max packet size for the endpoint
        if (i == ep_size)
        {
            // we've filled the max packet size so break and send the data
            break;
        }

        UEDATX = usb_buf_read(ep_num);
    }

    if (ep_num == EP_CTRL)
    {
        while (!TX_FIFO_READY);
    }

    // clearing these two will send the data out
    TX_IN_INT_CLR();
    FIFOCON_INT_CLR();
}

/**************************************************************************/
/*!
    Read data from the endpoint's FIFO. This is where data coming into the
    device from the host is stored.
*/
/**************************************************************************/
void ep_read(U8 ep_num)
{
    U8 i, len;
    usb_pcb_t *pcb = usb_pcb_get();

    len = FIFO_BYTE_CNT;

    for (i=0; i<len; i++)
    {
        usb_buf_write(ep_num,  UEDATX);
    }

    if (len > 0)
    {
        pcb->flags |= (ep_num == 0) ? (1<<SETUP_DATA_AVAIL) : (1<<RX_DATA_AVAIL);
    }
}

/**************************************************************************/
/*!
    Send a zero length packet on the specified endpoint. This is usually used
    to terminate a transfer.
*/
/**************************************************************************/
void ep_send_zlp(U8 ep_num)
{
    cli();
    ep_select(ep_num);
    while (!TX_FIFO_READY);
    TX_DATA();
    while (!TX_FIFO_READY);
    sei();
}

/**************************************************************************/
/*!
    Stall the endpoint due to some problem. This function will also set the
    stall flag in the protocol control block.
*/
/**************************************************************************/
void ep_set_stall(U8 ep_num)
{
    usb_pcb_t *pcb = usb_pcb_get();

    pcb->ep_stall |= (1 << ep_num);
    ep_select(ep_num);
    UECONX |= (1 << STALLRQ);
}

/**************************************************************************/
/*!
    This function will clear the stall on an endpoint.
*/
/**************************************************************************/
void ep_clear_stall(U8 ep_num)
{
    usb_pcb_t *pcb = usb_pcb_get();

    pcb->ep_stall &= ~(1 << ep_num);
    ep_select(ep_num);
    UECONX |= (1<<STALLRQC);
}

/**************************************************************************/
/*!
    Reset the data toggle on the specified endpoint.
*/
/**************************************************************************/
void ep_reset_toggle(U8 ep_num)
{
    ep_select(ep_num);
    UECONX |= (1<<RSTDT);
}

/**************************************************************************/
/*!
    Clear all endpoints and initialize ep0 for control transfers.
*/
/**************************************************************************/
void ep_init()
{
    U8 i;

    // disable and clear all endpoints
    for (i=0; i<MAX_EPS; i++)
    {
        ep_select(i);
        ep_disable(i);
        ep_cfg_clear(i);
    }

    // reset all the endpoints
    UERST = 0x7F;
    UERST = 0;

    // configure the control endpoint first since that one is needed for enumeration
    ep_config(EP_CTRL, CONTROL, DIR_OUT, PKTSZ_64);

    // set the rx setup interrupt to received the enumeration interrupts
    ep_select(EP_CTRL);
}

/**************************************************************************/
/*!
    Set the address for the device. This will be called when a SET_ADDR request
    is sent by the host. We can only set the address after we send a ZLP to the host
    informing it that we successfully received the request. Otherwise, we will
    be on a different address when the host ACKs us back on the original address (0).
*/
/**************************************************************************/
void ep_set_addr(U8 addr)
{
    // send out a zlp to ack the set address request
    ep_send_zlp(EP_CTRL);

    // only write the top 7 bits of the address. the 8th bit is for enable
    UDADDR = addr & 0x7F;

    // enable the address after the host acknowledges the frame was sent
    UDADDR |= (1 << ADDEN);
}


/**************************************************************************/
/*!
    Return the ep where an intp occurred. If no intp occurred, then return 0xff.
*/
/**************************************************************************/
U8 ep_intp_get_num()
{
    U8 i;

    for (i=0; i<MAX_EPS; i++)
    {
        if (UEINT & (1<<i))
        {
            return i;
        }
    }
    return 0xFF;
}

/**************************************************************************/
/*!
    Get the endpoint number that the interrupt occurred on. If no interrupt
    is found, return 0xFF.
*/
/**************************************************************************/
U8 ep_intp_get_src()
{
    U8 i;

    // get the intp src
    for (i=0; i<8; i++)
    {
        if ((UEINTX & (1<<i)) && (UEIENX & (1<<i)))
        {
            return i;
        }
    }
    return 0xFF;
}
