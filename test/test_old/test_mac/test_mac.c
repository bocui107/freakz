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

    Email: chris@oxbeef.com
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title:

    Description:
*******************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "contiki.h"
#include "zigbee.h"
#include "test_driver.h"

/*********************** Global Variables **************************/
extern process_event_t rx_event, mac_req_event;
/*********************** Process Declarations **********************/
PROCESS(test_mac_process, "Test MAC Process");
AUTOSTART_PROCESSES(&test_mac_process);
/*******************************************************************/


/**************************************************************************
Function Name: test_mac_gen_mac_data

Description:
Generate data to be used in the MAC layer.
**************************************************************************/
void test_mac_gen_mac_data(mpdu_t *mpdu)
{
    memset(mpdu, 0, sizeof(mpdu_t));

    mpdu->hdr.ack_req                    = false;
    mpdu->hdr.dest_pan_id                = 0x1111;
    mpdu->hdr.dest_addr.mode             = SHORT_ADDR;
    mpdu->hdr.dest_addr.short_addr       = 0x1234;
    //mpdu->hdr.dest_addr.mode             = LONG_ADDR;
    //mpdu->hdr.dest_addr.long_addr        = 0x1234123412341234ULL;
    mpdu->hdr.pan_id_compr               = false;
    mpdu->hdr.src_pan_id                 = 0xffff;
    mpdu->hdr.src_addr.mode              = SHORT_ADDR;
    mpdu->hdr.src_addr.short_addr        = 0x5678;
    //mpdu->hdr.src_addr.mode              = LONG_ADDR;
    //mpdu->hdr.src_addr.long_addr         = 0x0FEDCBA987654321ULL;

    mpdu->hdr.frame_pending              = false;
    mpdu->hdr.frame_type                 = DATA;
}

/**************************************************************************
Function Name: test_mac_gen_nwk_data

Description:
Generate data to be used in the NWK layer.
**************************************************************************/
void test_mac_gen_nwk_data(npdu_t *npdu)
{
    memset(npdu, 0 , sizeof(npdu_t));

    npdu->hdr.nwk_frm_ctrl.frame_type           = NWK_DATA_FRM;
    npdu->hdr.nwk_frm_ctrl.disc_route           = false;
    npdu->hdr.nwk_frm_ctrl.protocol_ver         = 0x1;
    npdu->hdr.nwk_frm_ctrl.security             = false;
    //npdu->hdr.nwk_frm_ctrl.dest_ieee_addr_flag  = true;
    //npdu->hdr.nwk_frm_ctrl.src_ieee_addr_flag   = true;

    npdu->hdr.dest_addr                         = 0x1234;
    npdu->hdr.radius                            = 0x3;
    npdu->hdr.src_addr                          = 0x5678;
    //npdu->hdr.dest_ieee_addr                    = 0x123456789ABCDEF0ULL;
    //npdu->hdr.src_ieee_addr                     = 0x1122334455667788ULL;
}

/**************************************************************************
Function Name: test_mac_gen_aps_data

Description:
Generate the data for the aps layer.
**************************************************************************/
void test_mac_gen_aps_data(apdu_t *apdu)
{
    apdu->hdr.aps_frm_ctrl.ack_req          = true;
    apdu->hdr.aps_frm_ctrl.delivery_mode    = APS_UNICAST;
    apdu->hdr.aps_frm_ctrl.frm_type         = APS_DATA_FRM;
    apdu->hdr.aps_frm_ctrl.ind_addr_mode    = 0x1;
    apdu->hdr.aps_frm_ctrl.security         = false;

    apdu->hdr.clust_id  = 0x2222;
    apdu->hdr.dest_ep   = 0x11;
    //apdu->hdr.grp_addr  = 0x3333;
    apdu->hdr.prof_id   = 0x6666;
    apdu->hdr.src_ep    = 0x88;
    apdu->hdr.aps_ctr   = 0x03;
}

/**************************************************************************
Function Name: test_mac_send_data

Description:
Generate and send data to the DUT.
**************************************************************************/
void test_mac_send_data()
{
    buffer_t *txbuf = buf_get(TX);
    apdu_t apdu;
    npdu_t npdu;
    mpdu_t mpdu;

    // gen the header data
    test_mac_gen_aps_data(&apdu);
    test_mac_gen_nwk_data(&npdu);
    test_mac_gen_mac_data(&mpdu);

    // compile the data into a valid header. order is important.
    aps_gen_header(txbuf, &apdu);
    nwk_gen_header(txbuf, &npdu);
    mac_gen_header(txbuf, &mpdu);

    test_driver_write_rx_buf(txbuf->dptr, txbuf->len);
    test_driver_rx_isr();
}


void test()
{
    buffer_t *buf1 = buf_get(TX);
    buffer_t *buf2 = buf_get(TX);
    buffer_t *buf3 = buf_get(TX);
    mpdu_t mpdu;

    test_mac_gen_mac_data(&mpdu);

    if (buf1)
    {
        mac_gen_header(buf1, &mpdu);
        mac_tx_enqueue(buf1);
    }

    if (buf2)
    {
        mac_gen_header(buf2, &mpdu);
        mac_tx_enqueue(buf2);
    }

    if (buf3)
    {
        mac_gen_header(buf3, &mpdu);
        mac_tx_enqueue(buf3);
    }

}

/**************************************************************************
Function Name: PROCESS_THREAD: test_mac_process

Description:
The mac test.
**************************************************************************/
PROCESS_THREAD(test_mac_process, ev, data)
{
    PROCESS_BEGIN();
    static struct etimer tmr;
    queue_init();
    mac_init();
    nwk_init();
    buf_init();
    test_driver_init();
    etimer_set(&tmr, CLOCK_SECOND);

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&tmr));
        test_mac_send_data();
        //test();
        etimer_set(&tmr, CLOCK_SECOND);
    }
    PROCESS_END();
}




