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
    \file zcl_senes.h
    \ingroup zcl


*/
/**************************************************************************/
#ifndef ZCL_SCENES_H
#define ZCL_SCENES_H

#include "types.h"

#define ZCL_SCENES_ATTRIB_LIST_SZ       7
#define ZCL_SCENES_NAME_MAX_LEN         16
#define ZCL_SCENES_EXT_MAX_LEN          13
#define ZCL_SCENES_NAME_SUPPORT         0x80
#define ZCL_SCENES_GET_MEMB_CAPACITY    0xfe
#define ZCL_SCENES_ENTRY(m) ((zcl_scenes_entry_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as an zcl scene entry

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef enum _zcl_scenes_enum_t
{
    ZCL_SCENES_ATTRIB_SCENE_COUNT       = 0x0000,
    ZCL_SCENES_ATTRIB_CURR_SCENE        = 0x0001,
    ZCL_SCENES_ATTRIB_CURR_GROUP        = 0x0002,
    ZCL_SCENES_ATTRIB_SCENE_VALID       = 0x0003,
    ZCL_SCENES_ATTRIB_NAME_SUPP         = 0x0004,
    ZCL_SCENES_ATTRIB_LAST_CONFIG_BY    = 0x0005,

    ZCL_SCENES_CMD_ADD                  = 0x00,
    ZCL_SCENES_CMD_VEIW                 = 0x01,
    ZCL_SCENES_CMD_REM                  = 0x02,
    ZCL_SCENES_CMD_REM_ALL              = 0x03,
    ZCL_SCENES_CMD_STORE                = 0x04,
    ZCL_SCENES_CMD_RECALL               = 0x05,
    ZCL_SCENES_CMD_GET_MEMB             = 0x06,

    ZCL_SCENES_CMD_ADD_RESP             = 0x00,
    ZCL_SCENES_CMD_VIEW_RESP            = 0x01,
    ZCL_SCENES_CMD_REM_RESP             = 0x02,
    ZCL_SCENES_CMD_REM_ALL_RESP         = 0x03,
    ZCL_SCENES_CMD_STORE_RESP           = 0x04,
    ZCL_SCENES_CMD_GET_MEMB_RESP        = 0x06,

    ZCL_SCENES_ACTION_STORE             = 0x01,
    ZCL_SCENES_ACTION_RECALL            = 0x02
} zcl_scenes_enum_T;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_scenes_data_t
{
    U8      scene_cnt;
    U8      curr_scene;
    U16     curr_grp;
    bool    scene_valid;
    U8      name_supp;
    U64     last_cfg_by;
} zcl_scenes_data_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_scenes_attrib_list_t
{
    zcl_attrib_t list[ZCL_SCENES_ATTRIB_LIST_SZ];
    zcl_scenes_data_t data;
} zcl_scenes_attrib_list_t;

/**************************************************************************/
/*!
    An entry in the scenes table
*/
/**************************************************************************/
typedef struct _zcl_scenes_entry_t
{
    U8  ep
    U16 grp_id;                             ///< Scene group ID
    U8  id;                                 ///< Scene ID
    U8  name[ZCL_SCENE_NAME_MAX_LEN];       ///< Scene name
    U16 trans_time;                         ///< Transition time for this scene
    U8  ext_len;                            ///< Extension fields length
    U8  ext_field[ZCL_SCENES_EXT_MAX_LEN];  ///< Extension fields
} zcl_scenes_entry_t;

#endif
