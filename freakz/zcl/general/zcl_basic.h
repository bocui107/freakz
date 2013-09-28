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
    \file zcl_basic.h
    \ingroup zcl
    \brief ZCL basic cluster header file

        This is the header file for the ZCL basic cluster.
*/
/******************************************************************/
#ifndef ZCL_BASIC_H
#define ZCL_BASIC_H

#include "freakz.h"

// test data ... change this later
#define ZCL_BASIC_ATTRIB_LIST_SZ    13              ///< Basic attribute list size
#define ZCL_VER_NUM                 0xA3            ///< Version number
#define ZCL_APP_VER_NUM             0x55            ///< Application version number
#define ZCL_STACK_VER_NUM           0x12            ///< Stack version number
#define ZCL_HW_VER_NUM              0x34            ///< Hardware version number
#define ZCL_MANUF_ID                "FreakLabs"     ///< Manufacturer Name
#define ZCL_MODEL_ID                "FreakZ"        ///< Model Name
#define ZCL_DATE_CODE               "2009-02-10"    ///< Date code
#define ZCL_LOC_DESC                "Laboratory"    ///< Location description...wtf is this used for?

/**************************************************************************/
/*!
    These are the enumerated values specific to the ZCL Basic cluster.
*/
/**************************************************************************/
typedef enum _zcl_basic_enum_t
{
    // attrib id
    ZCL_BASIC_VER           = 0x0000,       ///< Version number attrib ID
    ZCL_BASIC_APP_VER       = 0x0001,       ///< App version attrib ID
    ZCL_BASIC_STACK_VER     = 0x0002,       ///< Stack version attrib ID
    ZCL_BASIC_HW_VER        = 0x0003,       ///< Hardware version attrib ID
    ZCL_BASIC_MANUF_NAME    = 0x0004,       ///< Manufacturer name attrib ID
    ZCL_BASIC_MODEL_ID      = 0x0005,       ///< Model name attrib ID
    ZCL_BASIC_DATE_CODE     = 0x0006,       ///< Date code attrib ID
    ZCL_BASIC_PWR_SRC       = 0x0007,       ///< Power source attrib ID
                                                  
    ZCL_BASIC_LOC_DESC      = 0x0010,       ///< Location description attrib ID
    ZCL_BASIC_PHYS_ENV      = 0x0011,       ///< Physical environment attrib ID
    ZCL_BASIC_DEV_ENB       = 0x0012,       ///< Device enable attrib ID
    ZCL_BASIC_ALARM_MSK     = 0x0013,       ///< Alarm mask attrib ID
                                                 
    // cmd id                                    
    ZCL_BASIC_CMD_RESET     = 0x00,         ///< Command reset command ID

    // pwr src enum
    ZCL_BASIC_PWR_ENUM_UNKNOWN                      = 0x00,     ///< Unknown power source
    ZCL_BASIC_PWR_ENUM_MAINS_SINGLE                 = 0x01,     ///< Mains powered single phase
    ZCL_BASIC_PWR_ENUM_MAINS_TRIPLE                 = 0x02,     ///< Mains powered three phase
    ZCL_BASIC_PWR_ENUM_BATTERY                      = 0x03,     ///< Battery powered
    ZCL_BASIC_PWR_ENUM_DC_SRC                       = 0x04,     ///< DC powered
    ZCL_BASIC_PWR_ENUM_EMERGENCY_MAINS_CONST_PWR    = 0x05,     ///< Emergency mains constant powered
    ZCL_BASIC_PWR_ENUM_EMERGENCY_MAINS_TRANS_SW     = 0x06,     ///< Emergency mains and transfer switch

    // phys env
    ZCL_BASIC_PHYS_ENV_UNSPECD  = 0x00,         ///< Physical environment unspecified
    ZCL_BASIC_PHYS_ENV_UNKNOWN  = 0xff          ///< Physical environment unknown
} zcl_basic_enum_t;       

/**************************************************************************/
/*!
    This is the structure that holds all a particular instance of the ZCL basic
    cluster.
*/
/**************************************************************************/
typedef struct _zcl_basic_data_t
{
    U8 zcl_ver;                                                 ///< ZCL version attribute
    U8 app_ver;                         ///< App version attribute
    U8 stack_ver;                       ///< Stack version attribute 
    U8 hw_ver;                          ///< Hardware version attribute 
    U8 manuf_id[ZCL_MAX_STR_SZ];        ///< Manufacturer name attribute 
    U8 model_id[ZCL_MAX_STR_SZ];        ///< Model name attribute 
    U8 date_code[ZCL_MAX_STR_SZ/2];     ///< Date code attribute - defined to be a max of 16 chars
    U8 pwr_src;                         ///< power source attribute 
                                             
    U8 loc_desc[ZCL_MAX_STR_SZ/2];      ///< Location description attribute 
    U8 phys_env;                        ///< Physical environment attribute 
    bool dev_enb;                       ///< Device enabled attribute 
    U8 alarm_msk;                       ///< Alarm mask attribute 
} zcl_basic_data_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_basic_attrib_list_t
{
    zcl_attrib_t list[ZCL_BASIC_ATTRIB_LIST_SZ];
    zcl_basic_data_t data;
} zcl_basic_attrib_list_t;

void zcl_basic_init(zcl_basic_attrib_list_t *attrib_list);
void zcl_basic_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr);

#endif
