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
    \file mac_start.c
    \ingroup mac
    \brief MAC device startup handler

    The MAC start service is used to start up the MAC layer. Currently,
    it doesn't do much other than set some default values.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    This function preps the mac layer to be started. Normally, the MAC start
    service has more to it than setting default values, however most of the
    start service functions are used for 802.15.4 beacon mode. Since we're not
    using beacon mode, the start service is dramatically simplified.
*/
/**************************************************************************/
void mac_start_req(const mac_start_req_t *args)
{
    mac_pib_t *pib = mac_pib_get();

    // set the pan id, channel, short addr, and ext pan id in the pib
    pib->pan_id                 = args->pan_id;
    pib->coord                  = true;
    pib->short_addr             = 0x0000;
    pib->coord_addr.mode        = SHORT_ADDR;
    pib->coord_addr.short_addr  = 0x0000;

    // set up the hardware with the device parameters
    mac_set_channel(args->channel);
    drvr_set_pan_id(pib->pan_id);
    drvr_set_short_addr(pib->short_addr);
    drvr_set_coord(pib->coord);
}



