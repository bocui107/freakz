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

    Title: test_nwk_join1.c

    Description:
    This test will test the parent (server) side of the nwk join process.
    It will send an association request, wait for a certain period of time,
    then send a data request for the association response. If the test succeeds,
    the association response will be returned with a success status and a
    short address.
*******************************************************************/
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"

static struct etimer tmr;
extern bool finished;

/**************************************************************************
Function Name: test_gen_assoc_req

Description:

**************************************************************************/
void test_gen_assoc_req(buffer_t *buf, nwk_capab_info_t *info)
{
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    mac_pib_t *pib = mac_pib_get();

    cmd.cmd_id = MAC_ASSOC_REQ;
    cmd.cmd_params.assoc_req.cap_info = nwk_gen_capab_info(info);
    mac_gen_cmd(buf, &cmd);

    hdr.mac_frm_ctrl.ack_req        = true;
    hdr.mac_frm_ctrl.frame_pending  = false;
    hdr.mac_frm_ctrl.frame_type     = MAC_COMMAND;
    hdr.mac_frm_ctrl.frame_ver      = 0x0;
    hdr.mac_frm_ctrl.pan_id_compr   = false;
    hdr.mac_frm_ctrl.sec_enb        = false;
    hdr.mac_fcf                     = mac_gen_frm_ctrl(&hdr);
    hdr.dest_addr.mode              = SHORT_ADDR;
    hdr.dest_addr.short_addr        = pib->short_addr;
    hdr.dest_pan_id                 = pib->pan_id;
    hdr.src_addr.mode               = LONG_ADDR;
    hdr.src_addr.long_addr          = 0x1111222233334444ULL;
    hdr.src_pan_id                  = pib->pan_id;
    hdr.dsn                         = rand();
    hdr.msdu_handle                 = hdr.dsn;
    mac_gen_header(buf, &hdr);
}

/**************************************************************************
Function Name: test_gen_data_req

Description:
Generate a data request for the indirect transfer.
**************************************************************************/
static void test_gen_data_req(buffer_t *buf)
{
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    mac_pib_t *pib = mac_pib_get();

    cmd.cmd_id = MAC_DATA_REQ;

    memset(&hdr, 0, sizeof(mac_hdr_t));
    hdr.mac_frm_ctrl.frame_type     = MAC_COMMAND;
    hdr.mac_frm_ctrl.frame_pending  = false;
    hdr.mac_frm_ctrl.ack_req        = true;
    hdr.mac_frm_ctrl.pan_id_compr   = false;
    hdr.mac_fcf                     = mac_gen_frm_ctrl(&hdr);
    hdr.dest_pan_id                 = pib->pan_id;
    hdr.dest_addr.mode              = SHORT_ADDR;
    hdr.dest_addr.short_addr        = pib->short_addr;
    hdr.src_pan_id                  = pib->pan_id;
    hdr.src_addr.mode               = LONG_ADDR;
    hdr.src_addr.long_addr          = 0x1111222233334444ULL;

    mac_gen_cmd(buf, &cmd);
    mac_gen_header(buf, &hdr);
}

/**************************************************************************
Function Name: test_gen_check_data

Description:
Generate the data that will be checked against.
**************************************************************************/
static void test_gen_check_data()
{
    mac_hdr_t mac_hdr;
    mac_cmd_t mac_cmd;
    mac_pib_t *pib = mac_pib_get();

    memset(&mac_hdr, 0, sizeof(mac_hdr_t));
    mac_hdr.mac_frm_ctrl.frame_type     = MAC_COMMAND;
    mac_hdr.mac_frm_ctrl.frame_pending  = false;
    mac_hdr.mac_frm_ctrl.ack_req        = true;
    mac_hdr.mac_frm_ctrl.pan_id_compr   = true;
    mac_hdr.mac_fcf                     = mac_gen_frm_ctrl(&mac_hdr);
    mac_hdr.dest_pan_id                 = pib->pan_id;
    mac_hdr.dest_addr.mode              = LONG_ADDR;
    mac_hdr.dest_addr.long_addr         = 0x1111222233334444ULL;
    mac_hdr.src_pan_id                  = pib->pan_id;
    mac_hdr.src_addr.mode               = SHORT_ADDR;
    mac_hdr.src_addr.short_addr         = pib->short_addr;
    test_check_load_mac_hdr(&mac_hdr);

    mac_cmd.cmd_id                              = MAC_ASSOC_RESP;
    mac_cmd.cmd_params.assoc_resp.assoc_status  = MAC_SUCCESS;
    mac_cmd.cmd_params.assoc_resp.short_addr    = 0x1;
    test_check_load_mac_cmd(&mac_cmd);
}

