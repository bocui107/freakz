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
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "sim.h"
#include "zigbee.h"
#include "test_driver.h"

/*---------------------------------------------------------------------------*/
PROCESS(sim_process, "Simulation Process");

//static struct etimer tmr;
static sim_node_t self_node;
extern process_event_t event_cmd_rcvd;
/*---------------------------------------------------------------------------*/

/**************************************************************************
Function Name: sim_get_self

Description:
Return the node struct pointer to the calling process.
**************************************************************************/
sim_node_t *sim_get_self()
{
    return &self_node;
}

/**************************************************************************
Function Name: sim_check_readpipe

Description:

**************************************************************************/
bool sim_check_readpipe()
{
    fd_set fdset;
    struct timeval tv;
    int ret;
    int max_fd;

    // set polling interval to zero which makes the select non-blocking.
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fdset);
    FD_SET(self_node.data_rx[READPIPE], &fdset);
    FD_SET(self_node.cmd_rx[READPIPE], &fdset);

    // get the max fd number to use for the select call.
    // the select call scans the fdset bitmap up to the specified fd number so we need to get the max and add 1 to it.
    max_fd = (self_node.data_rx[READPIPE] > self_node.cmd_rx[READPIPE]) ? self_node.data_rx[READPIPE] + 1 : self_node.cmd_rx[READPIPE] + 1;
    ret = select(max_fd, &fdset, NULL, NULL, &tv);

    if (FD_ISSET(self_node.data_rx[READPIPE], &fdset))
    {
        if ((read(self_node.data_rx[READPIPE], self_node.buf, MSGBUFSIZE)) != -1)
        {
            test_driver_write_rx_buf(self_node.buf, self_node.buf[0]);
            test_driver_rx_isr();
        }
        else
        {
            DBG_PRINT("Read failed on the data pipe.\n");
        }
        return true;
    }
    else if (FD_ISSET(self_node.cmd_rx[READPIPE], &fdset))
    {
        if ((read(self_node.cmd_rx[READPIPE], self_node.buf, MSGBUFSIZE)) != -1)
        {
            DBG_PRINT("Cmd Rcvd: %s\n", self_node.buf);
            process_post(&zdo_cmd_process, event_cmd_rcvd, self_node.buf);
        }
        else
        {
            DBG_PRINT("Read failed on the data pipe.\n");
        }
        return true;
    }
    else
    {
        return false;
    }
}

void sim_pollhandler()
{
    process_poll(&sim_process);
}

/**************************************************************************
Function Name: process_sim_process

Description:
The main process to handle the simulation
**************************************************************************/
PROCESS_THREAD(sim_process, ev, data)
{
    PROCESS_POLLHANDLER(sim_pollhandler());

    PROCESS_BEGIN();

    // init all the layers and the poller for this process
    process_poll(&sim_process);
    ctimer_init();
    mac_init();
    nwk_init();
    aps_init();
    zdo_init();
    buf_init();
    test_driver_init();

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(sim_check_readpipe());
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
