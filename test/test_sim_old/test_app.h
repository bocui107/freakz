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

    Title: test_app.h

    Description:
*******************************************************************/
#ifndef TEST_APP_H
#define TEST_APP_H

#define TEST_EP                  25
#define TEST_PROFILE_ID          33
#define TEST_OUT_CLUST           0
#define TEST_IN_CLUST            1
#define TEST_SET_LONG_XFER_CLUST 2

typedef enum
{
    TESTAPP_NORMAL,
    TESTAPP_LONG
} xfer_states_t;

typedef struct
{
    char *cmd;
    void (*func)(U8 argc, char **argv);
} test_app_cmd_t;

void test_app_init();
void test_app_parse(char *data);
void test_app_zdo_start(U8 argc, char **argv);
void test_app_form_req(U8 argc, char **argv);
void test_app_dump_nib(U8 argc, char **argv);
void test_app_dump_pib(U8 argc, char **argv);
void test_app_unicast_data_req(U8 argc, char **argv);
void test_app_unicast_data_req_tree(U8 argc, char **argv);
void test_app_unicast_rel_data_req(U8 argc, char **argv);
void test_app_xfer_long_file(U8 argc, char **argv);
void test_app_conf_handler(U8 status, U8 handle);
void test_app_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 dest_clust);
void test_app_reg_conf_cb(U8 argc, char **argv);
void test_app_get_buf_cnt(U8 argc, char **argv);
#endif


