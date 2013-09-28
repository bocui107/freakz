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

    Title:

    Description:
*******************************************************************/
#include "contiki.h"
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"

/**************************************************************************
Function Name: test_parse_rx_data

Description:
Parse the rx data.
**************************************************************************/
void test_parse_rx_data(buffer_t *buf, aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr)
{
    mac_parse_hdr(buf, mac_hdr);
    nwk_parse_hdr(buf, nwk_hdr);
    aps_parse_hdr(buf, aps_hdr);
}

/**************************************************************************
Function Name: test_gen_ack

Description:
Generate an ack frame.
**************************************************************************/
void test_gen_ack(buffer_t *buf, mac_hdr_t *hdr_in, bool frm_pend)
{
    mac_hdr_t hdr_out;

    memset(&hdr_out, 0 , sizeof(mac_hdr_t));
    mac_gen_ack(buf, frm_pend, hdr_in->dsn);
}

/**************************************************************************
Function Name: test_check_ack

Description:

**************************************************************************/
bool test_check_ack(buffer_t *buf)
{
    mac_hdr_t hdr;

    mac_parse_hdr(buf, &hdr);
    if (hdr.mac_frm_ctrl.frame_type == MAC_ACK)
    {
        DBG_PRINT("TEST_CHECK_ACK: ACK Received.\n");
        return true;
    }
    else
    {
        return false;
    }
}

