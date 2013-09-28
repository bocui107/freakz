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

    Title: test_data_out.c

    Description:
    Tests the outbound data path of the FreakZ stack.
*******************************************************************/
#include <string.h>
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"

#define NEIGHBOR_COUNT 1

extern bool finished;
extern bool i_got_an_ack;

// for parsing incoming frame
static mac_hdr_t mac_hdr_in;
static nwk_hdr_t nwk_hdr_in;
static aps_hdr_t aps_hdr_in;

// for generating outgoing frame
static mac_hdr_t mac_hdr_out;
static nwk_hdr_t nwk_hdr_out;
static aps_hdr_t aps_hdr_out;

// receive and transmit frame buffers
static buffer_t rx_buf;
static buffer_t tx_buf;

/**************************************************************************
Function Name: test_gen_neighbors

Description:
Generate the neighbors in the neighbor table for this device.
**************************************************************************/
static void test_gen_neighbors()
{
    U8 i;
    nbor_tbl_entry_t *entry;
    U16 neighbors[NEIGHBOR_COUNT][2] = {
        {0x1234,   NWK_CHILD},
    };

    for (i=0; i<NEIGHBOR_COUNT; i++)
    {
        if ((nwk_neighbor_tbl_find_short_addr(neighbors[i][0])) == NULL)
        {
            if ((entry = nwk_neighbor_tbl_alloc()) == NULL) assert(0);
            entry->nwk_addr = neighbors[i][0];
            entry->relationship = neighbors[i][1];
            entry->rx_on_when_idle = true;
        }
    }
}

/**************************************************************************
Function Name: test_mac_gen_mac_data

Description:
Generate data to be used in the MAC layer.
**************************************************************************/
static void test_gen_mac_data(mac_hdr_t *hdr)
{
    memset(hdr, 0, sizeof(mac_hdr_t));

    hdr->mac_frm_ctrl.ack_req       = false;
    hdr->dest_pan_id                = mac_hdr_in.src_pan_id;
    hdr->dest_addr.mode             = mac_hdr_in.src_addr.mode;
    hdr->dest_addr.short_addr       = mac_hdr_in.src_addr.short_addr;
    hdr->mac_frm_ctrl.pan_id_compr  = false;
    hdr->src_pan_id                 = mac_hdr_in.dest_pan_id;
    hdr->src_addr.mode              = mac_hdr_in.dest_addr.mode;
    hdr->src_addr.short_addr        = mac_hdr_in.dest_addr.short_addr;
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
    hdr->nwk_frm_ctrl.protocol_ver         = nwk_hdr_in.nwk_frm_ctrl.protocol_ver;
    hdr->nwk_frm_ctrl.security             = false;
    hdr->dest_addr                         = nwk_hdr_in.src_addr;
    hdr->radius                            = nwk_hdr_in.radius;
    hdr->src_addr                          = nwk_hdr_in.dest_addr;
    hdr->seq_num                           = nib->seq_num;
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
    hdr->aps_frm_ctrl.frm_type         = APS_ACK_FRM;
    hdr->aps_frm_ctrl.security         = false;

    hdr->clust_id  = aps_hdr_in.clust_id;
    hdr->dest_ep   = aps_hdr_in.dest_ep;
    hdr->prof_id   = aps_hdr_in.prof_id;
    hdr->src_ep    = aps_hdr_in.src_ep;
    hdr->aps_ctr   = aps_hdr_in.aps_ctr;
}

/**************************************************************************
Function Name: test_gen_check_data

Description:
Generate the data that will be checked against.
**************************************************************************/
static void test_gen_check_data(apsde_data_req_t *req)
{
    aps_hdr_t aps_hdr;
    nwk_hdr_t nwk_hdr;
    mac_hdr_t mac_hdr;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    memset(&mac_hdr, 0, sizeof(mac_hdr_t));
    mac_hdr.mac_frm_ctrl.frame_type     = MAC_DATA;
    mac_hdr.mac_frm_ctrl.frame_pending  = false;
    mac_hdr.mac_frm_ctrl.ack_req        = false;
    mac_hdr.mac_frm_ctrl.pan_id_compr   = false;
    mac_hdr.mac_fcf                     = mac_gen_frm_ctrl(&mac_hdr);
    mac_hdr.dest_pan_id                 = pib->pan_id;
    mac_hdr.dest_addr.mode              = req->dest_addr.mode;
    mac_hdr.dest_addr.short_addr        = req->dest_addr.short_addr;
    mac_hdr.src_pan_id                  = pib->pan_id;
    mac_hdr.src_addr.mode               = SHORT_ADDR;
    mac_hdr.src_addr.short_addr         = nib->short_addr;

    memset(&nwk_hdr, 0 , sizeof(nwk_hdr_t));
    nwk_hdr.nwk_frm_ctrl.frame_type     = NWK_DATA_FRM;
    nwk_hdr.nwk_frm_ctrl.disc_route     = false;
    nwk_hdr.nwk_frm_ctrl.protocol_ver   = nwkcProtocolVersion;
    nwk_hdr.nwk_frm_ctrl.security       = false;
    nwk_hdr.nwk_fcf                     = nwk_gen_frm_ctrl(&nwk_hdr);
    nwk_hdr.dest_addr                   = req->dest_addr.short_addr;
    nwk_hdr.radius                      = 0x5;
    nwk_hdr.src_addr                    = nib->short_addr;

    memset(&aps_hdr, 0 , sizeof(aps_hdr_t));
    aps_hdr.aps_frm_ctrl.ack_req        = true;
    aps_hdr.aps_frm_ctrl.delivery_mode  = APS_UNICAST;
    aps_hdr.aps_frm_ctrl.frm_type       = APS_DATA_FRM;
    aps_hdr.aps_frm_ctrl.ind_addr_mode  = 0x0;
    aps_hdr.aps_frm_ctrl.security       = false;
    aps_hdr.aps_fcf                     = aps_gen_frm_ctrl(&aps_hdr);
    aps_hdr.clust_id                    = req->clust_id;
    aps_hdr.dest_ep                     = req->dest_ep;
    aps_hdr.prof_id                     = req->prof_id;
    aps_hdr.src_ep                      = req->src_ep;
    aps_hdr.aps_ctr                     = 0x03;

    test_check_load_frm(&aps_hdr, &nwk_hdr, &mac_hdr, NULL, 0);
}