/**************************************************************************
Function Name: test_nwk_join1 thread

Description:

**************************************************************************/
PT_THREAD(test_nwk_join1(struct pt *pt))
{
    PT_BEGIN(pt);
    buffer_t *tx_buf = NULL;
    static buffer_t *rx_buf = NULL;
    nwk_capab_info_t info;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();
    mac_hdr_t mac_hdr;
    mac_cmd_t mac_cmd;

    // set up the device
    nib->max_children = 8;
    nib->max_routers = 4;
    nib->max_depth = 3;
    nib->short_addr = pib->short_addr = 0;
    pib->coord_addr.mode = SHORT_ADDR;
    pib->coord_addr.short_addr = 0;
    nib->cskip = nwk_rte_tree_calc_cskip(0);
    nib->rtr_cap = nib->max_routers;
    nib->ed_cap = nib->max_children - nib->max_routers;

    DBG_PRINT("TEST_NWK_JOIN1: Started.\n");
    BUF_ALLOC(tx_buf, TX);

    // generate the association request
    info.alloc_addr = true;
    info.dev_type   = NWK_ROUTER;
    info.pwr_src    = true;
    info.rx_on_idle = true;
    info.security   = false;
    test_gen_assoc_req(tx_buf, &info);

    // send out the association request
    DBG_PRINT("TEST_NWK_JOIN1: Sending out the association request.\n");
    test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
    test_driver_rx_isr();
    buf_free(tx_buf);

    // first frame should be an ack that is returned.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    assert(test_check_ack(rx_buf));
    buf_free(rx_buf);

    // wait for the association response
    etimer_set(&tmr, aMacResponseWaitTime);
    PT_WAIT_UNTIL(pt, etimer_expired(&tmr));

    // send out the data request
    DBG_PRINT("TEST_NWK_JOIN1: Sending out the data request.\n");
    BUF_ALLOC(tx_buf, TX);
    test_gen_data_req(tx_buf);
    test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
    test_driver_rx_isr();
    buf_free(tx_buf);

    // gen the check data
    test_gen_check_data();

    // first frame should be an ack that is returned.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    assert(test_check_ack(rx_buf));
    buf_free(rx_buf);

    // second frame should be the association response.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    mac_parse_hdr(rx_buf, &mac_hdr);
    mac_parse_cmd(rx_buf, &mac_cmd);
    test_check_mac(&mac_hdr);
    test_check_mac_cmd(&mac_cmd);
    buf_free(rx_buf);
    DBG_PRINT("TEST_NWK_JOIN1: Association response received.\n");

    if (mac_hdr.mac_frm_ctrl.ack_req)
    {
        // gen the ack and send it into the device
        BUF_ALLOC(tx_buf, TX);
        test_gen_ack(tx_buf, &mac_hdr, false);
        test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
        test_driver_rx_isr();
        buf_free(tx_buf);

        PT_WAIT_UNTIL(pt, mac_queue_is_empty(TX));
    }

    // check that all buffers are freed at the end.
    DBG_PRINT("\nTEST_NWK_JOIN1: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));

    finished = true;
    DBG_PRINT("TEST_NWK_JOIN1: Finished.\n");

    PT_END(pt);
}


