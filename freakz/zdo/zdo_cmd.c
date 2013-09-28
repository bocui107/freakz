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

    Title: zdo_cmd.c

    Description:
    This file handles the command line interface to the zdo.
*******************************************************************/
#include "zigbee.h"

// suppress these lint warnings in this file...
//
//lint --e{818} Info 818: Pointer parameter 'argv' could be declared as pointing to const
// this is not an actual stack file and the argv parameter is just being used for the function pointers

//lint --e{715} Info 715: Symbol 'argv' not referenced

#if (TEST_SIM)
// command table for node
static zdo_cmd_t zdo_cmd_tbl[] =
{
    {"zs", zdo_cmd_zdo_start,},
    {"dn", zdo_cmd_dump_nib,},
    {"dp", zdo_cmd_dump_pib,},
    {"zud", zdo_cmd_unicast_data_req,},
    {"zudt", zdo_cmd_unicast_data_req_tree,},
    {"zurd", zdo_cmd_unicast_rel_data_req,},
    {"zbr", zdo_cmd_brc_data_req,},
    {NULL, NULL}
};

/**************************************************************************
Function Name: zdo_cmd_zdo_start

Description:
**************************************************************************/
void zdo_cmd_zdo_start(U8 argc, char **argv)
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
Function Name: zdo_cmd_dump_nib

Description:
**************************************************************************/
void zdo_cmd_dump_nib(U8 argc, char **argv)
{
    debug_dump_nib();
}

/**************************************************************************
Function Name: zdo_cmd_dump_pib

Description:
**************************************************************************/
void zdo_cmd_dump_pib(U8 argc, char **argv)
{
    debug_dump_pib();
}

/**************************************************************************
Function Name: zdo_nwk_form_req

Description:
**************************************************************************/
void zdo_cmd_form_req(U8 argc, char **argv)
{
    nwk_form_req(aPhyChannelsSupported, DEFAULT_SCAN_DURATION);
}

/**************************************************************************
Function Name: zdo_cmd_brc_data_req

Description:
Send data via the app sublayer data request service.
**************************************************************************/
void zdo_cmd_brc_data_req(U8 argc, char **argv)
{
    aps_data_req_t req;
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

    // generate the aps data request
    req.dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req.dest_addr.mode          = SHORT_ADDR;
    req.dest_addr.short_addr    = MAC_BROADCAST_ADDR;
    req.dest_ep                 = ZDO_EP;
    req.prof_id                 = 0;
    req.clust_id                = 0;
    req.src_ep                  = ZDO_EP;
    req.asdu_len                = len;
    req.buf                     = buf;
    req.tx_opt                  = 0x0;
    req.radius                  = 0x6;

    // send it into the aps tx queue
    aps_data_req(&req);
}

/**************************************************************************
Function Name: zdo_cmd_unicast_rel_data_req

Description:
Send data via the app sublayer data request service using aps level ack.
**************************************************************************/
void zdo_cmd_unicast_rel_data_req(U8 argc, char **argv)
{
    aps_data_req_t req;
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

    // generate the aps data request
    req.dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req.dest_addr.mode          = SHORT_ADDR;
    req.dest_addr.short_addr    = dest_addr;
    req.dest_ep                 = ZDO_EP;
    req.prof_id                 = 0;
    req.clust_id                = 0;
    req.src_ep                  = ZDO_EP;
    req.asdu_len                = len;
    req.buf                     = buf;
    req.tx_opt                  = (1<<APS_TX_REQ_ACK_TX_OFF);
    req.radius                  = 0x1;

    // send it into the aps tx queue
    aps_data_req(&req);
}

/**************************************************************************
Function Name: zdo_cmd_unicast_data_req_tree

Description:
Send data via the app sublayer data request service. No route discovery.
**************************************************************************/
void zdo_cmd_unicast_data_req_tree(U8 argc, char **argv)
{
    aps_data_req_t req;
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

    // generate the aps data request
    req.dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req.dest_addr.mode          = SHORT_ADDR;
    req.dest_addr.short_addr    = dest_addr;
    req.dest_ep                 = ZDO_EP;
    req.prof_id                 = 0;
    req.clust_id                = 0;
    req.src_ep                  = ZDO_EP;
    req.asdu_len                = len;
    req.buf                     = buf;
    req.tx_opt                  = (1<<APS_TX_RTE_DISC_DISABLE);
    req.radius                  = 0x6;

    // send it into the aps tx queue
    aps_data_req(&req);
}

/**************************************************************************
Function Name: zdo_cmd_unicast_data_req

Description:
Send data via the app sublayer data request service.
**************************************************************************/
void zdo_cmd_unicast_data_req(U8 argc, char **argv)
{
    aps_data_req_t req;
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

    // generate the aps data request
    req.dest_addr_mode          = APS_DEST_ADDR_16_EP_PRESENT;
    req.dest_addr.mode          = SHORT_ADDR;
    req.dest_addr.short_addr    = dest_addr;
    req.dest_ep                 = ZDO_EP;
    req.prof_id                 = 0;
    req.clust_id                = 0;
    req.src_ep                  = ZDO_EP;
    req.asdu_len                = len;
    req.buf                     = buf;
    req.tx_opt                  = 0x0;
    req.radius                  = 0x6;

    // send it into the aps tx queue
    aps_data_req(&req);
}

/**************************************************************************
Function Name: zdo_cmd_parse

Description:
**************************************************************************/
void zdo_cmd_parse(char *data)
{
    U8 argc, i = 0;
    char *argv[30];

    argv[i] = strtok(data, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));

    argc = i;
    for (i=0; zdo_cmd_tbl[i].cmd != NULL; i++)
    {
        if (!strcmp(argv[0], zdo_cmd_tbl[i].cmd))
        {
            zdo_cmd_tbl[i].func(argc, argv);
            return;
        }
    }
    DBG_PRINT("ZDO_CMD: Command not recognized.\n");
}

#endif //TEST_SIM
