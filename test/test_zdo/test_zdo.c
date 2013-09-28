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

    Title: test_zdo.c

    Description:
    This is the main test file for the zdo layer.
*******************************************************************/
#include "freakz.h"
#include "test_zdo.h"
#include "contiki-main.h"

PROCESS(test_process, "Test Process");
AUTOSTART_PROCESSES(&test_process);
static struct etimer tmr;

void (*test_req[])() =
{
    test_zdo_gen_nwk_addr_req,
    test_zdo_gen_ieee_addr_req,
    test_zdo_gen_node_desc_req,
    test_zdo_gen_pwr_desc_req,
    test_zdo_gen_simple_desc_req,
    test_zdo_gen_active_ep_req,
    test_zdo_gen_match_desc_req,
    test_zdo_gen_dev_annce_req,
    NULL
};

void (*test_resp[])() =
{
    test_zdo_nwk_addr_req_handler,
    test_zdo_node_desc_req_handler,
    test_zdo_pwr_desc_req_handler,
    test_zdo_simple_desc_req_handler,
    test_zdo_active_ep_req_handler,
    test_zdo_match_desc_req_handler,
    NULL
};

/**************************************************************************
Function Name: main

Description:

**************************************************************************/
int main ()
{
    contiki_main();
    return 0;
}

/**************************************************************************
Function Name: test_zdo_check_req_resp

Description:

**************************************************************************/
void test_zdo_check_req_resp(buffer_t *buf, U16 clust_id)
{
    switch (clust_id)
    {
    case NWK_ADDR_REQ_CLUST:
        test_zdo_check_nwk_addr_req(buf);
        break;
    case IEEE_ADDR_REQ_CLUST:
        test_zdo_check_ieee_addr_req(buf);
        break;
    case NODE_DESC_REQ_CLUST:
        test_zdo_check_node_desc_req(buf);
        break;
    case PWR_DESC_REQ_CLUST:
        test_zdo_check_pwr_desc_req(buf);
        break;
    case SIMPLE_DESC_REQ_CLUST:
        test_zdo_check_simple_desc_req(buf);
        break;
    case ACTIVE_EP_REQ_CLUST:
        test_zdo_check_active_ep_req(buf);
        break;
    case MATCH_DESC_REQ_CLUST:
        test_zdo_check_match_desc_req(buf);
        break;
    case DEV_ANNCE_REQ_CLUST:
        test_zdo_check_dev_annce_req(buf);
        break;

    case NWK_ADDR_RESP_CLUST:
        test_zdo_check_addr_resp(buf);
        break;
    case NODE_DESC_RESP_CLUST:
        test_zdo_check_node_desc_resp(buf);
        break;
    case PWR_DESC_RESP_CLUST:
        test_zdo_check_pwr_desc_resp(buf);
        break;
    case SIMPLE_DESC_RESP_CLUST:
        test_zdo_check_simple_desc_resp(buf);
        break;
    case ACTIVE_EP_RESP_CLUST:
        test_zdo_active_ep_resp(buf);
        break;
    case MATCH_DESC_RESP_CLUST:
        test_zdo_check_match_desc_resp(buf);
        break;

    default:
        break;
    }
    buf_free(buf);
}

/**************************************************************************
Function Name: PROCESS_THREAD: test_process

Description:
Test Process
**************************************************************************/
PROCESS_THREAD(test_process, ev, data)
{
    PROCESS_BEGIN();
    static U8 i;
    event_test_proc = process_alloc_event();

    DBG_PRINT_RAW("---------------------------------\n");
    DBG_PRINT_RAW("CHECKING ZDO REQUEST GENERATION\n");
    DBG_PRINT_RAW("---------------------------------\n");
    for (i=0; test_req[i] != NULL; i++)
    {
        test_req[i]();
    }

    DBG_PRINT_RAW("\n---------------------------------\n");
    DBG_PRINT_RAW("CHECKING ZDO RESP GENERATION\n");
    DBG_PRINT_RAW("---------------------------------\n");

    for (i=0; test_resp[i] != NULL; i++)
    {
        test_resp[i]();
        PROCESS_WAIT_EVENT_UNTIL(ev == event_test_proc);
    }

    // check that all buffers are freed before we exit the tests
    etimer_set(&tmr, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    assert(buf_get_cnt() == 0);

    // if we made it here, then all tests passed. yay!
    DBG_PRINT_RAW("\n---------------------------------\n");
    DBG_PRINT_RAW("ZDO TESTS FINISHED.\n");
    DBG_PRINT_RAW("---------------------------------\n");

    PROCESS_END();
}


