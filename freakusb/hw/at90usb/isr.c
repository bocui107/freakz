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
    \file isr.c
    \ingroup hw_at90usb
*/
/*******************************************************************/
#include <avr/interrupt.h>
#include "freakusb.h"
#include "at90usb.h"

/**************************************************************************/
/*!
    Clear all USB related interrupts.
*/
/**************************************************************************/
void intp_clear_all()
{
    U8 i;

    USBINT = 0;
        UDINT  = 0;

    for (i=0; i<MAX_EPS; i++)
    {
        ep_select(i);
        UEINTX = 0;
    }
}

/**************************************************************************/
/*!
    VBUS Change interrupt handler.
*/
/**************************************************************************/
void intp_vbus_change()
{
    usb_pcb_t *pcb = usb_pcb_get();

    VBUS_INT_CLR();

    if (!pcb->connected)
    {
        pcb->connected = true;

        // enabled the USB voltage regulator
        UHWCON |= (1 << UVREGE);

        // unfreeze the clock and attach to the bus
        USBCON &= ~(1 << FRZCLK);
        UDCON  &= ~(1 << DETACH);
    }
    else
    {
        // if we're connected, but VBUS is gone, then detach
                if (!(USBSTA & (1<<VBUS)))
        {
            // detach from the bus
            UDCON  |= (1 << DETACH);

            // freeze the clock and turn off the USB PLL
            USBCON |= (1 << FRZCLK);
            PLLCSR = 0;

            // disable the USB voltage regulator
            UHWCON &= ~(1 << UVREGE);

            pcb->connected = false;
            pcb->flags = 0;
        }
    }
}

/**************************************************************************/
/*!
    Suspend interrupt handler.
*/
/**************************************************************************/
void intp_suspend()
{
    SUSP_INT_CLR();
    WAKEUP_INT_ENB();
    WAKEUP_INT_CLR();

    // freeze the clock and turn off the USB PLL
    USBCON |= (1 << FRZCLK);
}

/**************************************************************************/
/*!
    Resume interrupt handler.
*/
/**************************************************************************/
void intp_resume()
{
    WAKEUP_INT_DIS();
    RESM_INT_CLR();
    RESM_INT_DIS();
}

/**************************************************************************/
/*!
    Wakeup interrupt handler.
*/
/**************************************************************************/
void intp_wakeup()
{
    // unfreeze the clock
    USBCON &= ~(1 << FRZCLK);

    WAKEUP_INT_CLR();
    WAKEUP_INT_DIS();

    // disable the wakeup until next time we go into suspend
    WAKEUP_INT_DIS();
}

/**************************************************************************/
/*!
    End of Reset interrupt handler. Gets triggered at the end of a bus reset.
*/
/**************************************************************************/
void intp_eor()
{
    EOR_INT_CLR();
    ep_init();
}

/**************************************************************************/
/*!
    This is the ISR that handles communications for the AT90USB. These interrupts
    are endpoint specific and are mostly used for data transfers and communications.
*/
/**************************************************************************/
ISR(USB_COM_vect)
{
    U8 ep_intp_num, intp_src, ep_num;
    usb_pcb_t *pcb;

    cli();

    // get the pcb for later use
    pcb = usb_pcb_get();

    // save off the ep number we just had
    ep_num = UENUM;

    if ((ep_intp_num= ep_intp_get_num()) == 0xFF)
    {
        // no intp number was found. restore the ep number and enable the interrupts
        ep_select(ep_num);
        sei();
        return;
    }

    // select the endpoint number and get the intp src
    ep_select(ep_intp_num);
    if ((intp_src = ep_intp_get_src()) == 0xFF)
    {   
                // no intp source was found. restore the ep number and enable the interrupts
        ep_select(ep_num);
        sei();
        return;
    }

    switch (intp_src)
    {
    case RXSTPI:
        ep_read(ep_intp_num);

        // clear the intp
        RX_SETUP_INT_CLR();
        break;
    case RXOUTI:
        ep_read(ep_intp_num);
        
        // clear the intps
        RX_OUT_INT_CLR();
        FIFOCON_INT_CLR();
        break;
    case TXINI:
        ep_write(ep_intp_num);

        // clear the intps
        TX_IN_INT_CLR();
        break;
    case STALLEDI:
        break;
    case NAKOUTI:
        break;
    case RWAL:
        break;
    case NAKINI:
        break;
    case FIFOCON:
        break;
    default:
        break;
    }

    // restore the endpoint number
    ep_select(ep_num);

    sei();
}

/**************************************************************************/
/*!
    This ISR handles general USB functions on the AT90USB. The interrupts
    are sepearated into a general and communications interrupt. The general
    interrupt handles global USB features such as VBUS detection, suspend, resume
    and bus reset.
*/
/**************************************************************************/
ISR(USB_GEN_vect)
{
    U8 ep_num;

    cli();
        
        // save off the endpoint number we were just on. we'll restore it later
    ep_num = UENUM;

    // suspend detected
    if (SUSP_INT && SUSP_INT_ENABLED)
    {
        intp_suspend();
    }
        
        // vbus status change
    if (VBUS_INT && VBUS_INT_ENABLED)
    {
        intp_vbus_change();
    }

    if (RESM_INT && RESM_INT_ENABLED)
    {
        intp_resume();
    }

    // wakeup detected
    if (WAKEUP_INT && WAKEUP_INT_ENABLED)
    {
        intp_wakeup();
    }

    // end of bus reset
    if (EOR_INT && EOR_INT_ENABLED)
    {
        intp_eor();
    }

    // restore the endpoint number
    ep_select(ep_num);

    sei();
}
