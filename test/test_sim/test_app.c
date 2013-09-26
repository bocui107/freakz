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
    \file test_app.c
    \ingroup


*/
/**************************************************************************/
#include "freakz.h"
#include "test_app.h"
#include "test_zdo.h"
#include "test_zcl.h"
#include "test_data.h"
#include "zcl_basic.h"
#include "zcl_on_off.h"

#if (TEST_SIM == 1)
FILE *fp;
FILE *fout;
#endif

// Function Prototypes
static void test_app_hello(U8 argc, char **argv);
static void test_app_zdo_start(U8 argc, char **argv);
static void test_app_dump_nib(U8 argc, char **argv);
static void test_app_dump_pib(U8 argc, char **argv);
static void test_app_dump_nbor_tbl(U8 argc, char **argv);
static void test_app_dump_rte_tbl(U8 argc, char **argv);
static void test_app_dump_bnd_tbl(U8 argc, char **argv);
static void test_app_dump_grp_tbl(U8 argc, char **argv);
static void test_app_add_bnd(U8 argc, char **argv);
static void test_app_rem_bnd(U8 argc, char **argv);
static void test_app_add_grp(U8 argc, char **argv);
static void test_app_rem_grp(U8 argc, char **argv);
static void test_app_dump_free_bufs(U8 argc, char **argv);

// Main command table for this node
static test_app_cmd_t test_app_cmd_tbl[] =
{
    {"hello",   test_app_hello},
    {"zs",      test_app_zdo_start},
    {"nib",     test_app_dump_nib},
    {"pib",     test_app_dump_pib},
    {"buf",     test_app_dump_free_bufs},
    {"dnt",     test_app_dump_nbor_tbl},
    {"drt",     test_app_dump_rte_tbl},
    {"dbt",     test_app_dump_bnd_tbl},
    {"dgt",     test_app_dump_grp_tbl},
    {"abe",     test_app_add_bnd},
    {"rbe",     test_app_rem_bnd},
    {"ag",      test_app_add_grp},
    {"rg",      test_app_rem_grp},
    {"ra",      test_zcl_read_attrib},
    {"wa",      test_zcl_write_attrib},
    {"da",      test_zcl_disc_attrib},
    {"cr",      test_zcl_config_rpt},
    {"id",      test_zcl_id_req},
    {"onoff",   test_zcl_on_off_req},
    {"movt",    test_zcl_level_move_to_req},
    {"move",    test_zcl_level_move_req},
    {"step",    test_zcl_level_step_req},
    {"stop",    test_zcl_level_stop_req},
    {"nar",     test_zdo_nwk_addr_req},
    {"iar",     test_zdo_ieee_addr_req},
    {"edb",     test_zdo_end_dev_bind},
    {"znl",     test_zdo_nwk_lqi_req},
    {"lv",      test_zdo_nwk_leave_req},
    {"ndr",     test_zdo_nwk_disc_req},
    {"nlr",     test_zdo_server_nwk_leave_req},
    {"npj",     test_zdo_permit_join_req},
    {"zbr",     test_zdo_bind_req},
    {"zur",     test_zdo_unbind_req},
    {"nur",     test_zdo_nwk_update_req},
    {"udr",     test_data_unicast_data_req},
    {"rudr",    test_data_unicast_rel_data_req},
    {"idr",     test_data_ind_data_req},
    {"gdr",     test_data_grp_data_req},
    {NULL,      NULL}
};

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_app_init()
{
    test_zdo_init();
    test_zcl_init();
    test_data_init();
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

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_hello(U8 argc, char **argv)
{
    DBG_PRINT("Hello World!\n");
}

/**************************************************************************
Function Name: test_app_zdo_start

Description:
**************************************************************************/
static void test_app_zdo_start(U8 argc, char **argv)
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
static void test_app_dump_nib(U8 argc, char **argv)
{
    debug_dump_nib();
}

/**************************************************************************
Function Name: test_app_dump_pib

Description:
**************************************************************************/
static void test_app_dump_pib(U8 argc, char **argv)
{
    debug_dump_pib();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_dump_free_bufs(U8 argc, char **argv)
{
    DBG_PRINT("Used buffers: %d.\n", buf_get_cnt());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_dump_nbor_tbl(U8 argc, char **argv)
{
    debug_dump_nbor_tbl();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_dump_rte_tbl(U8 argc, char **argv)
{
    debug_dump_rte_tbl();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_dump_bnd_tbl(U8 argc, char **argv)
{
    debug_dump_bnd_tbl();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_dump_grp_tbl(U8 argc, char **argv)
{
    debug_dump_grp_tbl();
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_add_bnd(U8 argc, char **argv)
{
    U8 src_ep, dest_ep;
    U16 dest_addr, clust_id;
    address_t addr;

    if (argc != 5)
    {
        DBG_PRINT("ERROR: Arguments are incorrect!\n");
    }

    src_ep      = strtol(argv[1], NULL, 16);
    clust_id    = strtol(argv[2], NULL, 16);
    dest_addr   = strtol(argv[3], NULL, 16);
    dest_ep     = strtol(argv[4], NULL, 16);

    addr.mode       = SHORT_ADDR;
    addr.short_addr = dest_addr;

    aps_bind_req(src_ep, &addr, dest_ep, clust_id);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_rem_bnd(U8 argc, char **argv)
{
    U8 src_ep, dest_ep;
    U16 dest_addr, clust_id;
    address_t addr;

    if (argc != 5)
    {
        DBG_PRINT("ERROR: Arguments are incorrect!\n");
    }

    src_ep      = strtol(argv[1], NULL, 16);
    clust_id    = strtol(argv[2], NULL, 16);
    dest_addr   = strtol(argv[3], NULL, 16);
    dest_ep     = strtol(argv[4], NULL, 16);

    addr.mode       = SHORT_ADDR;
    addr.short_addr = dest_addr;

    aps_unbind_req(src_ep, &addr, dest_ep, clust_id);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_add_grp(U8 argc, char **argv)
{
    U8 ep;
    U16 id;

    if (argc != 3)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
    }

    id = strtol(argv[1], NULL, 16);
    ep = strtol(argv[2], NULL, 16);
    aps_grp_add(id, ep);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_app_rem_grp(U8 argc, char **argv)
{
    U8 ep;
    U16 id;

    if (argc != 3)
    {
        DBG_PRINT_SIMONLY("ERROR: Arguments are incorrect!\n");
    }

    id = strtol(argv[1], NULL, 16);
    ep = strtol(argv[2], NULL, 16);
    aps_grp_rem(id, ep);
}




