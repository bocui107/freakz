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
    \file zcl_id.h
    \ingroup zcl
    \brief ZCL Identify cluster header file
 
*/
/**************************************************************************/
#ifndef ZCL_ID_H
#define ZCL_ID_H

#include "types.h"

/*! attribute list size - this will be one larger than the actual list size
    to hold the end marker
*/
#define ZCL_ID_ATTRIB_LIST_SZ   2
#define ZCL_ID_TMR(m)   ((zcl_id_tmr_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as an zcl identify timer value

/**************************************************************************/
/*!
    Constants for this cluster
*/
/**************************************************************************/
typedef enum _zcl_id_enums_t
{
    ZCL_ID_ATTRIB               = 0x00,
    ZCL_ID_CMD_ID               = 0x00,
    ZCL_ID_CMD_ID_QUERY         = 0x01,
    ZCL_ID_CMD_ID_QUERY_RESP    = 0x00,

    ZCL_ID_ACTION_ID_ON         = 0x00,
    ZCL_ID_ACTION_ID_OFF        = 0x01
} zcl_id_enums_t;

/**************************************************************************/
/*!
    This is the data for the attribute.
*/
/**************************************************************************/
typedef struct _zcl_id_data_t
{
    U16 id_time;
} zcl_id_data_t;

/**************************************************************************/
/*!
    This struct holds the actual attribute list as well as the attribute
    data.
*/
/**************************************************************************/
typedef struct _zcl_id_attrib_list_t
{
    zcl_attrib_t    list[ZCL_ID_ATTRIB_LIST_SZ];
    zcl_id_data_t   data;
} zcl_id_attrib_list_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_id_tmr_t
{
    U8 ep;
    U16 *time;      ///< Using a pointer here because we want it to point to the attrib data
    void (*action_handler)(U8 action, void *data);
} zcl_id_tmr_t;

// prototypes
void zcl_id_init(zcl_id_attrib_list_t *attrib_list);
mem_ptr_t *zcl_id_get_head();
U8 zcl_id_gen_req(U8 *data, zcl_hdr_t *hdr, U16 timeout);
void zcl_id_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr);
void zcl_id_tmr_periodic();

#endif
