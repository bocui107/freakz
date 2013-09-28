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

    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title: ep_test.c

    Description:
    This file handles the command line interface to the zdo.
*******************************************************************/
#include "zigbee.h"
#include "ep_test.h"

// suppress these lint warnings in this file...
//
//lint --e{818} Info 818: Pointer parameter 'argv' could be declared as pointing to const
//lint --e{715} Info 715: Symbol 'argv' not referenced
// this is just a test file and the argv parameter is just being used for the function pointers

static FILE *fin;
static U8 ep_test_state;

// command table for node
static ep_test_t ep_test_tbl[] =
{
    {"zs", ep_test_zdo_start,},
    {"dn", ep_test_dump_nib,},
    {"dp", ep_test_dump_pib,},
    {"zud", ep_test_unicast_data_req,},
    {"zudt", ep_test_unicast_data_req_tree,},
    {"zurd", ep_test_unicast_rel_data_req,},
    {"zbr", ep_test_brc_data_req,},
    {"xlf", ep_test_xfer_long_file,},
    {NULL, NULL}
};

static simple_desc_t ep_test_desc = {
    1,                      // endpoint
    FREAKZ_GENERIC_EP,      // profile id
    0,                      // dev id
    1,                      // dev ver
    0,                      // out clust
    1                       // in clust
};

/**************************************************************************
Function Name: ep_test_init

Description:

**************************************************************************/
void ep_test_init()
{
    app_cb_t *app_cb = zdo_app_cb_get();
    app_cb->app_data_conf_cb = ep_test_data_conf_cb;
    ep_test_state = NORMAL;
    af_register_ep(&ep_test_desc, 8, ep_rx_handler, ep_conf);
}

/**************************************************************************
Function Name: ep_test_data_conf_cb

Description:

**************************************************************************/
void ep_test_data_conf_cb(U8 status, U8 handle)
{
    switch (ep_test_state)
    {
    case NORMAL:
        DBG_PRINT_RAW("TEST_CMD_DATA_CONF: Status = %d, Handle = %d.\n", status, handle);
    case XFER_LONG_FILE_STATE:
        if (status == APP_SUCCESS)
        {
            // TODO : Finish this
        }
        break;
    default:
        break;
    }
}

/**************************************************************************
Function Name: ep_test_zdo_start

Description:
**************************************************************************/
void ep_test_zdo_start(U8 argc, char **argv)
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
Function Name: ep_test_dump_nib

Description:
**************************************************************************/
void ep_test_dump_nib(U8 argc, char **argv)
{
    debug_dump_nib();
}

/**************************************************************************
Function Name: ep_test_dump_pib

Description:
**************************************************************************/
void ep_test_dump_pib(U8 argc, char **argv)
{
    debug_dump_pib();
}

/**************************************************************************
Function Name: zdo_nwk_form_req

Description:
**************************************************************************/
void ep_test_form_req(U8 argc, char **argv)
{
    nwk_form_req(aPhyChannelsSupported, DEFAULT_SCAN_DURATION);
}

/**************************************************************************
Function Name: ep_test_send_data

Description:

**************************************************************************/
void ep_test_send_data(buffer_t *buf, U16 dest_addr, U8 tx_opt)
{
    aps_data_req_t req;

    // generate the aps data request
    req.dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req.dest_addr.mode          = SHORT_ADDR;
    req.dest_addr.short_addr    = dest_addr;
    req.dest_ep                 = ZDO_EP;
    req.prof_id                 = 0;
    req.clust_id                = GENERIC_DATA_RX_CLUST;
    req.src_ep                  = ZDO_EP;
    req.asdu_len                = buf->len;
    req.buf                     = buf;
    req.tx_opt                  = tx_opt;
    req.radius                  = 0x6;

    // send it into the aps tx queue
    aps_data_req(&req);
}

/**************************************************************************
Function Name: ep_test_xfer_long_file

Description:

**************************************************************************/
void ep_test_xfer_long_file(U8 argc, char **argv)
{
    char c, fname[30];
    U8 tmp[128], cnt;
    buffer_t *buf;
    U16 dest_addr;
    sim_node_t *nd = node_get();

    // argv[1] = dest_addr, rest is data
    dest_addr =  (U16)strtol(argv[1], NULL, 16);

    sprintf(fname, "./scripts/test_%d.txt", nd->index);
    if ((fin = fopen(fname, "r")) == NULL)
    {
        DBG_PRINT_RAW("ERROR: Cannot open file %s.\n", fname);
    }

    cnt = 0;
    while ((c = getc(fp)) != EOF)
    {
        tmp[cnt] = c;
        if (cnt == MAX_APS_PAYLOAD)
        {
            // send out the data in the tmp array
            BUF_ALLOC(buf, TX);
            buf->dptr -= cnt;
            buf->len += cnt;
            memcpy(buf->dptr, tmp, cnt);
            ep_test_send_data(buf, dest_addr, 0);
            cnt = 0;
        }
        else
        {
            cnt++;
        }
    }

    // send out the remaining data in the tmp array
    if (cnt != 0)
    {
        BUF_ALLOC(buf, TX);
        buf->dptr -= cnt;
        buf->len += cnt;
        memcpy(buf->dptr, tmp, cnt);
        ep_test_send_data(buf, dest_addr, 0);
    }
}

