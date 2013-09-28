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
    \file zcl_groups.h
    \ingroup zcl


*/
/**************************************************************************/
#ifndef ZCL_GRP_H
#define ZCL_GRP_H

#include "types.h"

#define ZCL_GRP_ATTRIB_LIST_SZ  2
#define ZCL_GRP_MAX_LIST_SZ     10
#define ZCL_GRP_NAME_SUPPORT    0x00

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef enum _zcl_grp_enums_t
{
    ZCL_GRP_ATTRIB_NAME_SUPPORT  = 0x00,

    ZCL_GRP_CMD_ADD_GRP         = 0x00,
    ZCL_GRP_CMD_VIEW_GRP        = 0x01,
    ZCL_GRP_CMD_GET_MEMB        = 0x02,
    ZCL_GRP_CMD_REM_GRP         = 0x03,
    ZCL_GRP_CMD_REM_ALL         = 0x04,
    ZCL_GRP_CMD_ADD_IF_ID       = 0x05,

    ZCL_GRP_CMD_ADD_GRP_RESP    = 0x00,
    ZCL_GRP_CMD_VIEW_GRP_RESP   = 0x01,
    ZCL_GRP_CMD_GET_MEMB_RESP   = 0x02,
    ZCL_GRP_CMD_REM_GRP_RESP    = 0x03,

    ZCL_GRP_CAPACITY_AVAIL      = 0xfe
} zcl_grp_enums_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_grp_data_t
{
    U8 name_supp;
} zcl_grp_data_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_grp_attrib_list_t
{
    zcl_attrib_t list[ZCL_GRP_ATTRIB_LIST_SZ];
    zcl_grp_data_t data;
} zcl_grp_attrib_list_t;

#endif
