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
    \file freakusb.h
    \ingroup usb
*/
/*******************************************************************/
#ifndef FREAKUSB_H
#define FREAKUSB_H

#include <string.h>
#include <stdio.h>
#include "types.h"

/* class specific */
#include "cdc.h"

/* hw specific */
#include "hw.h"

/* config */
#define FLASHMEM            PROGMEM     /* AVR Specific for storing data in flash */
#define MAX_RX_ENTRIES      5
#define MAX_EPS             7           /* AVR Specific for AT90USB USB controller */
#define POWER_SRC           BUS_POWERED
#define MAX_REQUEST_SIZE    32
#define CTRL_IN_REQ_SZ      8

/* eps */
#define EP_CTRL         0
#define EP_1            1
#define EP_2            2
#define EP_3            3
#define EP_4            4
#define EP_5            5
#define EP_6            6
#define EP_7            7

/* packet sizes */
#define PKTSZ_8         0
#define PKTSZ_16        1
#define PKTSZ_32        2
#define PKTSZ_64        3
#define PKTSZ_128       4
#define PKTSZ_256       5
#define PKTSZ_512       6
#define PKTSZ_1024      7

/* xfer type */
#define CONTROL         0
#define ISOCHRONOUS     1
#define BULK            2
#define INTP            3

/* direction */
#define DIR_OUT         0
#define DIR_IN          1

/* token types */
#define PKTTYP_SETUP    0
#define PKTTYP_IN       1
#define PKTTYP_OUT      2

// get requests
#define GET_STATUS          0x00
#define GET_DESCRIPTOR      0x06
#define GET_CONFIGURATION   0x08
#define GET_INTERFACE       0x0A
#define SYNC_FRAME          0x0C

// set requests
#define SET_FEATURE         0x03
#define SET_ADDRESS         0x05
#define SET_DESCRIPTOR      0x07
#define SET_CONFIGURATION   0x09
#define SET_INTERFACE       0x0B
#define CLEAR_FEATURE       0x01

// descriptor types
#define DEV_DESCR           1
#define CFG_DESCR           2
#define STR_DESCR           3
#define INTF_DESCR          4
#define EP_DESCR            5
#define DEV_QUAL_DESCR      6

// request types
#define HOST_TO_DEVICE      0x00
#define DEVICE_TO_HOST      0x80
#define TYPE_STD            0x00
#define TYPE_CLASS          0x20
#define TYPE_VENDOR         0x40
#define RECIPIENT_DEV       0x00
#define RECIPIENT_INTF      0x01
#define RECIPIENT_EP        0x02

// get status requests
#define GET_DEVICE_STATUS   0x80
#define GET_INTF_STATUS     0x81
#define GET_EP_STATUS       0x82

// string indexes
#define LANG_DESC_IDX       0
#define MANUF_DESC_IDX      1
#define PROD_DESC_IDX       2
#define SERIAL_DESC_IDX     3

// control features
#define ENDPOINT_HALT       0
#define REMOTE_WAKEUP       1

// flags defines
#define RX_DATA_AVAIL       0
#define TX_DATA_AVAIL       1
#define SETUP_DATA_AVAIL    2
#define REMOTE_WAKEUP_ENB   3
#define ENUMERATED          4

// power sources
#define SELF_POWERED        1
#define BUS_POWERED         0

/* get desc request */
typedef struct req_t
{
	U8 type;
	U8 req;
	U16 val;
	U16 idx;
	U16 len;
	U8 data[];
} req_t;

/* buffer used for circular fifo */
typedef struct _usb_buffer_t
{
	U8 ep_dir;
	volatile U8 len;    // this may change in an interrupt and the value would be required
	volatile U8 wr_ptr;
	volatile U8 rd_ptr;
	U8 buf[MAX_BUF_SZ];
} usb_buffer_t;

/* protocol control block */
typedef struct _usb_pcb_t
{
	bool connected;
	volatile U8 flags;
	U8 intp_flags;
	U8 cfg_num;
	U8 ep_stall;
	U8 test;
	usb_buffer_t fifo[NUM_EPS];
	void (*class_init)();
	void (*class_req_handler)(req_t *req);
	void (*class_rx_handler)();
} usb_pcb_t;

// prototypes

// usb.c
void usb_init();
usb_pcb_t *usb_pcb_get();
void usb_reg_class_drvr(void (*class_cfg_init)(),
                        void (*class_req_handler)(),
                        void (*class_rx_handler)());
void usb_poll();

// req.c
void ctrl_handler();

// ep.c
void ep_init();
void ep_select(U8 ep_num);
void ep_write_from_flash(U8 ep_num, U8 *data, U8 len);
void ep_write(U8 ep_num);
void ep_write_ctrl(U8 *data, U8 len, bool read_from_flash);
void ep_read(U8 ep_num);
void ep_set_addr(U8 addr);
U8 ep_intp_get_num();
U8 ep_intp_get_src();
void ep_set_stall(U8 ep_num);
void ep_clear_stall(U8 ep_num);
void ep_reset_toggle(U8 ep_num);
void ep_send_zlp(U8 ep_num);
void ep_config(U8 ep_num, U8 type, U8 dir, U8 size);

// desc.c
U8 *desc_dev_get();
U8 desc_dev_get_len();
U8 *desc_cfg_get();
U8 desc_cfg_get_len();
U8 *desc_dev_qual_get();
U8 desc_dev_qual_get_len();
U8 *desc_str_get(U8 index);
U8 desc_str_get_len(U8 index);

// buf
void usb_buf_init(U8 ep_num, U8 ep_dir);
U8 usb_buf_read(U8 ep_num);
void usb_buf_write(U8 ep_num, U8 data);
void usb_buf_clear_fifo(U8 ep_num);
U8 usb_buf_data_pending(U8 ep_dir);
#endif