/**************************************************************************
Function Name: ep_test_brc_data_req

Description:
Send data via the app sublayer data request service.
**************************************************************************/
void ep_test_brc_data_req(U8 argc, char **argv)
{
    buffer_t *buf;
    U8 i, len, data[128];

    DBG_PRINT("Data for transmission ");
    for (i=1; argv[i] != NULL; i++)
    {
        data[i-1] = (U8)strtol(argv[i], NULL, 16);

        if (i>1)
        {
            DBG_PRINT_RAW("%02X ", data[i-2]);
        }
    }
    DBG_PRINT_RAW("\n");
    len = i-1;

    // fill in the buffer
    BUF_ALLOC(buf, TX);
    buf->dptr -= len;
    buf->len += len;
    memcpy(buf->dptr, data, len);
    ep_test_send_data(buf, MAC_BROADCAST_ADDR, 0);
}

/**************************************************************************
Function Name: ep_test_unicast_rel_data_req

Description:
Send data via the app sublayer data request service using aps level ack.
**************************************************************************/
void ep_test_unicast_rel_data_req(U8 argc, char **argv)
{
    buffer_t *buf;
    U16 dest_addr;
    U8 i, len, data[128];

    // argv[1] = dest_addr, rest is data
    dest_addr =  (U16)strtol(argv[1], NULL, 16);

    DBG_PRINT("Data for transmission ");
    for (i=2; argv[i] != NULL; i++)
    {
        data[i-2] = (U8)strtol(argv[i], NULL, 16);
        DBG_PRINT_RAW("%02X ", data[i-2]);
    }
    DBG_PRINT_RAW("\n");
    len = i-2;

    // fill in the buffer
    BUF_ALLOC(buf, TX);
    buf->dptr -= len;
    buf->len += len;
    memcpy(buf->dptr, data, len);
    ep_test_send_data(buf, dest_addr, 1<<APS_TX_REQ_ACK_TX_OFF);
}

/**************************************************************************
Function Name: ep_test_unicast_data_req_tree

Description:
Send data via the app sublayer data request service. No route discovery.
**************************************************************************/
void ep_test_unicast_data_req_tree(U8 argc, char **argv)
{
    buffer_t *buf;
    U16 dest_addr;
    U8 i, len, data[128];

    // argv[1] = dest_addr, rest is data
    dest_addr =  (U16)strtol(argv[1], NULL, 16);

    DBG_PRINT("Data for transmission ");
    for (i=2; argv[i] != NULL; i++)
    {
        data[i-2] = (U8)strtol(argv[i], NULL, 16);
        DBG_PRINT_RAW("%02X ", data[i-2]);
    }
    DBG_PRINT_RAW("\n");
    len = i-2;

    // fill in the buffer
    BUF_ALLOC(buf, TX);
    buf->dptr -= len;
    buf->len += len;
    memcpy(buf->dptr, data, len);
    ep_test_send_data(buf, dest_addr,1<<APS_TX_RTE_DISC_DISABLE);
}

/**************************************************************************
Function Name: ep_test_unicast_data_req

Description:
Send data via the app sublayer data request service.
**************************************************************************/
void ep_test_unicast_data_req(U8 argc, char **argv)
{
    U16 dest_addr;
    U8 i, len, data[128];

    // argv[1] = dest_addr, rest is data
    dest_addr =  (U16)strtol(argv[1], NULL, 16);

    DBG_PRINT("Data for transmission ");
    for (i=2; argv[i] != NULL; i++)
    {
        data[i-2] = (U8)strtol(argv[i], NULL, 16);
        DBG_PRINT_RAW("%02X ", data[i-2]);
    }
    DBG_PRINT_RAW("\n");
    len = i-2;

    zdo_data_req(data, len, 0, dest_addr, GENERIC_DATA_RX_CLUST, 0);
}

/**************************************************************************
Function Name: ep_test_parse

Description:
**************************************************************************/
void ep_test_parse(char *data)
{
    U8 argc, i = 0;
    char *argv[30];

    argv[i] = strtok(data, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));

    argc = i;
    for (i=0; ep_test_tbl[i].cmd != NULL; i++)
    {
        if (!strcmp(argv[0], ep_test_tbl[i].cmd))
        {
            ep_test_tbl[i].func(argc, argv);
            return;
        }
    }
    DBG_PRINT("ZDO_CMD: Command not recognized.\n");
}
