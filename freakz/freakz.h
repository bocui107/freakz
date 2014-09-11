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
    \file freakz.h
    \ingroup misc
    \brief Main Freakz header file

    The main header file for this stack.
*/
/*******************************************************************
    A smorgasbord of Zigbee files.
// have Ash get dirty felix sleepy now
*******************************************************************/
#ifndef FREAKZ_H
#define FREAKZ_H

#if (FREAKZ_DEBUG)
	#include <stdio.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#if (TEST_SIM == 1)
	#include <unistd.h>
	#include "test_sim.h"
	#include "sim_drvr.h"
	#define FREAKZ_DEBUG	1
	#define DEBUG_FRMS	1
	#define DEBUG_BUF	1
	#define DEBUG_MAC	1
	#define DEBUG_NWK	1
	#define DEBUG_APS	1
	#define DEBUG_ZDO	1
	#define DEBUG_ZCL	1
#endif

#if ((TEST_RAVENUSB == 1) || (TEST_RAVEN == 1))
	#include "drvr_avr_at86.h"
	#define FREAKZ_DEBUG	1
	#define DEBUG_FRMS	0
	#define DEBUG_BUF	0
	#define DEBUG_MAC	0
	#define DEBUG_NWK	0
	#define DEBUG_APS	0
	#define DEBUG_ZDO	0
	#define DEBUG_ZCL	0
#endif

#include "constants.h"
#include "types.h"
#include "buf.h"
#include "mem_heap.h"
#include "mac.h"
#include "dev_dbg.h"
#include "zcl.h"
#include "zdo.h"
#include "af.h"
#include "aps.h"
#include "nwk.h"
#include "mac_hw.h"
#include "misc.h"
#include "slow_clock.h"
#include "mmem.h"

/* MAC rx event */
process_event_t event_mac_rx;

/* Application Framework tx event */
process_event_t event_af_tx;

/* Application Framework rx event */
process_event_t event_af_rx;

/* Application Framework confirm event */
process_event_t event_af_conf;

/* Driver confirmation available */
process_event_t event_drvr_conf;

/* End dev bind request received */
process_event_t event_ed_bind_req;

/* End dev bind - clust match finished */
process_event_t event_ed_bind_match;

/* End dev bind - Unbind response received */
process_event_t event_unbind_resp;

void freakz_init();
#endif
