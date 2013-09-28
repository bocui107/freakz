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
    \defgroup hw_at90usb Hardware - AT90USB
    \file hw.c
    \ingroup hw_at90usb
*/
/*******************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "at90usb.h"
#include "types.h"

/**************************************************************************/
/*!
    Initialize the hardware.
    1) Turn off the watchdog timer (in AVR MCUs)
    2) Configure the PLL
    3) Enable the USB and relevant peripherals
    4) Enable the global interrupt.
*/
/**************************************************************************/
void hw_init()
{
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    SET_DEVICE_MODE();

    // config PLL and turn it on
    PLLCSR  = (1 << PLLP1) | (1 << PLLP0);
    PLLCSR |= (1 << PLLE);
        while (!PLL_LOCKED);

    // enable the USB block
    USBCON |= ((1 << USBE) | (1 << OTGPADE));

    // unfreeze the clock
    USBCON &= ~(1 << FRZCLK);

    // set the speed
    UDCON &= ~(1 << LSM);

    // set the interrupts: vbus, suspend, and end of reset
    USBCON |= (1 << VBUSTE);
    UDIEN  |= (1 << SUSPE) | (1 << EORSTE);

    // turn on the global interrupts
    sei();
}

/**************************************************************************/
/*!
    Returns a byte stored in the flash. Some MCUs can't access the flash
    memory from a standard pointer so they need a special function to handle it.
*/
/**************************************************************************/
U8 hw_flash_get_byte(U8 *addr)
{
    return pgm_read_byte(addr);
}

/**************************************************************************/
/*!
    Disable global interrupts
*/
/**************************************************************************/
void hw_intp_disable()
{
    cli();
}

/**************************************************************************/
/*!
    Enable global interrupts
*/
/**************************************************************************/
void hw_intp_enable()
{
    sei();
}
