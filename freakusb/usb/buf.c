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
    \file buf.c
    \ingroup usb

    This file handles the implementation of a circular fifo. The actual buffers
    are instantiated in the protocol control block and these functions
    are used to control those buffers in a circular fashion.
*/
/*******************************************************************/
#include "freakusb.h"

/**************************************************************************/
/*!
    Initialize the specified buffer.
*/
/**************************************************************************/
void buf_init(U8 ep_num, U8 ep_dir)
{
    usb_pcb_t *pcb = usb_pcb_get();

    pcb->fifo[ep_num].ep_dir    = ep_dir;
    pcb->fifo[ep_num].len       = 0;
}

/**************************************************************************/
/*!
    Read one byte out of the specified buffer. This function will return the byte
    located at the array index of the read pointer, and then increment the read pointer index.
    If the read pointer exceeds the maximum buffer size, it will roll over to zero.
*/
/**************************************************************************/
U8 buf_read(U8 ep_num)
{
    usb_pcb_t *pcb = usb_pcb_get();
    U8 data;

    data    = pcb->fifo[ep_num].buf[pcb->fifo[ep_num].rd_ptr];
    pcb->fifo[ep_num].rd_ptr = (pcb->fifo[ep_num].rd_ptr + 1) % MAX_BUF_SZ;
    pcb->fifo[ep_num].len--;
    return data;
}

/**************************************************************************/
/*!
    Write one byte into the specified buffer. This function will write one
    byte into the array index specified by the write pointer and increment
    the write index. If the write index exceeds the max buffer size, then it
    will roll over to zero.
*/
/**************************************************************************/
void buf_write(U8 ep_num, U8 data)
{
    usb_pcb_t *pcb = usb_pcb_get();

    pcb->fifo[ep_num].buf[pcb->fifo[ep_num].wr_ptr] = data;
    pcb->fifo[ep_num].wr_ptr = (pcb->fifo[ep_num].wr_ptr + 1) % MAX_BUF_SZ;
    pcb->fifo[ep_num].len++;
}

/**************************************************************************/
/*!
    Clear the fifo read and write pointers and set the length to zero.
*/
/**************************************************************************/
void buf_clear_fifo(U8 ep_num)
{
    usb_pcb_t *pcb = usb_pcb_get();

    pcb->fifo[ep_num].rd_ptr = 0;
    pcb->fifo[ep_num].wr_ptr = 0;
    pcb->fifo[ep_num].len = 0;
}

/**************************************************************************/
/*!
    Return the number of the first fifo that has the specified direction
    and also has pending data to be processed or transmitted. If no data is
    pending, it will return a value of 0xFF.
*/
/**************************************************************************/
U8 buf_data_pending(U8 ep_dir)
{
    U8 i;
    usb_pcb_t *pcb = usb_pcb_get();

    // start from ep 1 since we aren't checking the ctrl ep
    for (i=1; i<(NUM_EPS+1); i++)
    {
        if ((pcb->fifo[i].ep_dir == ep_dir) && (pcb->fifo[i].len != 0))
        {
            return i;
        }
    }
    return 0xFF;
}
