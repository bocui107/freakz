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

    Title: test_brc_rtr.c

    Description:
    Test the network broadcast function. This test only sends the broadcast
    to routers and coordinators. It doesn't send a return broadcast so the device
    should send the broadcast three times and then give up.
*******************************************************************/
#include "contiki.h"
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"

extern bool finished;
static nwk_pcb_t *pcb;

/**************************************************************************
Function Name: test_brc_rtr_gen_neighbors

Description:
Generate the neighbors in the neighbor table for this device.
**************************************************************************/
void test_brc_rtr_gen_neighbors()
{
    U8 i;
    nbor_tbl_entry_t *entry;
    U16 neighbors[8][2] = {
        {3,   NWK_CHILD},
        {4,   NWK_CHILD},
        {5,   NWK_CHILD},
        {6,   NWK_CHILD},
        {1,   NWK_PARENT},
        {7,   NWK_SIBLING},
        {12,  NWK_SIBLING},
        {17,  NWK_SIBLING}
    };

    for (i=0; i<8; i++)
    {
        if ((nwk_neighbor_tbl_find_short_addr(neighbors[i][0])) == NULL)
        {
            if ((entry = nwk_neighbor_tbl_alloc()) == NULL) assert(0);
            entry->nwk_addr = neighbors[i][0];
            entry->relationship = neighbors[i][1];
        }
    }
}

/**************************************************************************
Function Name: test_brc_rtr_gen_frame_in

Description:
Generate the test frame that we're gonna send into the device.
**************************************************************************/
static void test_brc_rtr_gen_frame_in(buffer_t *buf, aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr)
{
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    memset(mac_hdr, 0, sizeof(mac_hdr_t));
    mac_hdr->mac_frm_ctrl.ack_req           = false;
    mac_hdr->dest_pan_id                    = pib->pan_id;
    mac_hdr->dest_addr.mode                 = SHORT_ADDR;
    mac_hdr->dest_addr.short_addr           = MAC_BROADCAST_ADDR;
    mac_hdr->mac_frm_ctrl.pan_id_compr      = false;
    mac_hdr->src_pan_id                     = pib->pan_id;
    mac_hdr->src_addr.mode                  = SHORT_ADDR;
    mac_hdr->src_addr.short_addr            = 0x3;
    mac_hdr->mac_frm_ctrl.frame_pending     = false;
    mac_hdr->mac_frm_ctrl.frame_type        = MAC_DATA;

    memset(nwk_hdr, 0 , sizeof(nwk_hdr_t));
    nwk_hdr->nwk_frm_ctrl.frame_type        = NWK_DATA_FRM;
    nwk_hdr->nwk_frm_ctrl.disc_route        = false;
    nwk_hdr->nwk_frm_ctrl.protocol_ver      = nwkcProtocolVersion;
    nwk_hdr->nwk_frm_ctrl.security          = false;
    nwk_hdr->nwk_fcf                        = nwk_gen_frm_ctrl(nwk_hdr);
    nwk_hdr->dest_addr                      = NWK_BROADCAST_ROUTERS_COORD;
    nwk_hdr->radius                         = 0x3;
    nwk_hdr->src_addr                       = 0x3;
    nwk_hdr->seq_num                        = nib->seq_num;

    memset(aps_hdr, 0 , sizeof(aps_hdr_t));
    aps_hdr->aps_frm_ctrl.ack_req           = false;
    aps_hdr->aps_frm_ctrl.delivery_mode     = APS_UNICAST;
    aps_hdr->aps_frm_ctrl.frm_type          = APS_DATA_FRM;
    aps_hdr->aps_frm_ctrl.ind_addr_mode     = 0x1;
    aps_hdr->aps_frm_ctrl.security          = false;
    aps_hdr->clust_id                       = 0x2222;
    aps_hdr->dest_ep                        = 0x11;
    aps_hdr->prof_id                        = 0x6666;
    aps_hdr->src_ep                         = 0x88;
    aps_hdr->aps_ctr                        = 0x03;

    // compile the data into a valid header. order is important.
    aps_gen_header(buf, aps_hdr);
    nwk_gen_header(buf, nwk_hdr);
    mac_gen_header(buf, mac_hdr);
}

