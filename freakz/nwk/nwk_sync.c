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
    \file nwk_sync.c
    \ingroup nwk
    \brief NWK sync

    This file contains functions that support the nlme sync service.
    Since beacon mode is not supported in this stack, the sync function
    will only generate a poll request.
*/
/**************************************************************************/
#include "freakz.h"

static struct ctimer sync_tmr;  ///< Callback timer for the sync poll

/**************************************************************************/
/*!
    Generate a MAC poll request to extract indirect data from the parent.
*/
/**************************************************************************/
void nwk_sync_req()
{
    mac_poll_req();

    // that was easy, wasn't it...
}

/**************************************************************************/
/*!
    For a sleepy device, a polling interval is usually set so that it will
    wake up every polling period and poll the parent for any data. If no data exists,
    then the device will just go back to sleep. This function will start the
    polling timer which determines when the device will poll the parent for data.
*/
/**************************************************************************/
void nwk_sync_start()
{
    ctimer_set(&sync_tmr, ZIGBEE_POLL_INTERVAL * CLOCK_SECOND, nwk_sync_periodic, NULL);
}

/**************************************************************************/
/*!
    This is a periodic function that will poll the parent for indirect
    data.
*/
/**************************************************************************/
//lint -e{715} Info 715: Symbol 'ptr' not referenced
//lint -e{818} Info 818: Pointer parameter ptr' could be declared as pointing to const
void nwk_sync_periodic(void *ptr)
{
    nwk_sync_req();
    ctimer_set(&sync_tmr, ZIGBEE_POLL_INTERVAL * CLOCK_SECOND, nwk_sync_periodic, NULL);
}

/**************************************************************************/
/*!
    Stop the NWK sync timer.
*/
/**************************************************************************/
void nwk_sync_stop_tmr()
{
    ctimer_stop(&sync_tmr);
}
