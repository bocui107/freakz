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
    \file zcl_level.c
    \ingroup zcl


*/
/**************************************************************************/
#include "freakz.h"
#include "zcl_level.h"

// function prototypes
static mem_ptr_t *zcl_level_tmr_alloc();
static void zcl_level_tmr_free(mem_ptr_t *mem_ptr);
static void zcl_level_tmr_clear();

/**************************************************************************/
/*!
    This is the timer list that will be used to keep track of the remaining
    time required to move a dimmer command to the required level.
*/
/**************************************************************************/
LIST(level_tmr_list);

static struct ctimer level_tmr;     // this is the timer for the slow clock

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_level_init(zcl_level_attrib_list_t *attrib_list)
{
    // init the attrib data values first
    memset(&attrib_list->data, 0, sizeof(zcl_level_data_t));
    attrib_list->data.on_level = 0xfe;

    zcl_set_attrib(&attrib_list->list[0], ZCL_LEVEL_ATTRIB_CURR_LEVEL,          ZCL_TYPE_U8,    ZCL_ACCESS_READ_ONLY,   &attrib_list->data.curr_level);
    zcl_set_attrib(&attrib_list->list[2], ZCL_LEVEL_ATTRIB_ON_OFF_TRANS_TIME,   ZCL_TYPE_U16,   ZCL_ACCESS_READ_WRITE,  &attrib_list->data.on_off_trans_time);
    zcl_set_attrib(&attrib_list->list[3], ZCL_LEVEL_ATTRIB_ON_LEVEL,            ZCL_TYPE_U8,    ZCL_ACCESS_READ_WRITE,  &attrib_list->data.on_level);
    zcl_set_attrib(&attrib_list->list[4], ZCL_END_MARKER, 0, 0, NULL);

    list_init(level_tmr_list);
    zcl_level_tmr_periodic(NULL);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_level_rx_handler(U8 *resp, U8 *resp_len, U16 addr, U8 ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    zcl_attrib_t *attrib;
    U8 intv, curr_level, step, level, *resp_ptr, *data_ptr;
    U16 trans_time, rate, rem;
    bool with_on_off;
    bool dir;

    // init the pointers and variables
    data_ptr    = hdr->payload;
    resp_ptr    = resp;
    with_on_off = false;

    // get the current level
    if ((attrib = zcl_find_attrib(clust->attrib_list, ZCL_LEVEL_ATTRIB_CURR_LEVEL)) == NULL)
    {
        if (!hdr->frm_ctrl.dis_def_resp)
        {
            *resp_len = zcl_gen_def_resp(resp, ZCL_STATUS_NOT_FOUND, hdr);
        }
        return;
    }
    curr_level = *(U8 *)attrib->data;
    
    // if bit 2 is set, then that means we need to integrate the command with the on/off cluster.
    // so first we need to detect whether or not this is the case.
    if (hdr->cmd & 0x4)
    {
        with_on_off = true;
    }

    // decode the commands. each command decode contains two command sets. the first set is without
    // the ON_OFF handling. the second set requires using the ON_OFF cluster.
    switch (hdr->cmd)
    {
    case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
    case ZCL_LEVEL_CMD_MOVE_TO_LEVEL_WITH_ON_OFF:
        level = *data_ptr++;
        trans_time = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        // we need to figure out the step size for the transition. first, figure out the direction
        // and the interval we need to traverse
        if (level > curr_level)
        {
            dir = ZCL_LEVEL_MODE_UP;
            intv = level - curr_level;
        }
        else
        {
            dir = ZCL_LEVEL_MODE_DOWN;
            intv = curr_level - level;
        }

        // calculate the step size. this is the value we will step for every 0.1 sec refresh interval
        step = intv / trans_time;

        // calculate the remainder term. this will be used to track how much roundoff error in the
        // accumulator and adjust the step size to get to the target level within the transition time
        rem = ((intv % trans_time) << 8) / trans_time;  

        // add the new timer
        zcl_level_tmr_add((U8 *)attrib->data, trans_time, step, rem, dir, with_on_off, clust->action_handler);
        break;

    case ZCL_LEVEL_CMD_MOVE:
    case ZCL_LEVEL_CMD_MOVE_WITH_ON_OFF:
        dir = *(bool *)data_ptr++;
        rate = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        // calculate the step that we're going to move at. The minimum step size
        // is 1 unit per tenth of a second. 
        step = (rate < 10) ? 1 : rate/10;

        // add the new timer
        zcl_level_tmr_add((U8 *)attrib->data, 0xffff, step, 0, dir, with_on_off, clust->action_handler);
        break;
                             
    case ZCL_LEVEL_CMD_STEP: 
    case ZCL_LEVEL_CMD_STEP_WITH_ON_OFF:
        dir = *(bool *)data_ptr++;
        step = *data_ptr++;
        trans_time = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        // add the new timer
        zcl_level_tmr_add((U8 *)attrib->data, trans_time, step, 0, dir, with_on_off, clust->action_handler);
        break;                     

    case ZCL_LEVEL_CMD_STOP:            
    case ZCL_LEVEL_CMD_STOP_WITH_ON_OFF:
        // clear the tmr table. all actions will be stopped.
        zcl_level_tmr_clear();
        break;                     
    }

    // generate the default response
    if (!hdr->frm_ctrl.dis_def_resp)
    {
        *resp_len = zcl_gen_def_resp(resp, ZCL_STATUS_SUCCESS, hdr);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
U8 zcl_level_gen_req(U8 *data, zcl_hdr_t *hdr, zcl_level_req_t *req)
{
    U8 len, *data_ptr;

    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    switch (hdr->cmd)
    {
    case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
        *data_ptr++ = req->move_to_level.level;
        *(U16 *)data_ptr = req->move_to_level.trans_time;
        data_ptr += sizeof(U16);
        break;

    case ZCL_LEVEL_CMD_MOVE:
        *data_ptr++ = req->move.dir;
        *(U16 *)data_ptr = req->move.rate;
        data_ptr += sizeof(U16);
        break;

    case ZCL_LEVEL_CMD_STEP:
        *data_ptr++ = req->step.dir;
        *data_ptr++ = req->step.step_size;
        *(U16 *)data_ptr = req->step.trans_time;
        data_ptr += sizeof(U16);
        break;

    default:
        break;
    }
    return data_ptr - data;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static mem_ptr_t *zcl_level_tmr_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(zcl_level_tmr_t))) != NULL)
    {
        list_add(level_tmr_list, mem_ptr);
    }

    return mem_ptr;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void zcl_level_tmr_free(mem_ptr_t *mem_ptr)
{
    if (mem_ptr)
    {
        list_remove(level_tmr_list, mem_ptr);
        mem_heap_free(mem_ptr);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void zcl_level_tmr_clear()
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_chop(level_tmr_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        zcl_level_tmr_free(mem_ptr);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_level_tmr_add(U8 *level, U16 rem_time, U8 step, U8 rem, bool dir, bool on_off, void (*action_handler)(U8, void *))
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = zcl_level_tmr_alloc()) != NULL)
    {
        ZCL_LEVEL_TMR(mem_ptr)->level           = level;
        ZCL_LEVEL_TMR(mem_ptr)->rem_time        = rem_time;
        ZCL_LEVEL_TMR(mem_ptr)->rem             = rem;
        ZCL_LEVEL_TMR(mem_ptr)->step            = step;
        ZCL_LEVEL_TMR(mem_ptr)->dir             = dir;
        ZCL_LEVEL_TMR(mem_ptr)->with_on_off     = on_off;
        ZCL_LEVEL_TMR(mem_ptr)->action_handler  = action_handler;
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_level_tmr_periodic(void *ptr)
{
    mem_ptr_t *mem_ptr;
    U8 level, step, rem;
    U16 rem_time;
    bool dir;

    for (mem_ptr = list_head(level_tmr_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        level       = *ZCL_LEVEL_TMR(mem_ptr)->level;
        step        = ZCL_LEVEL_TMR(mem_ptr)->step;
        rem_time    = ZCL_LEVEL_TMR(mem_ptr)->rem_time;
        dir         = ZCL_LEVEL_TMR(mem_ptr)->dir;
        rem         = ZCL_LEVEL_TMR(mem_ptr)->rem;

        if (rem_time == 0) 
        {
            // adjust the level value so that any small offset will be adjusted in the last step.
            *ZCL_LEVEL_TMR(mem_ptr)->level = level;
            
            // handle any actions by sending a refresh command to the action handler
            ZCL_LEVEL_TMR(mem_ptr)->action_handler(ZCL_LEVEL_ACTION_REFRESH, mem_ptr);

            // the time for this operation has expired. free the timer.
            zcl_level_tmr_free(mem_ptr);
        }
        else if ((level == 0xff) && (dir == ZCL_LEVEL_MODE_UP)) 
        {
            // we've reached the max level. inform the action handler in case any action is 
            // to be taken. then free the timer.
            ZCL_LEVEL_TMR(mem_ptr)->action_handler(ZCL_LEVEL_ACTION_MAX_LEVEL, mem_ptr);
            zcl_level_tmr_free(mem_ptr);
        }
        else if ((level == 0x00) && (dir == ZCL_LEVEL_MODE_DOWN))
        {
            // we've reached the min level. inform the action handler in case any action is
            // to be taken. then free the timer.
            ZCL_LEVEL_TMR(mem_ptr)->action_handler(ZCL_LEVEL_ACTION_MIN_LEVEL, mem_ptr);
            zcl_level_tmr_free(mem_ptr);
        }
        else
        {
            // now adjust the level. we need to handle the roundoff error first, otherwise we run
            // the risk of overflowing.

            // this part is to handle the roundoff error accumulation. 
            // add the remainder term to the accumulator to accumulate the roundoff error.
            // when it exceeds 0x7fff, then we will add one to the level. Just a note, the accumulator
            // is the fractional portion of the step size and it is normalized so that 0x8000 = 1. so
            // when we are > 0x7fff, then we've overflowed and need to adjust the step by 1 and subtract
            // 1 from the accumulator.
            ZCL_LEVEL_TMR(mem_ptr)->acc += rem;
            if (ZCL_LEVEL_TMR(mem_ptr)->acc > 0xff)
            {
                // subtract 1 from the accumulator
                ZCL_LEVEL_TMR(mem_ptr)->acc -= 0x100;
                
                // adjust the step size
                level = dir ? level - 1 : level + 1;
            }

            // limit the max and min values so that we don't overflow
            if (dir)
            {
                // we're moving downwards. if the level is less than the step size, then we
                // will set the level to 0. otherwise, just decrement the step size.  
                *ZCL_LEVEL_TMR(mem_ptr)->level = (level < step) ? 0 : (level - step);
            }
            else
            {
                // we're moving upwards. if the step size is greater than the headroom between 0xff
                // and the current level, then we need to limit it to the max value.
                *ZCL_LEVEL_TMR(mem_ptr)->level = ((0xff - level) < step) ? 0xff : (level + step);
            }

            // decrement the remaining time
            ZCL_LEVEL_TMR(mem_ptr)->rem_time--;

            // handle any actions by sending a refresh command to the action handler
            ZCL_LEVEL_TMR(mem_ptr)->action_handler(ZCL_LEVEL_ACTION_REFRESH, mem_ptr);
        }
    }
    ctimer_set(&level_tmr, CLOCK_SECOND/10, zcl_level_tmr_periodic, NULL);
}
