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
    \file test_avr.h
    \ingroup
*/
/*******************************************************************/
#ifndef TEST_AVR_H
#define TEST_AVR_H

#include "types.h"

#define TEST_ZCL_EP              1
#define TEST_EP                  25
#define TEST_PROFILE_ID          33
#define TEST_OUT_CLUST           0
#define TEST_IN_CLUST            1
#define TEST_SET_LONG_XFER_CLUST 2
#define MAX_MSG_SIZE             30
#define TEST_AVR_ARGV_PTRS_MAX   10
#define TEST_AVR_ARGV_MAX        10

/* this is the command table for the command line interface */
typedef struct
{
    char *cmd;
    void (*func)(U8 argc, char **argv);
} test_avr_cmd_t;

void test_avr_init();
void test_avr_usb_rx_handler();
void test_avr_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 dest_clust);
void test_avr_conf_handler(U8 status, U8 in_handle);
void test_avr_periodic();

// commands
void test_avr_hello();
void test_avr_print_hello(U8 argc, char **argv);
void test_avr_timed_hello(U8 argc, char **argv);
void test_avr_tx_data(U8 argc, char **argv);
void test_avr_zdo_start(U8 argc, char **argv);
void test_avr_get_state(U8 argc, char **argv);

#endif
