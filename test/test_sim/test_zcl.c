/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.
    4. This software is subject to the additional restrictions placed on the
       Zigbee Specification's Terms of Use.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file test_zcl.c
    \ingroup test_sim

    This file is meant to test the ZCL functionality of the stack.
*/
/**************************************************************************/
#include "freakz.h"
#include "test_zcl.h"
#include "zcl_basic.h"
#include "zcl_on_off.h"
#include "zcl_id.h"
#include "zcl_level.h"

/**************************************************************************/
/*!
    Simple descriptor for this ep
*/
/**************************************************************************/
static U8 simple_desc[] = {
    TEST_ZCL_EP,                // ep
    0x04,                       // profile id
    0x01,
    TEST_ZCL_DEV_ID_ON_OFF,     // dev id
    TEST_ZCL_DEV_VER,           // dev ver
    2,                          // num in clusters
    ZCL_BASIC_CLUST_ID,         // in clusters
    ZCL_ON_OFF_CLUST_ID,
    1,                          // num out clusters
    ZCL_ON_OFF_CLUST_ID
};

/**************************************************************************/
/*!
    Basic Cluster
*/
/**************************************************************************/
static zcl_basic_attrib_list_t basic_attrib_list;
static zcl_clust_t basic_clust;

/**************************************************************************/
/*!
        Initialize the on/off attribute list for the on/off cluster
*/
/**************************************************************************/
// this struct holds the data that the attributes points to
static zcl_on_off_attrib_list_t on_off_attrib_list;
static zcl_clust_t on_off_clust;

/**************************************************************************/
/*!
    ZCL ID Cluster
*/
/**************************************************************************/
static zcl_id_attrib_list_t id_attrib_list;
static zcl_clust_t id_clust;

/**************************************************************************/
/*!
    ZCL Level Control Cluster
*/
/**************************************************************************/
static zcl_level_attrib_list_t level_attrib_list;
static zcl_clust_t level_clust;

