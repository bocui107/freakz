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

    Title: test_nwk_join2.c

    Description:
    This file tests the child (client) side of the nwk join procedure.
    It starts by initiaitng a network scan, choosing a parent, and then
    doing an association request. This file will play the part of the
    parent and perform the appropriate responses to the child actions.
*******************************************************************/
#include "zigbee.h"
#include "test_driver.h"
#include "test.h"

extern bool data_received;
extern bool finished;

/**************************************************************************
Function Name: test_init_device

Description:
Init the parameters for the device.
**************************************************************************/
static void test_init_device()
{
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    nib->stack_profile  = 0x1;
    nib->prot_ver       = 0x2;
    nib->ext_pan_ID     = 0x1122334455667788ULL;
    nib->max_children   = 8;
    nib->max_routers    = 4;
    nib->max_depth      = 3;
    nib->depth          = 0;
    nib->rtr_cap        = nib->max_routers;
    nib->ed_cap         = nib->max_children - nib->max_routers;
    nib->dev_type       = NWK_ROUTER;

    pib->coord          = true;
    pib->assoc_permit   = true;
}

/**************************************************************************
Function Name: test_send_beacon

Description:
Send a beacon in response to a beacon request.
**************************************************************************/
static void test_send_beacon(mac_hdr_t *hdr)
{
    mac_pib_t *pib = mac_pib_get();
    buffer_t *buf = NULL;

    memset(hdr, 0, sizeof(mac_hdr_t));
    hdr->mac_frm_ctrl.frame_type   = MAC_BEACON;
    hdr->mac_frm_ctrl.ack_req       = false;
    hdr->mac_frm_ctrl.pan_id_compr  = false;
    hdr->src_pan_id                 = pib->pan_id;
    hdr->src_addr.mode              = SHORT_ADDR;
    hdr->src_addr.short_addr        = 0x5678;

    DBG_PRINT("TEST_NWK_JOIN2: Sending beacon back in response to beacon request.\n");
    BUF_ALLOC(buf, TX);
    mac_gen_beacon(buf);
    mac_gen_header(buf, hdr);

    test_driver_write_rx_buf(buf->dptr, buf->len);
    test_driver_rx_isr();
    buf_free(buf);
}

/**************************************************************************
Function Name: test_send_join_req

Description:

**************************************************************************/
void test_send_join_req(pan_descr_t *desc)
{
    nwk_join_req_t args;

    args.ext_pan_id     = desc->ext_pan_id;
    args.join_as_rtr    = true;
    args.rejoin_nwk     = false;
    nwk_join_req(&args);
}

/**************************************************************************
Function Name: test_send_assoc_resp

Description:

**************************************************************************/
static void test_send_assoc_resp()
{
    mac_cmd_t cmd;
    mac_hdr_t hdr;
    buffer_t *buf = NULL;
    mac_pib_t *pib = mac_pib_get();

    cmd.cmd_id = MAC_ASSOC_RESP;
    cmd.cmd_params.assoc_resp.assoc_status = MAC_SUCCESS;
    cmd.cmd_params.assoc_resp.short_addr = 0x1111;

    memset(&hdr, 0, sizeof(mac_hdr_t));
    hdr.mac_frm_ctrl.frame_type     = MAC_COMMAND;
    hdr.mac_frm_ctrl.frame_pending  = false;
    hdr.mac_frm_ctrl.ack_req        = true;
    hdr.mac_frm_ctrl.pan_id_compr   = false;
    hdr.mac_fcf                     = mac_gen_frm_ctrl(&hdr);
    hdr.dest_pan_id                 = pib->pan_id;
    hdr.dest_addr.mode              = LONG_ADDR;
    hdr.dest_addr.long_addr         = pib->ext_addr;
    hdr.src_pan_id                  = pib->pan_id;
    hdr.src_addr.mode               = SHORT_ADDR;
    hdr.src_addr.short_addr         = 0x5678;

    DBG_PRINT("TEST_NWK_JOIN2: Sending association response.\n");
    BUF_ALLOC(buf, TX);
    mac_gen_cmd(buf, &cmd);
    mac_gen_header(buf, &hdr);

    test_driver_write_rx_buf(buf->dptr, buf->len);
    test_driver_rx_isr();
    buf_free(buf);
}

