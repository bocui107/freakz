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
    \file nwk_permit_join.c
    \ingroup nwk
    \brief NWK permit join

    This file contains functions that implement the permit_join service.
*/
/*******************************************************************/
#include "freakz.h"

static struct ctimer permit_join_tmr;           ///< Callback timer to timeout the permit join

/**************************************************************************/
/*!
    Request to permit joining for a specified duration. The value specified
    is in units of seconds. A duration of 0 will disable joining for this device.
    A value of 0xFF will permanently enable joining for this device.
*/
/**************************************************************************/
void nwk_permit_join_req(U8 duration)
{
    mac_pib_t *pib = mac_pib_get();

    switch (duration)
    {
    case NWK_PERMIT_JOIN_DISABLE:
        pib->assoc_permit = false;
        break;

    case NWK_PERMIT_JOIN_ENABLE:
        pib->assoc_permit = true;
        break;

    default:
        pib->assoc_permit = true;
        ctimer_set(&permit_join_tmr, duration * CLOCK_SECOND, nwk_permit_join_disable, NULL);
        break;
    }
}

/**************************************************************************/
/*!
    Disable the nwk permit join. When nwk_permit_join is called, it will set
    a timer for the duration that devices are allowed to join. Once the duration
    expires, this function will be called to disable joining.
*/
/**************************************************************************/
void nwk_permit_join_disable()
{
    mac_pib_t *pib = mac_pib_get();

    pib->assoc_permit = false;
}