/**************************************************************************/
/*!
        Initialize the cluster list
*/
/**************************************************************************/
/// This is where the cluster list is defined
static zcl_clust_t *test_zcl_clust_list[] =
{
    &basic_clust,
    &on_off_clust,
    &id_clust,
    &level_clust,
    NULL
};

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_init()
{
    // init the clusters with the proper data
    zcl_set_clust(&basic_clust,     TEST_ZCL_EP, ZCL_BASIC_CLUST_ID,    basic_attrib_list.list,     zcl_basic_rx_handler,   NULL);
    zcl_set_clust(&on_off_clust,    TEST_ZCL_EP, ZCL_ON_OFF_CLUST_ID,   on_off_attrib_list.list,    zcl_on_off_rx_handler,  test_zcl_on_off_action_handler);
    zcl_set_clust(&id_clust,        TEST_ZCL_EP, ZCL_IDENTIFY_CLUST_ID, id_attrib_list.list,        zcl_id_rx_handler,      test_zcl_id_action_handler);
    zcl_set_clust(&level_clust,     TEST_ZCL_EP, ZCL_LEVEL_CLUST_ID,    level_attrib_list.list,     zcl_level_rx_handler,   test_zcl_level_action_handler);

    // init the attributes
    zcl_basic_init(&basic_attrib_list);
    zcl_on_off_init(&on_off_attrib_list);
    zcl_id_init(&id_attrib_list);
    zcl_level_init(&level_attrib_list);

    // register the endpoint as a ZCL endpoint
    af_ep_add(TEST_ZCL_EP, simple_desc, sizeof(simple_desc), true, test_zcl_rx_handler, test_zcl_conf_handler);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id)
{
    zcl_clust_t *clust;
    zcl_hdr_t hdr;
    U8 resp_len, resp[ZCL_MAX_PAYLOAD_SIZE];

    resp_len = 0;
    if ((clust = zcl_find_clust(test_zcl_clust_list, clust_id)) == NULL)
    {
        // non-existent cluster
        DBG_PRINT_SIMONLY("TEST_ZCL: Non existent cluster.\n");
        return;
    }

    zcl_parse_hdr(data, len, &hdr);
    debug_dump_zcl_frm(data, &hdr, clust_id);

    if (hdr.frm_ctrl.frm_type == ZCL_FRM_TYPE_GENERAL)
    {
        zcl_cmd_handler(resp, &resp_len, src_addr, src_ep, TEST_ZCL_STACK_PROF_HA, clust, &hdr);
    }
    else
    {
        if (clust->rx_handler)
        {
            clust->rx_handler(resp, &resp_len, src_addr, src_ep, clust, &hdr);
        }
    }

    // if resp_len is zero, that probably means we got a write attrib no response command. in that case, don't
    // return a response.
    if (resp_len)
    {
        af_tx(resp, resp_len, TEST_ZCL_EP, src_addr, src_ep, clust_id, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, 6, af_handle_get());
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_conf_handler(U8 status, U8 handle)
{
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_on_off_action_handler(U8 action, void *data)
{
    switch (action)
    {
    case ZCL_ON_OFF_CMD_OFF: DBG_PRINT("ON OFF ACTION HANDLER: OFF.\n"); break;
    case ZCL_ON_OFF_CMD_ON: DBG_PRINT("ON OFF ACTION HANDLER: ON.\n"); break;
    case ZCL_ON_OFF_CMD_TOGGLE: DBG_PRINT("ON OFF ACTION HANDLER: TOGGLE.\n"); break;
    }
    DBG_PRINT_SIMONLY("ON OFF ACTION HANDLER: Current value of on_off attrib: %02X.\n", on_off_attrib_list.data.on_off);
    return;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_id_action_handler(U8 action, void *data)
{
    switch (action)
    {
    case ZCL_ID_ACTION_ID_ON: DBG_PRINT("ID ACTION HANDLER: IDENTIFY ON. Timeout: %04X.\n", id_attrib_list.data.id_time); break;
    case ZCL_ID_ACTION_ID_OFF: DBG_PRINT("ID ACTION HANDLER: IDENTIFY OFF.\n"); break;
    }
    return;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_level_action_handler(U8 action, void *data)
{
    U8 on_off;
    mem_ptr_t *mem_ptr = data;

    on_off = ZCL_LEVEL_TMR(mem_ptr)->with_on_off;

    switch (action)
    {
    case ZCL_LEVEL_ACTION_MAX_LEVEL:
        DBG_PRINT_SIMONLY("LEVEL ACTION HANDLER: MAX LEVEL REACHED.\n");
        break;

    case ZCL_LEVEL_ACTION_MIN_LEVEL:
        DBG_PRINT_SIMONLY("LEVEL ACTION HANDLER: MIN LEVEL REACHED.\n");

        // if the with_on_off command is issued, then switch the on_off attribute
        if (on_off)
        {
            on_off_attrib_list.data.on_off = 0;
            DBG_PRINT_SIMONLY("LEVEL ACTION HANDLER: ON OFF ATTRIBUTE TURNED OFF.\n");
        }
        break;

    case ZCL_LEVEL_ACTION_REFRESH:
        DBG_PRINT_SIMONLY("LEVEL ACTION HANDLER: CURRENT LEVEL = %02X.\n", level_attrib_list.data.curr_level);

        if (on_off)
        {
            if (on_off_attrib_list.data.on_off == 0)
            {
                on_off_attrib_list.data.on_off = 1;
                DBG_PRINT("LEVEL ACTION HANDLER: ON OFF ATTRIBUTE TURNED ON.\n");
            }
        }
        break;
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_read_attrib(U8 argc, char **argv)
{
    U16 clust_id, dest_addr, attrib_list[TEST_ZCL_MAX_ATTRIBS];
    U8 i, len, data_out[TEST_ZCL_MAX_BUF_SZ];
    zcl_hdr_t hdr;

    if (argc < 4)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr = strtol(argv[1], NULL, 16);
    clust_id = strtol(argv[2], NULL, 10);
    for (i=0; i<argc-3; i++)
    {
        attrib_list[i] = strtol(argv[i+3], NULL, 10);
    }

    memset(&hdr, 0, sizeof(hdr));
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.seq_num                 = zcl_get_seq_num();
    hdr.cmd                     = ZCL_CMD_READ_ATTRIB;
    len = zcl_gen_read_attrib(data_out, &hdr, attrib_list, argc-3);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, clust_id, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!
        You can only write U8 values for now. The format is:
        <cmd> <clust_id> <attrib_id 0> <val 0> <attrib_id 1> <val 1> ...
        ex: wa 0 0 0 1 0 2 1
        cmd = Write Attribute
        clust_id = 0
        attrib 0 = 0
        val 0 = 1
        attrib 1 = 1
        val 1           = 0
*/
/**************************************************************************/
void test_zcl_write_attrib(U8 argc, char **argv)
{
    U16 dest_addr, clust_id;
    U8 i, j, len, data_out[TEST_ZCL_MAX_BUF_SZ], tmp_data[TEST_ZCL_MAX_ATTRIBS];
    zcl_hdr_t hdr;
    zcl_attrib_t attrib_list[TEST_ZCL_MAX_ATTRIBS];

    if (argc < 4)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr = strtol(argv[1], NULL, 16);
    clust_id = strtol(argv[2], NULL, 16);

    j = 0;
    i = 2;
    while (i<argc)
    {
        attrib_list[j].id           = strtol(argv[i++], NULL, 10);
        attrib_list[j].type         = ZCL_TYPE_U8;
        tmp_data[j]                 = strtol(argv[i++], NULL, 10);  // store the data here temporarily
        attrib_list[j].data         = &tmp_data[j];                 // point to the temp stored data
        j++;
    }

    memset(&hdr, 0, sizeof(zcl_hdr_t));
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.seq_num                 = zcl_get_seq_num();
    hdr.cmd                     = ZCL_CMD_WRITE_ATTRIB;
    len = zcl_gen_write_attrib(data_out, &hdr, attrib_list, j);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, clust_id, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_disc_attrib(U8 argc, char **argv)
{
    U16 dest_addr, clust_id, start_id;
    U8 len, max_attribs, data_out[TEST_ZCL_MAX_BUF_SZ];
    zcl_hdr_t hdr;

    if (argc < 5)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 10);
    clust_id    = strtol(argv[2], NULL, 10);
    start_id    = strtol(argv[3], NULL, 10);
    max_attribs = strtol(argv[4], NULL, 10);

    memset(&hdr, 0, sizeof(zcl_hdr_t));
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.seq_num                 = zcl_get_seq_num();
    hdr.cmd                     = ZCL_CMD_DISC_ATTRIB;
    len = zcl_gen_disc_attrib(data_out, &hdr, start_id, max_attribs);
    af_tx(data_out, len, TEST_ZCL_EP, 0, TEST_ZCL_EP, clust_id, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_config_rpt(U8 argc, char **argv)
{
    U8 len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr, clust_id, attrib_id, timeout;
    zcl_hdr_t hdr;
    zcl_clust_t *clust;

    if (argc < 5)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);
    clust_id    = strtol(argv[2], NULL, 16);
    attrib_id   = strtol(argv[3], NULL, 16);
    timeout     = strtol(argv[4], NULL, 16);

    if ((clust = zcl_find_clust(test_zcl_clust_list, clust_id)) == NULL)
    {
        DBG_PRINT_SIMONLY("TEST_ZCL_CONFIG_RPT: Cluster not found.\n");
        return;
    }

    memset(&hdr, 0, sizeof(zcl_hdr_t));
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.seq_num                 = zcl_get_seq_num();
    hdr.cmd                     = ZCL_CMD_CONFIG_REPORT;
    len = zcl_gen_config_rpt(data_out, &hdr, clust, attrib_id, 0, timeout, 0);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, clust_id, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_on_off_req(U8 argc, char **argv)
{
    U8 len, cmd, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr;
    zcl_hdr_t hdr;

    if (argc != 3)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);
    cmd         = strtol(argv[2], NULL, 16);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = true;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = cmd;
    hdr.seq_num                 = zcl_get_seq_num();

    len = zcl_on_off_gen_req(data_out, &hdr);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_ON_OFF_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_id_req(U8 argc, char **argv)
{
    U8 len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr, timeout;
    zcl_hdr_t hdr;

    if (argc != 3)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr = strtol(argv[1], NULL, 16);
    timeout = strtol(argv[2], NULL, 16);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = true;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = ZCL_ID_CMD_ID;
    hdr.seq_num                 = zcl_get_seq_num();

    len = zcl_id_gen_req(data_out, &hdr, timeout);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_IDENTIFY_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_level_move_to_req(U8 argc, char **argv)
{
    U8 level, len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr, trans_time;
    zcl_hdr_t hdr;
    zcl_level_req_t req;

    if (argc != 4)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);
    level       = strtol(argv[2], NULL, 10);
    trans_time  = strtol(argv[3], NULL, 10);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = ZCL_LEVEL_CMD_MOVE_TO_LEVEL;
    hdr.seq_num                 = zcl_get_seq_num();

    req.move_to_level.level         = level;
    req.move_to_level.trans_time    = trans_time;

    len = zcl_level_gen_req(data_out, &hdr, &req);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_LEVEL_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_level_move_req(U8 argc, char **argv)
{
    U8 dir, len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr, rate;
    zcl_hdr_t hdr;
    zcl_level_req_t req;

    if (argc != 4)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);
    dir         = strtol(argv[2], NULL, 10);
    rate        = strtol(argv[3], NULL, 10);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = ZCL_LEVEL_CMD_MOVE;
    hdr.seq_num                 = zcl_get_seq_num();

    req.move.dir    = dir;
    req.move.rate   = rate;

    len = zcl_level_gen_req(data_out, &hdr, &req);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_LEVEL_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_level_step_req(U8 argc, char **argv)
{
    U8 dir, step_size, len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr, trans_time;
    zcl_hdr_t hdr;
    zcl_level_req_t req;

    if (argc != 5)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);
    dir         = strtol(argv[2], NULL, 10);
    step_size   = strtol(argv[3], NULL, 10);
    trans_time  = strtol(argv[4], NULL, 10);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = ZCL_LEVEL_CMD_STEP;
    hdr.seq_num                 = zcl_get_seq_num();

    req.step.dir       = dir;
    req.step.step_size  = step_size;
    req.step.trans_time = trans_time;

    len = zcl_level_gen_req(data_out, &hdr, &req);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_LEVEL_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_zcl_level_stop_req(U8 argc, char **argv)
{
    U8 len, data_out[TEST_ZCL_MAX_BUF_SZ];
    U16 dest_addr;
    zcl_hdr_t hdr;

    if (argc != 2)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
        return;
    }

    dest_addr   = strtol(argv[1], NULL, 16);

    hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_SRVR;
    hdr.frm_ctrl.dis_def_resp   = false;
    hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_CLUST_SPEC;
    hdr.frm_ctrl.manuf_spec     = false;
    hdr.cmd                     = ZCL_LEVEL_CMD_STOP;
    hdr.seq_num                 = zcl_get_seq_num();

    len = zcl_level_gen_req(data_out, &hdr, NULL);
    af_tx(data_out, len, TEST_ZCL_EP, dest_addr, TEST_ZCL_EP, ZCL_LEVEL_CLUST_ID, TEST_ZCL_STACK_PROF_HA, APS_DEST_ADDR_16_EP_PRESENT, 0, ZIGBEE_DEFAULT_RADIUS, af_handle_get());
}
