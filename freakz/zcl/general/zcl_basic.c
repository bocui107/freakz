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
    \file zcl_basic.c 
    \ingroup zcl
    \brief ZCL Basic cluster 

        This file implements the ZCL basic clusters.
*/
/******************************************************************/
#include "freakz.h"
#include "zcl_basic.h"

/**************************************************************************/
/*!
    Set the ZCL Basic Cluster attributes to their default values. These can
    be changed here or when you init the endpoint. The basic clusters are
        mostly read-only and just provide information about the device. The only
        action that is specified for them is to reset them to their default values.
*/
/**************************************************************************/
void zcl_basic_init(zcl_basic_attrib_list_t *attrib_list)
{
    char *manuf_id  = ZCL_MANUF_ID,
         *model_id  = ZCL_MODEL_ID,
         *date_code = ZCL_DATE_CODE,
         *loc_desc  = ZCL_LOC_DESC;

    // init the data values first
    memset(attrib_list, 0, sizeof(zcl_basic_attrib_list_t));
    attrib_list->data.zcl_ver     = ZCL_VER_NUM;
    attrib_list->data.app_ver     = ZCL_APP_VER_NUM;
    attrib_list->data.stack_ver   = ZCL_STACK_VER_NUM;
    attrib_list->data.hw_ver      = ZCL_HW_VER_NUM; 
    attrib_list->data.pwr_src     = ZCL_BASIC_PWR_ENUM_MAINS_SINGLE;
    attrib_list->data.phys_env    = ZCL_BASIC_PHYS_ENV_UNSPECD;
    attrib_list->data.dev_enb     = true;
    attrib_list->data.alarm_msk   = 0;

    // init the string data. these need special processing.
    zcl_set_string_attrib(attrib_list->data.manuf_id, (U8 *)manuf_id, ZCL_MAX_STR_SZ);
    zcl_set_string_attrib(attrib_list->data.model_id, (U8 *)model_id, ZCL_MAX_STR_SZ);
    zcl_set_string_attrib(attrib_list->data.date_code, (U8 *)date_code, ZCL_MAX_STR_SZ/2);
    zcl_set_string_attrib(attrib_list->data.loc_desc, (U8 *)loc_desc, ZCL_MAX_STR_SZ/2);

    // init the attribute fields
    zcl_set_attrib(&attrib_list->list[0],     ZCL_BASIC_VER,          ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.zcl_ver);
    zcl_set_attrib(&attrib_list->list[1],     ZCL_BASIC_APP_VER,      ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.app_ver);
    zcl_set_attrib(&attrib_list->list[2],     ZCL_BASIC_STACK_VER,    ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.stack_ver);
    zcl_set_attrib(&attrib_list->list[3],     ZCL_BASIC_HW_VER,       ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.hw_ver);
    zcl_set_attrib(&attrib_list->list[4],     ZCL_BASIC_MANUF_NAME,   ZCL_TYPE_CHAR_STRING,    ZCL_ACCESS_READ_ONLY,    attrib_list->data.manuf_id);
    zcl_set_attrib(&attrib_list->list[5],     ZCL_BASIC_MODEL_ID,     ZCL_TYPE_CHAR_STRING,    ZCL_ACCESS_READ_ONLY,    attrib_list->data.model_id);
    zcl_set_attrib(&attrib_list->list[6],     ZCL_BASIC_DATE_CODE,    ZCL_TYPE_CHAR_STRING,    ZCL_ACCESS_READ_ONLY,    attrib_list->data.date_code);
    zcl_set_attrib(&attrib_list->list[7],     ZCL_BASIC_PWR_SRC,      ZCL_TYPE_U8,             ZCL_ACCESS_READ_ONLY,   &attrib_list->data.pwr_src);
    zcl_set_attrib(&attrib_list->list[8],     ZCL_BASIC_LOC_DESC,     ZCL_TYPE_CHAR_STRING,    ZCL_ACCESS_READ_WRITE,   attrib_list->data.loc_desc);
    zcl_set_attrib(&attrib_list->list[9],     ZCL_BASIC_PHYS_ENV,     ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.phys_env);
    zcl_set_attrib(&attrib_list->list[10],    ZCL_BASIC_DEV_ENB,      ZCL_TYPE_BOOL,           ZCL_ACCESS_READ_WRITE,  &attrib_list->data.dev_enb);
    zcl_set_attrib(&attrib_list->list[11],    ZCL_BASIC_ALARM_MSK,    ZCL_TYPE_U8,             ZCL_ACCESS_READ_WRITE,  &attrib_list->data.alarm_msk);
    zcl_set_attrib(&attrib_list->list[12],    ZCL_END_MARKER,         0,                       0,                      NULL);
}

/**************************************************************************/
/*!
        This is the rx handler for the ZCL basic cluster. 

        TODO: Implement ZCL reset command if needed.
*/
/**************************************************************************/
void zcl_basic_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
        if (hdr->cmd == ZCL_BASIC_CMD_RESET)
        {
                // do nothing...not sure how I want to handle a reset to factory defaults yet...
        }
    return;
}

