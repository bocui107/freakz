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
/*******************************************************************
    Author: Christopher Wang

    Title: ep_test.h

    Description:
*******************************************************************/
#ifndef TEST_CMD_H
#define TEST_CMD_H

#include "zigbee.h"

#define EP_TEST_EP                  25
#define EP_TEST_PROFILE_ID          33
#define EP_TEST_OUT_CLUST           0
#define EP_TEST_IN_CLUST            1
#define EP_TEST_SET_LONG_XFER_CLUST 2
#define EP_TEST_MAX_RX_QUEUE        3
#define EP_TEST_MAX_CONF_QUEUE      3

/*************** rx handling *****************/
typedef struct _ep_test_rx_entry_t
{
    struct _ep_test_rx_entry_t *next;
    bool alloc;
    buffer_t *buf;
    U8 dest_ep;
    U16 dest_clust;
    U8 src_ep;
    U16 src_clust;
    U16 src_addr;
} ep_test_rx_entry_t;

typedef struct
{
    U16 clust_id;
    void (*handler)(const ep_test_rx_entry_t *entry);
} ep_test_rx_handler_t;

/*************** confirm queue *****************/
typedef struct _ep_test_conf_entry_t
{
    struct _ep_test_conf_entry_t *next;
    bool alloc;
    U8 status;
    U8 handle;
    U8 type;
} ep_test_conf_entry_t;

/*************** tx structs*****************/
typedef struct
{
    buffer_t *buf;
    U8  len;
    U8  src_ep;
    U16 dest_addr;
    U8  dest_ep;
    U16 clust;
    U8  tx_opt;
    U8  handle;
} ep_test_data_req_t;

typedef struct
{
    ep_test_data_req_t req;
} ep_test_pcb_t;

PROCESS_NAME(ep_test_process);

// ep_test
void ep_test_init();
void ep_test_rx(buffer_t *buf, aps_hdr_t *hdr);
void ep_test_conf(U8 status, U8 handle, U8 type);
void ep_test_tx(U8 *data, U8 len, U8 handle, U16 dest_addr, U16 dest_clust, U8 tx_opt);
void ep_test_reg_rx_cb(void (*rx_handler)(U8 *, U8, U16, U8, U16));
void ep_test_reg_conf_cb(void (*conf_handler)(U8, U8, U8));

// ep_test_rx_queue
void ep_test_rx_queue_init();
ep_test_rx_entry_t *ep_test_rx_queue_alloc();
void ep_test_rx_queue_free(ep_test_rx_entry_t *entry);
ep_test_rx_entry_t *ep_test_rx_queue_pop();

// ep_test_conf_queue
void ep_test_conf_queue_init();
ep_test_conf_entry_t *ep_test_conf_alloc();
void ep_test_conf_free(ep_test_conf_entry_t *entry);
ep_test_conf_entry_t *ep_test_conf_queue_pop();

#endif // TEST_CMD_H
