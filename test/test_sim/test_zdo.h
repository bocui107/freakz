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
    \file test_zdo.h
    \ingroup test_sim


*/
/**************************************************************************/
#ifndef TEST_ZDO_H
#define TEST_ZDO_H

// Function prototypes
void test_zdo_init();
void test_zdo_nwk_join_ind(U16 nwk_addr);
void test_zdo_nwk_form_conf(U8 status);
void test_zdo_nwk_join_conf(U8 status, U16 nwk_addr, U16 parent_addr);
void test_zdo_nwk_addr_req(U8 argc, char **argv);
void test_zdo_ieee_addr_req(U8 argc, char **argv);
void test_zdo_end_dev_bind(U8 argc, char **argv);
void test_zdo_nwk_lqi_req(U8 argc, char **argv);
void test_zdo_nwk_disc_req(U8 argc, char **argv);
void test_zdo_nwk_leave_req(U8 argc, char **argv);
void test_zdo_server_nwk_leave_req(U8 argc, char **argv);
void test_zdo_permit_join_req(U8 argc, char **argv);
void test_zdo_nwk_update_req(U8 argc, char **argv);
void test_zdo_bind_req(U8 argc, char **argv);
void test_zdo_unbind_req(U8 argc, char **argv);
#endif
