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
#include <string.h>
#include <assert.h>
#include "contiki.h"
#include "zigbee.h"
#include "test.h"
#include "test_driver.h"
#include "ctimer.h"

/*********************** Global Variables **************************/
bool finished;
static U8  i;
static struct pt thr;

PT_THREAD((*test[]))(struct pt *pt) =
{
    test_data_in,
    test_data_out,
    test_data_out_ack,
    test_data_in_rte1,
    test_data_in_rte2,
    test_brc_rtr,
    test_brc_rtr1,
    test_indirect,
    test_nwk_disc,
    test_nwk_disc1,
    test_nwk_join1,
    test_nwk_join2,
    NULL
};

/*********************** Process Declarations **********************/
PROCESS(test_process, "Test Process");
AUTOSTART_PROCESSES(&test_process);
/*******************************************************************/

void test_poll()
{
    test[i](&thr);
    process_poll(&test_process);
}

/**************************************************************************
Function Name: PROCESS_THREAD: test_process

Description:
Test Process
**************************************************************************/
PROCESS_THREAD(test_process, ev, data)
{
    PROCESS_POLLHANDLER(test_poll());

    PROCESS_BEGIN();

    finished = false;
    process_poll(&test_process);
    ctimer_init();
    mac_init();
    nwk_init();
    aps_init();
    zdo_init();
    buf_init();
    test_driver_init();

    for (i=0; test[i] != NULL; i++)
    {
        DBG_PRINT("\n-----------------------------------------------\n");
        DBG_PRINT("Currently running test [%x].\n", i);
        DBG_PRINT("-----------------------------------------------\n");

        PT_INIT(&thr);
        test[i](&thr);

        PROCESS_WAIT_UNTIL(finished == true);
        DBG_PRINT("TEST[%d] finished.\n", i);
        finished = false;
    }
    DBG_PRINT("TESTS FINISHED.\n");

    PROCESS_END();
}




