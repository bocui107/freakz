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
    \file test_zcl.h
    \ingroup test_sim

    This is the associated header file for test_zcl.c.
*/
/**************************************************************************/
#ifndef TEST_ZCL_H
#define TEST_ZCL_H

#define TEST_ZCL_EP             1
#define TEST_ZCL_STACK_PROF_HA  0x0104
#define TEST_ZCL_DEV_ID_ON_OFF  0
#define TEST_ZCL_DEV_VER        0
#define TEST_ZCL_MAX_BUF_SZ     ZCL_MAX_PAYLOAD_SIZE
#define TEST_ZCL_MAX_ATTRIBS    32
#define TEST_ZCL_CLUST_LIST_SZ  4
/**************************************************************************/
/*!
    Function prototypes
*/
/**************************************************************************/
void test_zcl_init();
void test_zcl_read_attrib(U8 argc, char **argv);
void test_zcl_write_attrib(U8 argc, char **argv);
void test_zcl_disc_attrib(U8 argc, char **argv);
void test_zcl_nwk_leave_req(U8 argc, char **argv);
void test_zcl_config_rpt(U8 argc, char **argv);
void test_zcl_id_req(U8 argc, char **argv);
void test_zcl_on_off_req(U8 argc, char **argv);
void test_zcl_on_off_action_handler(U8 action, void *data);
void test_zcl_id_action_handler(U8 action, void *data);
void test_zcl_level_action_handler(U8 action, void *data);
void test_zcl_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id);
void test_zcl_conf_handler(U8 status, U8 in_handle);

void test_zcl_level_move_to_req(U8 argc, char **argv);
void test_zcl_level_move_req(U8 argc, char **argv);
void test_zcl_level_step_req(U8 argc, char **argv);
void test_zcl_level_stop_req(U8 argc, char **argv);
#endif
