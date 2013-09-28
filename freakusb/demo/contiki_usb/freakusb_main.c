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
    \file freakusb_main.c
    \ingroup
*/
/*******************************************************************/
#include <stdio.h>
#include "freakusb_main.h"
#include "freakusb.h"
#include "test_avr_ravenusb.h"
#include "contiki.h"

FILE file_str = FDEV_SETUP_STREAM(freakusb_putchar, NULL, _FDEV_SETUP_WRITE);

/*********************** Process Declarations **********************/
PROCESS(freakusb_process, "FreakUSB Process");
/*******************************************************************/

/**************************************************************************/
/*!
    This is the putchar function that is used by avr-libc's printf. We need
    to hook this function into the stdout file stream using the FDEV_SETUP_STREAM
    macro in avr-libc. Once the stream is set up, we hook the stream to stdout
    and we can do printfs via USB.
*/
/**************************************************************************/
int freakusb_putchar(char c, FILE *unused)
{
    usb_pcb_t *pcb = usb_pcb_get();

    if (!(pcb->flags & (1<<ENUMERATED)))
    {
        return 0;
    }

    if (c == '\n')
    {
        usb_buf_write(EP_1, '\n');
        usb_buf_write(EP_1, '\r');
    }
    else
    {
        usb_buf_write(EP_1, (U8)c);
    }
    ep_write(EP_1);
    return 0;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void freakusb_pollhandler(void)
{
    usb_poll();
    process_poll(&freakusb_process);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
PROCESS_THREAD(freakusb_process, ev, data_proc)
{

    PROCESS_POLLHANDLER(freakusb_pollhandler());

    PROCESS_BEGIN();
    usb_init();
    hw_init();
    cdc_init();

    // TODO: Implement this when we decide to accept commands over the USB
    cdc_reg_rx_handler(test_avr_usb_rx_handler);

    // hook the putchar function to the printf and use it for stdout
    stdout = &file_str;

    // kick off the polling function
    process_poll(&freakusb_process);

    while (1)
    {
        PROCESS_YIELD();
    }

    PROCESS_END();
}
