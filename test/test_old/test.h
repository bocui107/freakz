/*******************************************************************
    Copyright (C) 2008 Christopher Wang

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

    Email: chris@freaklabs.org
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title: test.h

    Description:
    This holds commonly used variables and macros in the test programs.
*******************************************************************/
#ifndef TEST_H
#define TEST_H

#include "zigbee.h"

PROCESS_NAME(test_process);
PT_THREAD(test_data_in(struct pt *pt));
PT_THREAD(test_data_out(struct pt *pt));
PT_THREAD(test_data_out_ack(struct pt *pt));
PT_THREAD(test_data_in_rte1(struct pt *pt));
PT_THREAD(test_data_in_rte2(struct pt *pt));
PT_THREAD(test_brc_rtr(struct pt *pt));
PT_THREAD(test_brc_rtr1(struct pt *pt));
PT_THREAD(test_indirect(struct pt *pt));
PT_THREAD(test_nwk_disc(struct pt *pt));
PT_THREAD(test_nwk_disc1(struct pt *pt));
PT_THREAD(test_nwk_join1(struct pt *pt));
PT_THREAD(test_nwk_join2(struct pt *pt));

void test_parse_rx_data(buffer_t *buf, aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr);
void test_gen_ack(buffer_t *buf, mac_hdr_t *hdr_in, bool frm_pend);
bool test_check_ack(buffer_t *buf);

// auto checking
void test_check_load_aps_hdr(aps_hdr_t *hdr);
void test_check_load_nwk_hdr(nwk_hdr_t *hdr);
void test_check_load_mac_hdr(mac_hdr_t *hdr);
void test_check_load_nwk_cmd(nwk_cmd_t *cmd);
void test_check_load_mac_cmd(mac_cmd_t *cmd);
void test_check_load_pan_descr(pan_descr_t *desc);
void test_check_load_payload(U8 *data, U8 size);
void test_check_load_frm(aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr, U8 *payload, U8 payload_size);

void test_check_aps(aps_hdr_t *hdr_in);
void test_check_nwk(nwk_hdr_t *hdr_in);
void test_check_mac(mac_hdr_t *hdr_in);
void test_check_nwk_cmd(nwk_cmd_t *cmd_in);
void test_check_mac_cmd(mac_cmd_t *cmd_in);
void test_check_pan_descr(pan_descr_t *desc_in);
void test_check_frm(buffer_t *buf);
void test_check_frm_hdrs(aps_hdr_t *aps_hdr_in, nwk_hdr_t *nwk_hdr_in, mac_hdr_t *mac_hdr_in);


#endif // TEST_H


