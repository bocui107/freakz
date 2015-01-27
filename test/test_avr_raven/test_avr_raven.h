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
    \file test_raven.h
    \ingroup
*/
/*******************************************************************/
#ifndef TEST_RAVEN_H
#define TEST_RAVEN_H

#include "types.h"

#define TEST_EP                  25
#define TEST_PROFILE_ID          0xC000
#define TEST_OUT_CLUST           0
#define TEST_IN_CLUST            1
#define TEST_SET_LONG_XFER_CLUST 2
#define MAX_MSG_SIZE             30
#define TEST_AVR_ARGV_PTRS_MAX   10
#define TEST_AVR_ARGV_MAX        10

typedef struct _assoc_dev_t
{
    U8 index;
    U16 nwk_addr;
} assoc_dev_t;

void test_raven_init();
void test_raven_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 dest_clust);
void test_raven_conf_handler(U8 status, U8 in_handle);
void test_raven_zdo_start(bool coord);
void test_raven_nwk_form_conf(U8 status);
void test_raven_nwk_join_conf(U8 status, U16 nwk_addr, U16 parent_addr);
void test_raven_tx_data(U8 len, U8 *data);
void test_raven_nwk_join_ind(U16 nwk_addr);
void test_raven_set_addr();

#endif
