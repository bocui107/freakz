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
    \file zcl_on_off.h
    \ingroup zcl
    \brief ZCL on/off cluster header file

        Contains the definitions for the ZCL On/Off cluster including the
        attribute IDs, command IDs, and defines the structure that will
        hold the attributes.
*/
/******************************************************************/
#ifndef ZCL_ON_OFF_H
#define ZCL_ON_OFF_H

#include "types.h"

#define ZCL_ON_OFF_ATTRIB_LIST_SZ 2

/**************************************************************************/
/*!
        On/Off enumerated values.
*/
/**************************************************************************/
typedef enum _zcl_on_off_enum_t
{
        ZCL_ON_OFF_ATTRIB       = 0x0000,       ///< On/Off attribute ID

        ZCL_ON_OFF_CMD_OFF      = 0x00,         ///< Command ID to turn attrib status to off
        ZCL_ON_OFF_CMD_ON       = 0x01,         ///< Command ID to turn attrib status to on
        ZCL_ON_OFF_CMD_TOGGLE   = 0x02          ///< Command ID to toggle attrib status
} zcl_on_off_enum_t;

/**************************************************************************/
/*!
        This struct holds the actual attribute value.
*/
/**************************************************************************/
typedef struct _zcl_on_off_data_t
{
    bool on_off;                                                ///< On/off attribute status
} zcl_on_off_data_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_on_off_attrib_list_t
{
    zcl_attrib_t list[ZCL_ON_OFF_ATTRIB_LIST_SZ];
    zcl_on_off_data_t data;
} zcl_on_off_attrib_list_t;

void zcl_on_off_init(zcl_on_off_attrib_list_t *attrib_list);
void zcl_on_off_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr);
U8 zcl_on_off_gen_req(U8 *data, zcl_hdr_t *hdr);

#endif
