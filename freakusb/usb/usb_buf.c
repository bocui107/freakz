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
    \file buf.c
    \ingroup usb

    This file handles the implementation of a circular fifo. The actual buffers
    are instantiated in the protocol control block and these functions
    are used to control those buffers in a circular fashion.
*/
#include "freakusb.h"
#include <stdio.h>

/* Initialize the specified buffer. */
void usb_buf_init(U8 ep_num, U8 ep_dir)
{
	usb_pcb_t *pcb = usb_pcb_get();

	pcb->fifo[ep_num].ep_dir    = ep_dir;
	pcb->fifo[ep_num].len       = 0;
}

/*
 * Read one byte out of the specified buffer. This function will return the byte
 * located at the array index of the read pointer, and then increment the read pointer index.
 * If the read pointer exceeds the maximum buffer size, it will roll over to zero.
 */
U8 usb_buf_read(U8 ep_num)
{
	usb_pcb_t *pcb = usb_pcb_get();
	U8 data;

	data = pcb->fifo[ep_num].buf[pcb->fifo[ep_num].rd_ptr];
	pcb->fifo[ep_num].rd_ptr = (pcb->fifo[ep_num].rd_ptr + 1) % MAX_BUF_SZ;
	pcb->fifo[ep_num].len--;
	return data;
}

/*
 * Write one byte into the specified buffer. This function will write one
 * byte into the array index specified by the write pointer and increment
 * the write index. If the write index exceeds the max buffer size, then it
 * will roll over to zero.
 */
void usb_buf_write(U8 ep_num, U8 data)
{
	usb_pcb_t *pcb = usb_pcb_get();

	pcb->fifo[ep_num].buf[pcb->fifo[ep_num].wr_ptr] = data;
	pcb->fifo[ep_num].wr_ptr = (pcb->fifo[ep_num].wr_ptr + 1) % MAX_BUF_SZ;
	pcb->fifo[ep_num].len++;
}

/* Clear the fifo read and write pointers and set the length to zero. */
void usb_buf_clear_fifo(U8 ep_num)
{
	usb_pcb_t *pcb = usb_pcb_get();

	pcb->fifo[ep_num].rd_ptr = 0;
	pcb->fifo[ep_num].wr_ptr = 0;
	pcb->fifo[ep_num].len = 0;
}

/*
 * Return the number of the first fifo that has the specified direction
 * and also has pending data to be processed or transmitted. If no data is
 * pending, it will return a value of 0xFF.
 */
U8 usb_buf_data_pending(U8 ep_dir)
{
	U8 i;
	usb_pcb_t *pcb = usb_pcb_get();

	/* start from ep 1 since we aren't checking the ctrl ep */
	for (i = 1; i < (NUM_EPS + 1); i++) {
		if ((pcb->fifo[i].ep_dir == ep_dir) && (pcb->fifo[i].len != 0))
			return i;
	}
	return 0xFF;
}
