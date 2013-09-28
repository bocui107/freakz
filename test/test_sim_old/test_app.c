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

    Title: test_app.c

    Description:
    This file handles the cmd line interface to this node.
*******************************************************************/
#include "freakz.h"
#include "test_app.h"
#include "test_sim.h"

// suppress these lint warnings in this file...
//
//lint --e{818} Info 818: Pointer parameter 'argv' could be declared as pointing to const
//lint --e{715} Info 715: Symbol 'argv' not referenced
// this is just a test file and the argv parameter is just being used for the function pointers

static U8 handle;
static U8 xfer_state;
static bool finished;
static struct
{
    char    fname[80];
    U16     dest_addr;
    FILE    *fp;
} xfer_in, xfer_out;

// simple descriptor for this ep
static U8 test_app_simple_desc[] = {
    TEST_EP,                    // ep
    TEST_PROFILE_ID,            // profile id
    0,                          // dev id
    1,                          // dev ver
    2,                          // num in clusters
    TEST_IN_CLUST,              // in clusters
    TEST_SET_LONG_XFER_CLUST,
    0                           // num out clusters
};

// command table for node
static test_app_cmd_t test_app_cmd_tbl[] =
{
    {"zs",      test_app_zdo_start,},
    {"dn",      test_app_dump_nib,},
    {"dp",      test_app_dump_pib,},
    {"zud",     test_app_unicast_data_req,},
    {"zudt",    test_app_unicast_data_req_tree,},
    {"zurd",    test_app_unicast_rel_data_req,},
    {"xlf",     test_app_xfer_long_file,},
    {"gbc",     test_app_get_buf_cnt,},
    {NULL, NULL}
};

/**************************************************************************
Function Name: test_app_init

Description:

**************************************************************************/
void test_app_init()
{
    xfer_state = TESTAPP_NORMAL;
    finished = false;

    // generate the handle
    handle = (U8)drvr_get_rand();
    af_ep_add(TEST_EP, test_app_simple_desc, AF_BASE_SIMPLE_DESC_SIZE + 4, false, test_app_rx_handler, test_app_conf_handler);
}

/**************************************************************************
Function Name: test_app_get_handle

Description:
Generate a valid handle to be used to identify the data transfer. If the
handle is invalid, then generate a different one and use it.
**************************************************************************/
U8 test_app_get_handle()
{
    // if the handle is invalid (0), then increment it so that it won't get used.
    if (handle == ZIGBEE_INVALID_HANDLE)
    {
        ++handle;
    }
    return handle++;
}

/**************************************************************************
Function Name: test_app_zdo_start

Description:
**************************************************************************/
void test_app_zdo_start(U8 argc, char **argv)
{
    aps_aib_t *aib = aps_aib_get();

    if (argc == 2)
    {
        if (!strcmp(argv[1], "c"))
        {
            aib->desig_coord = true;
        }
        else
        {
            aib->desig_coord = false;
            aib->desig_parent = (U16)strtol(argv[1], NULL, 10);
            aib->use_desig_parent = true;
            DBG_PRINT("ZDO_CMD_ZDO_START: Designated to join %04X\n", aib->desig_parent);
        }
    }
    else
    {
        aib->desig_coord = false;
    }

    DBG_PRINT("ZDO_CMD_ZDO_START: Starting as a %s.\n", aib->desig_coord ? "Coordinator" : "Router");
    zdo_nwk_start();
}

/**************************************************************************
Function Name: test_app_dump_nib

Description:
**************************************************************************/
void test_app_dump_nib(U8 argc, char **argv)
{
    debug_dump_nib();
}

/**************************************************************************
Function Name: test_app_dump_pib

Description:
**************************************************************************/
void test_app_dump_pib(U8 argc, char **argv)
{
    debug_dump_pib();
}

/**************************************************************************
Function Name: zdo_nwk_form_req

Description:
**************************************************************************/
void test_app_form_req(U8 argc, char **argv)
{
    nwk_form_req(aPhyChannelsSupported, DEFAULT_SCAN_DURATION);
}

