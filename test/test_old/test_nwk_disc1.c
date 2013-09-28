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

    Title: test_nwk_disc1.c

    Description:
    This test performs a network discovery request. Once the request is
    issued, the device will send out a beacon request. We will respond with
    one beacon per channel. Once the device is finished, then it should dump
    the pan descriptor list.
*******************************************************************/
#include "zigbee.h"
#include "contiki.h"
#include "test.h"
#include "test_driver.h"

extern bool finished;
extern bool data_received;

/**************************************************************************
Function Name: test_send_beacon

Description:
Send a beacon in response to a beacon request.
**************************************************************************/
static void test_send_beacon(mac_hdr_t *hdr)
{
    buffer_t *buf;

    memset(hdr, 0, sizeof(mac_hdr_t));

    hdr->mac_frm_ctrl.frame_type   = MAC_BEACON;
    hdr->mac_frm_ctrl.ack_req       = false;
    hdr->mac_frm_ctrl.pan_id_compr  = false;
    hdr->src_pan_id                 = rand();
    hdr->src_addr.mode              = SHORT_ADDR;
    hdr->src_addr.short_addr        = rand();

    DBG_PRINT("TEST_NWK_DISC: Sending beacon back in response to beacon request.\n");
    BUF_ALLOC(buf, TX);
    mac_gen_beacon(buf);
    mac_gen_header(buf, hdr);

    test_driver_write_rx_buf(buf->dptr, buf->len);
    test_driver_rx_isr();
    buf_free(buf);
}

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
    nib->depth          = 0x3;
    nib->ed_cap         = 0x3;
    nib->rtr_cap        = 0x3;
    nib->ext_pan_ID     = 0x1122334455667788ULL;

    pib->coord          = true;
    pib->assoc_permit   = true;
}

/**************************************************************************
Function Name: test_nwk_disc1

Description:
This is the main test function thread.
**************************************************************************/
PT_THREAD(test_nwk_disc1(struct pt *pt))
{
    PT_BEGIN(pt);
    mac_hdr_t hdr;
    pan_descr_t *desc;
    static buffer_t *rx_buf;

    DBG_PRINT("TEST_NWK_DISC: Started.\n");
    test_init_device();

    data_received = false;
    nwk_disc_req(aPhyChannelsSupported, 4);

    while (1)
    {
        rx_buf = buf_get(RX);
        PT_WAIT_UNTIL(pt, (test_driver_read_tx_buf(rx_buf->buf) || data_received));
        if (!data_received)
        {
            DBG_PRINT("TEST_NWK_DISC: Frame Received.\n");
            buf_free(rx_buf);
            test_send_beacon(&hdr);
        }
        else
        {
            DBG_PRINT("\nTEST_NWK_DISC: Channel scan is finished.\n");
            buf_free(rx_buf);
            break;
        }
    }

    DBG_PRINT("TEST_NWK_DISC: Dumping PAN Descriptors.\n\n");
    for (desc = mac_scan_descr_get_head(); desc != NULL; desc = desc->next)
    {
        debug_dump_pan_descr(desc);
        DBG_PRINT("\n");
    }
    mac_scan_descr_clear_all();
    if (mac_scan_descr_get_head() != NULL) assert(0);

    // check that all buffers are freed at the end.
    DBG_PRINT("\nTEST_NWK_DISC: Buf Cnt = %02d. \n", buf_get_cnt());
    assert((buf_get_cnt() == 0));

    finished = true;
    DBG_PRINT("TEST_NWK_DISC: Finished.\n");

    PT_END(pt);
}



