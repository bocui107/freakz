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
    \file slow_clock.c
    \ingroup misc
    \brief Slow 1 second clock

    The purpose of these functions is to create a slow clock to be used
    by the tables that require periodic but slow checking.
*/
/**************************************************************************/
#include "freakz.h"
#include "zcl_id.h"

static struct ctimer slow_clk;  ///< Callback timer for the slow clock. This is the main timer that drives it.


/**************************************************************************/
/*!
    These are the callbacks for the slow clock to call every second. The
    slow clock is used because it saves space from having a dedicated timer
    for each of these functions. Instead, it's just a free running 1 second timer
    that will call each of these functions every second. Since it is only
    used for slow operations, it won't affect performance to call all these
    functions every second.
*/
/**************************************************************************/
static void (*timeouts[])(void) =
{
    af_conf_tbl_periodic,
    aps_dupe_periodic,
    aps_retry_periodic,
    nwk_brc_periodic,
    nwk_pend_periodic,
    nwk_rte_disc_periodic,
    mac_indir_periodic,
    zdo_nwk_mgr_periodic,
    zcl_rpt_periodic,
    zcl_id_tmr_periodic,
    NULL
};

/**************************************************************************/
/*!
    Initialize the slow clock.
*/
/**************************************************************************/
void slow_clock_init()
{
    slow_clock_periodic(NULL);
}

/**************************************************************************/
/*!
    Turn off the slow clock.
*/
/**************************************************************************/
void slow_clock_stop()
{
    ctimer_stop(&slow_clk);
}

/**************************************************************************/
/*!
    This function gets called every second and will handle the timeouts for
    all functions that require a slow clock.
*/
/**************************************************************************/
//lint -e{715} Symbol 'ptr' not referenced
//lint -e{818} Pointer parameter 'ptr' could be declared as pointing to const
// Note: (void *ptr) format required for callback timer
void slow_clock_periodic(void *ptr)
{
    U8 i;

    for (i=0; timeouts[i] != NULL; i++)
    {
        timeouts[i]();
    }
    ctimer_set(&slow_clk, CLOCK_SECOND, slow_clock_periodic, NULL);
}