/**************************************************************************
Function Name: test_app_get_buf_cnt

Description:

**************************************************************************/
void test_app_get_buf_cnt(U8 argc, char **argv)
{
    DBG_PRINT_RAW("CURRENT BUFFERS IN USE: %d.\n", buf_get_cnt());
}

/**************************************************************************
Function Name: test_app_extract_data

Description:
Extract the dest addr, data, and len from the argv string.
**************************************************************************/
U8 test_app_extract_data(U8 *data, U16 *dest_addr, char **argv)
{
    U8 i;
    U16 dest;

    // argv[1] = dest_addr, rest is data
    dest =  (U16)strtol(argv[1], NULL, 16);

    DBG_PRINT("Data for transmission ");
    for (i=1; argv[i] != NULL; i++)
    {
        data[i-2] = (U8)strtol(argv[i], NULL, 16);

        if (i>1)
        {
            DBG_PRINT_RAW("%02X ", data[i-2]);
        }
    }
    DBG_PRINT_RAW("\n");
    *dest_addr = dest;
    return i-2;
}

/**************************************************************************
Function Name: test_app_send_next

Description:
Send the next MAX_APS_PAYLOAD bytes for the transfer. If we reach EOF, then send the remaining
bytes.
**************************************************************************/
void test_app_send_next()
{
    char c;
    U8 i, data_handle, data[128];

    if (!finished)
    {
        for (i=0; i<MAX_APS_PAYLOAD; i++)
        {
            //lint -e{734} Info 734: Loss of precision (31 bits to 7 bits)
            // NOTE: comparing c to a const definition (default 32 bit signed)
            if ((c = getc(xfer_out.fp)) == EOF)
            {
                finished = true;
                break;
            }
            //lint -e{732} Loss of sign (assignment) (char to unsigned char)
            // Note: for testing only. we're reading from a file which is all characters.
            data[i] = c;
        }

        data_handle = test_app_get_handle();
        af_tx(data, i, TEST_EP, xfer_out.dest_addr, TEST_EP, TEST_IN_CLUST, 0, ZIGBEE_DEFAULT_RADIUS, data_handle);
    }
    else
    {
        finished = false;
        xfer_state = TESTAPP_NORMAL;
        af_tx(NULL, 0, TEST_EP, xfer_out.dest_addr, TEST_EP, TEST_SET_LONG_XFER_CLUST, 0, ZIGBEE_DEFAULT_RADIUS,  test_app_get_handle());
        fclose(xfer_out.fp);
    }
}

/**************************************************************************
Function Name: test_app_xfer_long_file

Description:

**************************************************************************/
void test_app_xfer_long_file(U8 argc, char **argv)
{
    sim_node_t *nd = node_get();

    // set the xfer state
    xfer_state = TESTAPP_LONG;

    // argv[1] = dest_addr, rest is data
    xfer_out.dest_addr =  (U16)strtol(argv[1], NULL, 16);

    sprintf(xfer_out.fname, "./log/test_%d.txt", nd->index);
    if ((xfer_out.fp = fopen(xfer_out.fname, "r")) == NULL)
    {
        DBG_PRINT_RAW("ERROR: Cannot open file %s.\n", xfer_out.fname);
    }

    af_tx(NULL, 0, TEST_EP, xfer_out.dest_addr, TEST_EP, TEST_SET_LONG_XFER_CLUST, 0, ZIGBEE_DEFAULT_RADIUS, test_app_get_handle());
}

/**************************************************************************
Function Name: test_app_unicast_rel_data_req

Description:
Send data via the app sublayer data request service using aps level ack.
**************************************************************************/
void test_app_unicast_rel_data_req(U8 argc, char **argv)
{
    U8 len, data_handle, data[128];
    U16 dest_addr;

    len = test_app_extract_data(data, &dest_addr, argv);
    data_handle = test_app_get_handle();
    af_tx(data, len, TEST_EP, dest_addr, TEST_EP, TEST_IN_CLUST, 1<<APS_TX_REQ_ACK_TX_OFF, ZIGBEE_DEFAULT_RADIUS, data_handle);
}