/**************************************************************************
Function Name: test_load_check_data

Description:
Load the check data.
**************************************************************************/
void test_gen_desc_check_data(U8 channel, mac_hdr_t *hdr)
{
    pan_descr_t desc;
    nwk_nib_t *nib = nwk_nib_get();
    mac_pib_t *pib = mac_pib_get();

    desc.channel                = 26;
    desc.coord_addr.mode        = SHORT_ADDR;
    desc.coord_addr.short_addr  = 0x5678;
    desc.coord_pan_id           = pib->pan_id;
    desc.depth                  = nib->depth;
    desc.end_dev_cap            = (nib->ed_cap > 0);
    desc.rtr_cap                = (nib->rtr_cap > 0);
    desc.ext_pan_id             = nib->ext_pan_ID;
    desc.stack_profile          = nib->stack_profile;
    desc.prot_ver               = nib->prot_ver;
    desc.prot_id                = 0;
    desc.superfrm_spec          = mac_gen_superfrm_spec();

    test_check_load_pan_descr(&desc);
}

/**************************************************************************
Function Name: test_gen_check_data

Description:
Generate the data that will be checked against.
**************************************************************************/
static void test_gen_assoc_req_check_data()
{
    mac_hdr_t mac_hdr;
    mac_cmd_t mac_cmd;
    mac_pib_t *pib = mac_pib_get();
    nwk_nib_t *nib = nwk_nib_get();

    memset(&mac_hdr, 0, sizeof(mac_hdr_t));
    mac_hdr.mac_frm_ctrl.frame_type     = MAC_COMMAND;
    mac_hdr.mac_frm_ctrl.frame_pending  = false;
    mac_hdr.mac_frm_ctrl.ack_req        = true;
    mac_hdr.mac_frm_ctrl.pan_id_compr   = true;
    mac_hdr.mac_fcf                     = mac_gen_frm_ctrl(&mac_hdr);
    mac_hdr.dest_pan_id                 = pib->pan_id;
    mac_hdr.dest_addr.mode              = SHORT_ADDR;
    mac_hdr.dest_addr.short_addr        = 0x5678;
    mac_hdr.src_pan_id                  = pib->pan_id;
    mac_hdr.src_addr.mode               = LONG_ADDR;
    mac_hdr.src_addr.long_addr          = pib->ext_addr;
    test_check_load_mac_hdr(&mac_hdr);

    mac_cmd.cmd_id                              = MAC_ASSOC_REQ;
    mac_cmd.cmd_params.assoc_req.cap_info       = nib->capability_info;
    test_check_load_mac_cmd(&mac_cmd);
}

/**************************************************************************
Function Name: test_gen_check_data

Description:
Generate the data that will be checked against.
**************************************************************************/
static void test_gen_data_req_check_data()
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
    mac_hdr.dest_addr.mode              = SHORT_ADDR;
    mac_hdr.dest_addr.long_addr         = 0x5678;
    mac_hdr.src_pan_id                  = pib->pan_id;
    mac_hdr.src_addr.mode               = LONG_ADDR;
    mac_hdr.src_addr.long_addr          = pib->ext_addr;
    test_check_load_mac_hdr(&mac_hdr);

    mac_cmd.cmd_id                      = MAC_DATA_REQ;
    test_check_load_mac_cmd(&mac_cmd);
}

