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
    \file nwk_reset.c
    \ingroup nwk
    \brief NWK reset handler

    This file contains functions that handle resetting this device's
    NWK layer.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Resets the nib, pcb, and clears all table entries. If warm start is set, it will only
    clear the table entries.
*/
/**************************************************************************/
void nwk_reset_req(bool cold_start)
{
    if (cold_start)
    {
        slow_clock_stop();
        nwk_sync_stop_tmr();
        nwk_rte_mesh_stop_tmr();
        nwk_init();
        slow_clock_init();
    }

    // clear all the tables
    nwk_rte_disc_clear();
    nwk_rte_tbl_clear();
    nwk_neighbor_tbl_clear();
    nwk_pend_clear();
    nwk_brc_clear();

    // reset the mac
    mac_reset_req(cold_start);
}


