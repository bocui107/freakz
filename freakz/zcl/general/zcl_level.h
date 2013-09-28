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
    \file zcl_level.h
    \ingroup zcl


*/
/**************************************************************************/
#ifndef ZCL_LEVEL_H
#define ZCL_LEVEL_H

#include "types.h"

#define ZCL_LEVEL_ATTRIB_LIST_SZ 5
#define ZCL_LEVEL_TMR(m)   ((zcl_level_tmr_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as an zcl level timer value

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef enum zcl_level_enums_t
{
    ZCL_LEVEL_ATTRIB_CURR_LEVEL             = 0x0000,
    ZCL_LEVEL_ATTRIB_ON_OFF_TRANS_TIME      = 0x0010,
    ZCL_LEVEL_ATTRIB_ON_LEVEL               = 0x0011,

    ZCL_LEVEL_CMD_MOVE_TO_LEVEL             = 0x00,
    ZCL_LEVEL_CMD_MOVE                      = 0x01,
    ZCL_LEVEL_CMD_STEP                      = 0x02,
    ZCL_LEVEL_CMD_STOP                      = 0x03,
    ZCL_LEVEL_CMD_MOVE_TO_LEVEL_WITH_ON_OFF = 0x04,
    ZCL_LEVEL_CMD_MOVE_WITH_ON_OFF          = 0x05,
    ZCL_LEVEL_CMD_STEP_WITH_ON_OFF          = 0x06,
    ZCL_LEVEL_CMD_STOP_WITH_ON_OFF          = 0x07,

    ZCL_LEVEL_MODE_UP                       = 0x00,
    ZCL_LEVEL_MODE_DOWN                     = 0x01,

    ZCL_LEVEL_ACTION_REFRESH                = 0x00,
    ZCL_LEVEL_ACTION_MAX_LEVEL              = 0x01,
    ZCL_LEVEL_ACTION_MIN_LEVEL              = 0x02
} zcl_level_enums_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct zcl_level_data_t
{
    U8  curr_level;
    U16 on_off_trans_time;
    U8  on_level;
} zcl_level_data_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_level_attrib_list_t
{
    zcl_attrib_t        list[ZCL_LEVEL_ATTRIB_LIST_SZ];
    zcl_level_data_t    data;
} zcl_level_attrib_list_t;

/**************************************************************************/
/*!
*/
/**************************************************************************/
typedef struct _zcl_level_tmr_t
{
    U8 *level;                          ///< pointer to level attribute
    U8 step;                            ///< step size
    U16 acc;                            ///< accumulator used to track roundoff error
    U8 rem;                            ///< remainder term used to handle roundoff error
    U16 rem_time;                       ///< time remaining for this operation
    bool dir;                           ///< direction of level change
    bool with_on_off;                   ///< integration with on/off attribute
    void (*action_handler)(U8, void *); ///< action handler for this operation
} zcl_level_tmr_t;

////////////////////////////////////////////////////////////////////////////
// client side requests
////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_level_move_to_level_req_t
{
    U8 level;
    U16 trans_time;
} zcl_level_move_to_level_req_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_level_move_req_t
{
    U8 dir;
    U16 rate;
} zcl_level_move_req_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_level_step_req_t
{
    U8 dir;
    U8 step_size;
    U16 trans_time;
} zcl_level_step_req_t;

/**************************************************************************/
/*!

*/
/**************************************************************************/
typedef struct _zcl_level_req_t
{
    zcl_level_move_to_level_req_t   move_to_level;
    zcl_level_move_req_t            move;
    zcl_level_step_req_t            step;
} zcl_level_req_t;

// prototypes
void zcl_level_init(zcl_level_attrib_list_t *attrib_list);
U8 zcl_level_gen_req(U8 *data, zcl_hdr_t *hdr, zcl_level_req_t *req);
void zcl_level_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr);
void zcl_level_tmr_add(U8 *level, U16 rem_time, U8 step, U8 rem, bool dir, bool on_off, void (*action_handler)(U8, void *));
void zcl_level_tmr_periodic(void *ptr);
#endif