/**************************************************************************
Function Name: test_gen_apsde_data_req

Description:
Generate the aps data request for the outbound data frame.
**************************************************************************/
static void test_gen_apsde_data_req(buffer_t *buf, apsde_data_req_t *req)
{
    req->dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req->dest_addr.mode          = SHORT_ADDR;
    req->dest_addr.short_addr    = 0x1234;
    req->dest_ep                 = 0x33;
    req->prof_id                 = 0x1111;
    req->clust_id                = 0x2222;
    req->src_ep                  = 0x33;
    req->asdu_len                = buf->len;
    req->buf                     = buf;
    req->tx_opt                  = APS_TX_REQ_ACK_TX_MASK;
    req->radius                  = 0x5;
}

/**************************************************************************
Function Name: process thread - test_data_out_ack

Description:
This is the main test function that generates the apsde data request, checks
the outbound packet, sends in an ACK packet, and checks the ACK packet for correctness.
**************************************************************************/
PT_THREAD(test_data_out_ack(struct pt *pt))
{
    PT_BEGIN(pt);
    U8 len;
    apsde_data_req_t *req;
    buffer_t *buf = buf_get(TX);
    U8 data[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    nib->short_addr = pib->short_addr = rand();
    pib->dsn = 0x69;
    nwk_neighbor_tbl_clear_tbl();
    test_gen_neighbors();

    memset(&rx_buf, 0, sizeof(buffer_t));
    memset(&tx_buf, 0, sizeof(buffer_t));
    rx_buf.dptr = &rx_buf.buf[0];
    tx_buf.dptr = &tx_buf.buf[aMaxPHYPacketSize-1];

    // Grab an aps_request, fill it out, and then put it in the request queue for processing
    if ((req = aps_queue_alloc()) == NULL) assert(0);

    len = sizeof(data);
    buf->dptr -= len;
    memcpy(buf->dptr, data, len);
    buf->len += len;
    test_gen_apsde_data_req(buf, req);

    // load the auto check buffers with the parameters we are expecting
    test_gen_check_data(req);

    DBG_PRINT("TEST_DATA_OUT_ACK: Start.\n");
    apsde_data_req(req);

    // Wait until data arrives at the output of the test driver
    PT_WAIT_UNTIL(pt, rx_buf.len = test_driver_read_tx_buf(rx_buf.buf));

    // Now that we have data, parse it and generate an APS ACK frame
    rx_buf.dptr = rx_buf.buf;
    test_parse_rx_data(&rx_buf, &aps_hdr_in, &nwk_hdr_in, &mac_hdr_in);
    test_check_frm_hdrs(&aps_hdr_in, &nwk_hdr_in, &mac_hdr_in);

    // check to make sure the transmission has ack request set
    if (mac_hdr_in.mac_frm_ctrl.ack_req)
    {
        // gen the ack and send it into the device
        BUF_ALLOC(buf, TX);
        test_gen_ack(buf, &mac_hdr_in, false);
        test_driver_write_rx_buf(buf->dptr, buf->len);
        test_driver_rx_isr();
        buf_free(buf);

        //PT_WAIT_UNTIL(pt, queue_is_empty(TX));
    }
    else
    {
        assert(0);
    }

    // generate the data for each header
    test_gen_mac_data(&mac_hdr_out);
    test_gen_nwk_data(&nwk_hdr_out);
    test_gen_aps_data(&aps_hdr_out);

    // now generate the actual header and put it in buffer. be careful, order matters here.
    aps_gen_header(&tx_buf, &aps_hdr_out);
    nwk_gen_header(&tx_buf, &nwk_hdr_out);
    mac_gen_header(&tx_buf, &mac_hdr_out);

    // load the auto-check for the ACK
    test_check_load_frm(&aps_hdr_out, &nwk_hdr_out, &mac_hdr_out, NULL, 0);

    test_driver_write_rx_buf(tx_buf.dptr, tx_buf.len);
    test_driver_rx_isr();

    PT_WAIT_UNTIL(pt, i_got_an_ack == true);
    finished = true;

    // check that all buffers are freed at the end.
    DBG_PRINT("TEST_DATA_IN: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));

    PT_END(pt);
}


