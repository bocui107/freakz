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

    Title: test_data_in.c

    Description:
*******************************************************************/
#include "contiki.h"
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"

/********************** Globals *****************************************/
extern bool finished;
bool data_received;

/**************************************************************************
Function Name: test_mac_gen_mac_data

Description:
Generate data to be used in the MAC layer.
**************************************************************************/
static void test_gen_mac_data(mac_hdr_t *hdr)
{
    memset(hdr, 0, sizeof(mac_hdr_t));

    hdr->mac_frm_ctrl.ack_req       = true;
    hdr->dest_pan_id                = 0x1111;
    hdr->dest_addr.mode             = SHORT_ADDR;
    hdr->dest_addr.short_addr       = 0x1234;
    //hdr->dest_addr.mode             = LONG_ADDR;
    //hdr->dest_addr.long_addr        = 0x1234123412341234ULL;
    hdr->mac_frm_ctrl.pan_id_compr  = false;
    hdr->src_pan_id                 = 0xffff;
    hdr->src_addr.mode              = SHORT_ADDR;
    hdr->src_addr.short_addr        = 0x5678;
    //hdr->src_addr.mode              = LONG_ADDR;
    //hdr->src_addr.long_addr         = 0x0FEDCBA987654321ULL;

    hdr->mac_frm_ctrl.frame_pending = false;
    hdr->mac_frm_ctrl.frame_type    = MAC_DATA;
}

/**************************************************************************
Function Name: test_mac_gen_nwk_data

Description:
Generate data to be used in the NWK layer.
**************************************************************************/
static void test_gen_nwk_data(nwk_hdr_t *hdr)
{
    nwk_nib_t *nib = nwk_nib_get();

    memset(hdr, 0 , sizeof(nwk_hdr_t));
    hdr->nwk_frm_ctrl.frame_type           = NWK_DATA_FRM;
    hdr->nwk_frm_ctrl.disc_route           = false;
    hdr->nwk_frm_ctrl.protocol_ver         = 0x1;
    hdr->nwk_frm_ctrl.security             = false;
    //hdr->nwk_frm_ctrl.dest_ieee_addr_flag  = true;
    //hdr->nwk_frm_ctrl.src_ieee_addr_flag   = true;

    hdr->dest_addr  = 0xFFFF;
    hdr->radius     = 0x3;
    hdr->src_addr   = 0x5678;
    hdr->seq_num    = nib->seq_num;

    //hdr->dest_ieee_addr                    = 0x123456789ABCDEF0ULL;
    //hdr->src_ieee_addr                     = 0x1122334455667788ULL;
}

/**************************************************************************
Function Name: test_mac_gen_aps_data

Description:
Generate the data for the aps layer.
**************************************************************************/
static void test_gen_aps_data(aps_hdr_t *hdr)
{
    memset(hdr, 0 , sizeof(aps_hdr_t));

    hdr->aps_frm_ctrl.ack_req          = false;
    hdr->aps_frm_ctrl.delivery_mode    = APS_UNICAST;
    hdr->aps_frm_ctrl.frm_type         = APS_DATA_FRM;
    hdr->aps_frm_ctrl.ind_addr_mode    = 0x1;
    hdr->aps_frm_ctrl.security         = false;

    hdr->clust_id  = 0x2222;
    hdr->dest_ep   = 0x11;
    //hdr->grp_addr  = 0x3333;
    hdr->prof_id   = 0x6666;
    hdr->src_ep    = 0x88;
    hdr->aps_ctr   = 0x03;
}

/**************************************************************************
Function Name: process thread - test_data_in

Description:
Generate and send data to the DUT.
**************************************************************************/
PT_THREAD(test_data_in(struct pt *pt))
{
    PT_BEGIN(pt);

    buffer_t *txbuf = buf_get(TX);
    static buffer_t *rx_buf;
    aps_hdr_t aps_hdr;
    nwk_hdr_t nwk_hdr;
    mac_hdr_t mac_hdr;
    mac_pib_t *pib = mac_pib_get();


    DBG_PRINT("TEST_DATA_IN: Start.\n");
    data_received = false;
    pib->dsn = 0x69;

    // gen the header data
    test_gen_aps_data(&aps_hdr);
    test_gen_nwk_data(&nwk_hdr);
    test_gen_mac_data(&mac_hdr);

    // compile the data into a valid header. order is important.
    aps_gen_header(txbuf, &aps_hdr);
    nwk_gen_header(txbuf, &nwk_hdr);
    mac_gen_header(txbuf, &mac_hdr);

    // load the autochecking fields
    test_check_load_frm(&aps_hdr, &nwk_hdr, &mac_hdr, NULL, 0);

    test_driver_write_rx_buf(txbuf->dptr, txbuf->len);
    test_driver_rx_isr();
    buf_free(txbuf);

    PT_WAIT_UNTIL(pt, data_received == true);
    PT_WAIT_UNTIL(pt, mac_queue_is_empty(TX));

    // wait for the ack from the frame to arrive.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    test_check_ack(rx_buf);
    buf_free(rx_buf);

    finished = true;
    DBG_PRINT("TEST_DATA_IN: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));
    PT_END(pt);
}


