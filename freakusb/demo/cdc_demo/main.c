/*******************************************************************
    Copyright (C) 2008 FreakLabs

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*!
    \defgroup cdc_demo CDC Demo
    \file main.c
    \ingroup cdc_demo
*/
/*******************************************************************/
#include <stdio.h>
#include "main.h"
#include "freakusb.h"

FILE file_str = FDEV_SETUP_STREAM(demo_putchar, NULL, _FDEV_SETUP_WRITE);

/**************************************************************************/
/*!
    This is the putchar function that is used by avr-libc's printf. We need
    to hook this function into the stdout file stream using the FDEV_SETUP_STREAM
    macro in avr-libc. Once the stream is set up, we hook the stream to stdout
    and we can do printfs via USB.
*/
/**************************************************************************/
int demo_putchar(char c, FILE *unused)
{
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
    This is the rx handling function. It will handle any incoming data from
    the USB. This function needs to be registered with the CDC class since
    the CDC doesn't know what to do with received data.
*/
/**************************************************************************/
void rx()
{
    U8 i, ep_num, len;
    usb_pcb_t *pcb = usb_pcb_get();

    // get the ep number of any endpoint with pending rx data
    if ((ep_num = usb_buf_data_pending(DIR_OUT)) != 0xFF)
    {
        // get the length of data in the OUT buffer
        len = pcb->fifo[ep_num].len;

        // read out the data in the buffer and echo it back to the host. Just for
        // fun, I put in an example of printf usage here whenever you hit return
        for (i=0; i<len; i++)
        {
            U8 c;

            if ((c = usb_buf_read(ep_num)) == '\r')
            {
                printf("\nWe're going to the next line, baby!\n");
            }
            else
            {
                usb_buf_write(EP_1,  c);
            }
        }
        pcb->flags |= (1 << TX_DATA_AVAIL);
    }
}

/**************************************************************************/
/*!
    This is the main function. Duh.
*/
/**************************************************************************/
int main()
{
    usb_init();
    hw_init();

	// init the class driver here
    cdc_init();

    // register the rx handler function with the cdc
    cdc_reg_rx_handler(rx);

    // hook the putchar function into the printf stdout filestream. This is needed
    // for printf to work.
    stdout = &file_str;

    // and off we go...
    while (1)
    {
        usb_poll();
    }
}
