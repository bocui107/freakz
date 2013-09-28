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

    Originally written by Christopher Wang.
    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title: test_zdo.h

    Description:
    This is the main header file for test_zdo.c.
*******************************************************************/
#ifndef TEST_ZDO_H
#define TEST_ZDO_H

#define MAX_RESP_DATA_SIZE 100

process_event_t event_test_proc;

PROCESS_NAME(test_process);
void test_zdo_check_load_req(zdo_req_t *req, U8 *data, U8 size);
void test_zdo_check_req_resp(buffer_t *buf, U16 clust_id);
void test_zdo_check_load_resp(zdo_resp_t *resp, U8 *data, U8 size);

// test_zdo_gen_req
void test_zdo_gen_nwk_addr_req();
void test_zdo_gen_ieee_addr_req();
void test_zdo_gen_node_desc_req();
void test_zdo_gen_pwr_desc_req();
void test_zdo_gen_simple_desc_req();
void test_zdo_gen_active_ep_req();
void test_zdo_gen_match_desc_req();
void test_zdo_gen_dev_annce_req();

// test_zdo_check_req
void test_zdo_check_nwk_addr_req(buffer_t *buf);
void test_zdo_check_ieee_addr_req(buffer_t *buf);
void test_zdo_check_node_desc_req(buffer_t *buf);
void test_zdo_check_pwr_desc_req(buffer_t *buf);
void test_zdo_check_simple_desc_req(buffer_t *buf);
void test_zdo_check_active_ep_req(buffer_t *buf);
void test_zdo_check_match_desc_req(buffer_t *buf);
void test_zdo_check_dev_annce_req(buffer_t *buf);

// test_zdo_req_handlers
void test_zdo_nwk_addr_req_handler();
void test_zdo_node_desc_req_handler();
void test_zdo_pwr_desc_req_handler();
void test_zdo_simple_desc_req_handler();
void test_zdo_active_ep_req_handler();
void test_zdo_match_desc_req_handler();
void test_zdo_dev_annce_req_handler();

// test_zdo_check_resp
void test_zdo_check_addr_resp(buffer_t *buf);
void test_zdo_check_node_desc_resp(buffer_t *buf);
void test_zdo_check_pwr_desc_resp(buffer_t *buf);
void test_zdo_check_simple_desc_resp(buffer_t *buf);
void test_zdo_active_ep_resp(buffer_t *buf);
void test_zdo_check_match_desc_resp(buffer_t *buf);
void test_zdo_check_load_resp(zdo_resp_t *resp, U8 *data, U8 size);
#endif


