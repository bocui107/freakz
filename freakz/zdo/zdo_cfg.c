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
    \file zdo_cfg.c
    \ingroup zdo
    \brief ZDO configuration manager

    This file holds the functions that set and retrieve the descriptors that
    describe the configuration of this device. It also contains functions that
    set /changethe configuration for this device.

    Each device must contain the
    following standard descriptors:
    - Node descriptor - type and capabilities of the node
    - Power descriptor - power characteristics of the node

    There are also optional descriptors such as:
    - User descriptor - user defined descriptor
    - Complex descriptor - further detailed information about the device and descriptors
*/
/**************************************************************************/
#include "freakz.h"

// Here is where the actual descriptors are
static node_desc_t node_desc;                       ///< Node descriptor instantiation
static U16 pwr_desc;                                ///< Power descriptor instantiation
static char user_desc[ZIGBEE_MAX_USER_DESC_MSG];    ///< User descriptor instantiation

/**************************************************************************/
/*!
    Generate the default node and power descriptors. The default values
    can be changed for each device, and the descriptors themselves can be
    changed on-the-fly in case this is needed.
*/
/**************************************************************************/
void zdo_cfg_init()
{
    zdo_cfg_node_desc_set();
    zdo_cfg_pwr_desc_set();
}

/**************************************************************************/
/*!
    Return a pointer to the node descriptor structure.
*/
/**************************************************************************/
node_desc_t *zdo_cfg_node_desc_get()
{
    return &node_desc;
}

/**************************************************************************/
/*!
    Return a pointer to the power descriptor structure.
*/
/**************************************************************************/
U16 *zdo_cfg_pwr_desc_get()
{
    return &pwr_desc;
}

/**************************************************************************/
/*!
    Set the node descriptor. The changeable fields in the node descriptor
    are derived from the NWK Info Base (NIB) so the nib must be changed first.
    Then this function should be called to update the descriptor.
*/
/**************************************************************************/
void zdo_cfg_node_desc_set()
{
    nwk_nib_t *nib = nwk_nib_get();

    memset(&node_desc, 0, sizeof(node_desc_t));
    switch (nib->dev_type)
    {
    case NWK_COORDINATOR:
        node_desc.node_desc_field1 |= LOGICAL_TYPE_COORDINATOR;
        break;
    case NWK_ROUTER:
        node_desc.node_desc_field1 |= LOGICAL_TYPE_ROUTER;
        break;
    case NWK_END_DEVICE:
        node_desc.node_desc_field1 |= LOGICAL_TYPE_END_DEVICE;
        break;
    default:
        // should not end up here
        return;
    }
    node_desc.node_desc_field1 |= ZIGBEE_COMPLEX_DESC_AVAIL << NODE_DESC_COMPLEX_DESC_AVAIL_OFF;
    node_desc.node_desc_field1 |= ZIGBEE_USER_DESC_AVAIL    << NODE_DESC_USER_DESC_AVAIL_OFF;
    node_desc.node_desc_field2 |= FREQ_BAND_2400_MHZ        << NODE_DESC_FREQ_BAND_OFF;
    node_desc.mac_capab_flags   = nib->capability_info;
    node_desc.manuf_code        = ZIGBEE_MANUF_CODE;
    node_desc.max_buf_size      = ZIGBEE_MAX_BUFSIZE;
    node_desc.server_mask       = ZIGBEE_SERVER_MASK;
    node_desc.max_in_xfer       = ZIGBEE_MAX_IN_XFER;
    node_desc.max_out_xfer      = ZIGBEE_MAX_OUT_XFER;
    node_desc.desc_capab        = ZIGBEE_DESC_CAPABILITY;
}

/**************************************************************************/
/*!
    Set the power descriptor. The power descriptor is a 2-byte description
    of the power characteristics of the node. The values cannot be changed
    on-the-fly currently.
*/
/**************************************************************************/
void zdo_cfg_pwr_desc_set()
{
    pwr_desc = 0;
    pwr_desc |= CURR_PWR_MODE_RX_ON_IDLE;
    pwr_desc |= (1 << MAINS_OFF)            << POWER_DESC_AVAIL_PWR_SRC_OFF;
    pwr_desc |= (1 << MAINS_OFF)            << POWER_DESC_CURR_PWR_SRC_OFF;
    pwr_desc |= CURR_PWR_SRC_LVL_100_PCT    << POWER_DESC_CURR_PWR_SRC_LEVEL_OFF;
}

/**************************************************************************/
/*!
    Set the User descriptor message. The User descriptor contains a user
    defined string which can be a description of some parameters of the device
    or simply an identifier for this device.
*/
/**************************************************************************/
void zdo_cfg_user_desc_set(char *msg)
{
    memcpy(user_desc, msg, ZIGBEE_MAX_USER_DESC_MSG);
}

/**************************************************************************/
/*!
    Set the transaction persistence time for the indirect queue. The transaction
    persistence time is the length of time that a buffer can stay in the
    indirect queue before it is expired and purged. The argument is specified
    in seconds.
*/
/**************************************************************************/
void zdo_cfg_transaction_persist(U8 persist_time)
{
    nwk_nib_t *nib = nwk_nib_get();

    nib->traxn_persist_time = persist_time;
}

/**************************************************************************/
/*!
    Set the number of network scan attempts. When a network discovery is
    performed, then multiple network scan attempts can improve the quality
    of captured networks. By default, only one scan attempt is used for
    network discovery.

    \note This function is not fully implemented yet. Another way to improve
    the quality of network scans is to increase the duration of the scan.
*/
/**************************************************************************/
void zdo_cfg_nwk_scan_attempts_set(U8 scan_attempts)
{
    nwk_nib_t *nib = nwk_nib_get();

    nib->nwk_scan_attempts = scan_attempts;
}

/**************************************************************************/
/*!
    Set the time in msec between network scan attempts.

    \note This function is not fully implemented yet.
*/
/**************************************************************************/
void zdo_cfg_nwk_time_betw_scans_set(U16 interval)
{
    nwk_nib_t *nib = nwk_nib_get();

    nib->time_betw_scans = interval;
}

/**************************************************************************/
/*!
    Set the permit join duration value. Setting the duration to 0xFF will permanently
    enable the permit join flag. Setting it to 0 will disable the permit_join flag.
    Any other value will set off a timer and enable the permit_join flag until the
    timer expires.
*/
/**************************************************************************/
void zdo_cfg_nwk_permit_join_set(U8 duration)
{
    nwk_permit_join_req(duration);
}

