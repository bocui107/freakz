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
    \file sim_drvr.h
    \ingroup simdrvr
    \brief Simulator driver header file

    This is the header file that holds the test driver constants and
    other stuff.
*/
/*******************************************************************/
#ifndef SIM_DRVR_H
#define SIM_DRVR_H

#include "contiki.h"
#include "types.h"
#include "buf.h"

/*!
    Enums used in the simulated driver.
*/
enum SIM_DRVR_ENUMS
{
	SIM_DRVR_MAX_BUF_SIZE = 128  ///< Max buf size for the test bufs
};


PROCESS_NAME(drvr_process);     ///< Main process for simulation driver
void drvr_init();
void drvr_rx_isr();
void drvr_write_rx_buf(U8 *buf, U8 len);
U8 drvr_read_tx_buf(U8 *buf);
bool drvr_get_cca();
U8 drvr_set_channel(U8 channel);
U8 drvr_get_channel();
U8 drvr_get_ed();
U16 drvr_get_rand();
U8 drvr_tx(const buffer_t *buf);
void drvr_set_frm_pend(bool pend);
void drvr_set_ext_addr(U64 addr);
void drvr_set_short_addr(U16 addr);
void drvr_set_pan_id(U16 pan_id);
void drvr_set_coord(bool coord);

#endif // SIM_DRVR_H