/**************************************************************************
Function Name: test_brc_rtr_check_data_frm

Description:
Check the final forwarded frame.
**************************************************************************/
static void test_brc_rtr_load_check_data_in()
{
    mac_hdr_t mac_hdr_exp;
    nwk_hdr_t nwk_hdr_exp;
    aps_hdr_t aps_hdr_exp;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    // generate the expected frame headers and load them into the check engine
    memset(&mac_hdr_exp, 0, sizeof(mac_hdr_t));
    mac_hdr_exp.mac_frm_ctrl.frame_type         = MAC_DATA;
    mac_hdr_exp.mac_frm_ctrl.frame_pending      = false;
    mac_hdr_exp.mac_frm_ctrl.ack_req            = false;
    mac_hdr_exp.mac_frm_ctrl.pan_id_compr       = false;
    mac_hdr_exp.mac_fcf                         = mac_gen_frm_ctrl(&mac_hdr_exp);
    mac_hdr_exp.dest_pan_id                     = pib->pan_id;
    mac_hdr_exp.dest_addr.mode                  = SHORT_ADDR;
    mac_hdr_exp.dest_addr.short_addr            = MAC_BROADCAST_ADDR;
    mac_hdr_exp.src_pan_id                      = pib->pan_id;
    mac_hdr_exp.src_addr.mode                   = SHORT_ADDR;
    mac_hdr_exp.src_addr.short_addr             = 0x3;
    test_check_load_mac_hdr(&mac_hdr_exp);

    memset(&nwk_hdr_exp, 0, sizeof(nwk_hdr_t));
    nwk_hdr_exp.nwk_frm_ctrl.frame_type         = NWK_DATA_FRM;
    nwk_hdr_exp.nwk_frm_ctrl.disc_route         = false;
    nwk_hdr_exp.nwk_frm_ctrl.protocol_ver       = nwkcProtocolVersion;
    nwk_hdr_exp.nwk_frm_ctrl.security           = false;
    nwk_hdr_exp.nwk_fcf                         = nwk_gen_frm_ctrl(&nwk_hdr_exp);
    nwk_hdr_exp.dest_addr                       = NWK_BROADCAST_ROUTERS_COORD;
    nwk_hdr_exp.radius                          = 0x2;
    nwk_hdr_exp.src_addr                        = 0x3;
    nwk_hdr_exp.seq_num                         = nib->seq_num;
    test_check_load_nwk_hdr(&nwk_hdr_exp);

    memset(&aps_hdr_exp, 0, sizeof(aps_hdr_t));
    aps_hdr_exp.aps_frm_ctrl.ack_req            = false;
    aps_hdr_exp.aps_frm_ctrl.delivery_mode      = APS_UNICAST;
    aps_hdr_exp.aps_frm_ctrl.frm_type           = APS_DATA_FRM;
    aps_hdr_exp.aps_frm_ctrl.ind_addr_mode      = 0x1;
    aps_hdr_exp.aps_frm_ctrl.security           = false;
    aps_hdr_exp.aps_fcf                         = aps_gen_frm_ctrl(&aps_hdr_exp);
    aps_hdr_exp.clust_id                        = 0x2222;
    aps_hdr_exp.dest_ep                         = 0x11;
    aps_hdr_exp.prof_id                         = 0x6666;
    aps_hdr_exp.src_ep                          = 0x88;
    aps_hdr_exp.aps_ctr                         = 0x03;
    test_check_load_aps_hdr(&aps_hdr_exp);
}