/**************************************************************************
Function Name: test_nwk_join2 thread

Description:
This is the main thread in the function where all the magic happens.
**************************************************************************/
PT_THREAD(test_nwk_join2(struct pt *pt))
{
    PT_BEGIN(pt);
    pan_descr_t *desc = NULL;
    mac_hdr_t hdr;
    mac_cmd_t cmd;
    buffer_t *tx_buf = NULL;
    static buffer_t *rx_buf = NULL;

    DBG_PRINT("TEST_NWK_JOIN2: Started.\n");

    // set up the device
    test_init_device();
    data_received = false;

    // start with a network discovery
    nwk_disc_req(aPhyChannelsSupported, 2);

    // check that the beacon requests are coming. only answer the beacon request on one channel.
    while (1)
    {
        BUF_ALLOC(rx_buf, RX);
        PT_WAIT_UNTIL(pt, (test_driver_read_tx_buf(rx_buf->buf) || data_received));
        if (!data_received)
        {
            DBG_PRINT("TEST_NWK_JOIN2: Beacon request received.\n");
            buf_free(rx_buf);

            if (test_driver_get_channel() == MAC_MAX_CHANNELS + MAC_PHY_CHANNEL_OFFSET - 1)
            {
                test_send_beacon(&hdr);
                test_gen_desc_check_data(test_driver_get_channel(), &hdr);
            }
        }
        else
        {
            DBG_PRINT("\nTEST_NWK_JOIN2: Channel scan is finished.\n");
            buf_free(rx_buf);
            break;
        }
    }

    // set data_received to false. we're gonna use this as a test for the nwk join confirm.
    data_received = false;

    // check the pan descriptor is what we expect and dump it to the screen
    DBG_PRINT("TEST_NWK_JOIN2: Dumping PAN Descriptors.\n\n");
    for (desc = mac_scan_descr_get_head(); desc != NULL; desc = desc->next)
    {
        debug_dump_pan_descr(desc);
        test_check_pan_descr(desc);
    }

    // now we can start the nwk join procedure
    DBG_PRINT("\nTEST_NWK_JOIN2: Starting join procedure. Sending join request with pan descriptor.\n");
    test_gen_assoc_req_check_data();
    test_send_join_req(mac_scan_descr_get_head());

    // wait for the association request
    DBG_PRINT("TEST_NWK_JOIN2: Waiting for association request from device.\n");
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    mac_parse_hdr(rx_buf, &hdr);
    mac_parse_cmd(rx_buf, &cmd);
    test_check_mac(&hdr);
    test_check_mac_cmd(&cmd);
    buf_free(rx_buf);
    DBG_PRINT("TEST_NWK_JOIN2: Association request received.\n");

    // send back the ack for the association request frame
    if (hdr.mac_frm_ctrl.ack_req)
    {
        // gen the ack and send it into the device
        BUF_ALLOC(tx_buf, TX);
        test_gen_ack(tx_buf, &hdr, false);
        test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
        test_driver_rx_isr();
        buf_free(tx_buf);
        PT_WAIT_UNTIL(pt, mac_queue_is_empty(TX));
    }

    // load the data request check data
    test_gen_data_req_check_data();

    // wait for the data request
    DBG_PRINT("TEST_NWK_JOIN2: Waiting for data request from device.\n");
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    mac_parse_hdr(rx_buf, &hdr);
    mac_parse_cmd(rx_buf, &cmd);
    test_check_mac(&hdr);
    test_check_mac_cmd(&cmd);
    buf_free(rx_buf);
    DBG_PRINT("TEST_NWK_JOIN2: Data request received.\n");

    // send back the ack for the data request frame
    if (hdr.mac_frm_ctrl.ack_req)
    {
        // gen the ack and send it into the device
        BUF_ALLOC(tx_buf, TX);
        test_gen_ack(tx_buf, &hdr, true);
        test_driver_write_rx_buf(tx_buf->dptr, tx_buf->len);
        test_driver_rx_isr();
        buf_free(tx_buf);
        PT_WAIT_UNTIL(pt, mac_queue_is_empty(TX));
    }

    // send in the association response frame
    test_send_assoc_resp();

    // association response should return an ack.
    rx_buf = buf_get(RX);
    PT_WAIT_UNTIL(pt, test_driver_read_tx_buf(rx_buf->buf));
    assert(test_check_ack(rx_buf));
    buf_free(rx_buf);

    // wait until the nwk join confirm happens.
    PT_WAIT_UNTIL(pt, data_received);

    // check that all buffers are freed at the end.
    DBG_PRINT("\nTEST_NWK_JOIN2: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));

    finished = true;
    DBG_PRINT("TEST_NWK_JOIN2: Finished.\n");
    PT_END(pt);
}