/**************************************************************************
Function Name: test_app_unicast_data_req_tree

Description:
Send data via the app sublayer data request service. No route discovery.
**************************************************************************/
void test_app_unicast_data_req_tree(U8 argc, char **argv)
{
    U8 len, data_handle, data[128];
    U16 dest_addr;

    len = test_app_extract_data(data, &dest_addr, argv);
    data_handle = test_app_get_handle();
    af_tx(data, len, TEST_EP, dest_addr, TEST_EP, TEST_IN_CLUST, 1<<APS_TX_RTE_DISC_DISABLE, ZIGBEE_DEFAULT_RADIUS, data_handle);
}

/**************************************************************************
Function Name: test_app_unicast_data_req

Description:
Send data via the app sublayer data request service.
**************************************************************************/
void test_app_unicast_data_req(U8 argc, char **argv)
{
    U8 len, data_handle, data[128];
    U16 dest_addr;

    len = test_app_extract_data(data, &dest_addr, argv);
    data_handle = test_app_get_handle();
    af_tx(data, len, TEST_EP, dest_addr, TEST_EP, TEST_IN_CLUST, 0, ZIGBEE_DEFAULT_RADIUS, data_handle);
}

/**************************************************************************
Function Name: test_app_parse

Description:
**************************************************************************/
void test_app_parse(char *data)
{
    U8 argc, i = 0;
    char *argv[30];

    DBG_PRINT("/-----------------/\n");
    DBG_PRINT("CMD: %s\n", data);
    DBG_PRINT("/-----------------/\n");
    fflush(stdout);

    argv[i] = strtok(data, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));

    argc = i;
    for (i=0; test_app_cmd_tbl[i].cmd != NULL; i++)
    {
        if (!strcmp(argv[0], test_app_cmd_tbl[i].cmd))
        {
            test_app_cmd_tbl[i].func(argc, argv);
            return;
        }
    }
    DBG_PRINT("ZDO_CMD: Command not recognized.\n");
}

/**************************************************************************
Function Name: test_app_conf_handler

Description:

**************************************************************************/
void test_app_conf_handler(U8 status, U8 in_handle)
{
    char *status_tbl[] = {
        "APP_SUCCESS",
        "APP_NO_FREE_BUFS",
        "APP_TX_FAILED",
        "APP_INVALID_HANDLE"
    };

    DBG_PRINT_RAW("TESTAPP_CONF_RCVD: Status: %s, Handle: %x.\n", status_tbl[status], in_handle);

    if (xfer_state == TESTAPP_LONG)
    {
        // this delay is needed or else we're going to flood the remote node
        // if we're doing non-APS-ACK'd transmissions
        busy_wait(100);

        // send the next batch of data to the remote node
        test_app_send_next();
    }
}

/**************************************************************************
Function Name: test_app_rx_handler

Description:

**************************************************************************/
void test_app_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 dest_clust)
{
    U8 i;
    sim_node_t *nd = node_get();

    switch (dest_clust)
    {
    case TEST_SET_LONG_XFER_CLUST:
        if (xfer_state == TESTAPP_NORMAL)
        {
            DBG_PRINT("TESTAPP: Xfer is started.\n");

            // set up to receive a long transfer
            sprintf(xfer_in.fname, "./log/test_%d.txt", nd->index);
            if ((xfer_in.fp = fopen(xfer_in.fname, "w")) == NULL)
            {
                DBG_PRINT_RAW("ERROR: Cannot open file %s.\n", xfer_in.fname);
            }

            // set the state
            xfer_state = TESTAPP_LONG;
        }
        else if (xfer_state == TESTAPP_LONG)
        {
            DBG_PRINT("TESTAPP: Xfer is finished.\n");
            fclose(xfer_in.fp);
            xfer_state = TESTAPP_NORMAL;
        }

        break;
    case TEST_IN_CLUST:
        DBG_PRINT("TESTAPP_DATA_RCVD: ");
        for (i=0; i<len; i++)
        {
            DBG_PRINT_RAW("%02X ", data[i]);
            if (xfer_state == TESTAPP_LONG)
            {
                putc(data[i], xfer_in.fp);
                fflush(xfer_in.fp);
            }
        }
        DBG_PRINT_RAW("\n");
        break;
    default:
        break;
    }
}