/**************************************************************************
Function Name: test_brc_rtr_check_data_frm

Description:
Check the final forwarded frame.
**************************************************************************/
static void test_brc_rtr_check_data_frm(buffer_t *buf, aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr)
{
    mac_hdr_t mac_hdr_exp;
    nwk_hdr_t nwk_hdr_exp;
    aps_hdr_t aps_hdr_exp;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    // generate the expected frame headers and load them into the check engine
    memset(&mac_hdr_exp, 0, sizeof(mac_hdr_t));
    mac_hdr_exp.mac_frm_ctrl.frame_type     = MAC_DATA;
    mac_hdr_exp.mac_frm_ctrl.frame_pending  = false;
    mac_hdr_exp.mac_frm_ctrl.ack_req        = false;
    mac_hdr_exp.mac_frm_ctrl.pan_id_compr   = false;
    mac_hdr_exp.mac_fcf                     = mac_gen_frm_ctrl(&mac_hdr_exp);
    mac_hdr_exp.dest_pan_id                 = pib->pan_id;
    mac_hdr_exp.dest_addr.mode              = SHORT_ADDR;
    mac_hdr_exp.dest_addr.short_addr        = MAC_BROADCAST_ADDR;
    mac_hdr_exp.src_pan_id                  = pib->pan_id;
    mac_hdr_exp.src_addr.mode               = SHORT_ADDR;
    mac_hdr_exp.src_addr.short_addr         = nib->short_addr;
    test_check_load_mac_hdr(&mac_hdr_exp);

    memset(&nwk_hdr_exp, 0, sizeof(nwk_hdr_t));
    nwk_hdr_exp.nwk_frm_ctrl.frame_type     = NWK_DATA_FRM;
    nwk_hdr_exp.nwk_frm_ctrl.disc_route     = false;
    nwk_hdr_exp.nwk_frm_ctrl.protocol_ver   = nwkcProtocolVersion;
    nwk_hdr_exp.nwk_frm_ctrl.security       = false;
    nwk_hdr_exp.nwk_fcf                     = nwk_gen_frm_ctrl(&nwk_hdr_exp);
    nwk_hdr_exp.dest_addr                   = NWK_BROADCAST_ROUTERS_COORD;
    nwk_hdr_exp.radius                      = 0x2;
    nwk_hdr_exp.src_addr                    = nib->short_addr;
    test_check_load_nwk_hdr(&nwk_hdr_exp);

    memset(&aps_hdr_exp, 0, sizeof(aps_hdr_t));
    aps_hdr_exp.aps_frm_ctrl.ack_req           = false;
    aps_hdr_exp.aps_frm_ctrl.delivery_mode     = APS_UNICAST;
    aps_hdr_exp.aps_frm_ctrl.frm_type          = APS_DATA_FRM;
    aps_hdr_exp.aps_frm_ctrl.ind_addr_mode     = 0x1;
    aps_hdr_exp.aps_frm_ctrl.security          = false;
    aps_hdr_exp.aps_fcf                        = aps_gen_frm_ctrl(&aps_hdr_exp);
    aps_hdr_exp.clust_id                       = 0x2222;
    aps_hdr_exp.dest_ep                        = 0x11;
    aps_hdr_exp.prof_id                        = 0x6666;
    aps_hdr_exp.src_ep                         = 0x88;
    aps_hdr_exp.aps_ctr                        = 0x03;
    test_check_load_aps_hdr(&aps_hdr_exp);

    // Parse the received data and check it for correctness
    mac_parse_hdr(buf, mac_hdr);
    nwk_parse_hdr(buf, nwk_hdr);
    aps_parse_hdr(buf, aps_hdr);

    debug_dump_mac_hdr(mac_hdr);
    debug_dump_nwk_hdr(nwk_hdr);
    debug_dump_aps_hdr(aps_hdr);

    test_check_mac(mac_hdr);
    test_check_nwk(nwk_hdr);
    test_check_aps(aps_hdr);
}

/**************************************************************************
Function Name: process thread - test_data_in_rte1

Description:
This is the main test function which will generate an inbound packet with a
nwk dest address that does not match the mac address.
**************************************************************************/
PT_THREAD(test_brc_rtr(struct pt *pt))
{
    PT_BEGIN(pt);
    buffer_t *tx_buf = buf_get(TX);
    static buffer_t *rx_buf;
    aps_hdr_t aps_hdr;
    nwk_hdr_t nwk_hdr;
    mac_hdr_t mac_hdr;
    nwk_nib_t *nib = nwk_nib_get();

    DBG_PRINT("TEST_BRC_RTR: Start.\n\n");

    // set up the NIB parameters needed for the tree routing algorithm
    nib->max_broadcast_retries = 3;
    nib->passive_ack_timeout = 0xBB8;
    nib->short_addr = 2;

    // set up the neighbors in the neighbor table
    test_brc_rtr_gen_neighbors();

    // generate the test frame and send it into the device
    test_brc_rtr_gen_frame_in(tx_buf, &aps_hdr, &nwk_hdr, &mac_hdr);
    test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
    test_driver_rx_isr();
    buf_free(tx_buf);

    // load the check data in to check the incoming frame is correct at the APS layer
    test_brc_rtr_load_check_data_in();

    // Now poll until data is received from device
    // We're expecting the next frame to be forwarded to the next hop towards the destination.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    buf_free(rx_buf);

    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    buf_free(rx_buf);

    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));

    // load the auto check with the expected data from the data frame
    test_brc_rtr_check_data_frm(rx_buf, &aps_hdr, &nwk_hdr, &mac_hdr);
    buf_free(rx_buf);

    DBG_PRINT("TEST_BRC_RTR: Data Received.\n");

    // wait until the brc timer expires before starting next test
    pcb = nwk_pcb_get();
    PT_WAIT_UNTIL(pt, pcb->brc_accept_new);

    // check that all buffers are freed at the end.
    DBG_PRINT("TEST_DATA_IN: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));

    finished = true;
    PT_END(pt);
}



