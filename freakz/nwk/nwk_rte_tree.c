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
    \file nwk_rte_tree.c
    \ingroup nwk
    \brief NWK tree routing implementation

    This file has the functions that will implement the tree routing algorithm
    as outlined in the Zigbee spec.
    Zigbee tree routing works on the basis of addresses. Addresses are
    assigned based on the position in the network. The main parameter
    that determines the address is the parent that the device is joined
    to. Based on the tree addressing scheme, a frame can be forwarded to
    its destination. The downside to this scheme is that it is unreliable
    since one failed node can potentially take down the whole tree.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Init the tree's cskip value to a default of 0xffff.
*/
/**************************************************************************/
void nwk_rte_tree_init()
{
    nwk_nib_t *nib = nwk_nib_get();
    nib->cskip = 0xFFFF;
}

/**************************************************************************/
/*!
    Calculate the CSkip value based on the depth. This will be used in determining the address
    of any joined device and also determining whether to route up or down in tree based routing.
    This function should only be called once after nwk_formation. After that, we can just store the
    value in a static variable so we never have to touch this again.
*/
/**************************************************************************/
U16 nwk_rte_tree_calc_cskip(U8 depth)
{
    U16 exp, temp;
    S16 num, den;
        U8 i;
    nwk_nib_t *nib = nwk_nib_get();

        exp = nib->max_depth - (depth + 1);

        if (nib->max_routers == 1)
        {
                return (U16)(1 + (nib->max_children * exp));
        }
        else
        {
        if (depth < nib->max_depth)
        {
            // Init the value of temp to 1. This is the lowest value it can have.
            // Then calculate the value of (max_routers^exp)
            temp = 1;
            for (i=0; i<exp; i++)
            {
                temp *= nib->max_routers;
            }

            // Need to be careful. There is a potential overflow if the value of max_children is large and temp is also large.
            num = (S16)((1 + nib->max_children - nib->max_routers) - (nib->max_children * temp));
            den = 1 - nib->max_routers;
            return (U16)(num/den);
        }
        }
    return 0;
}

/**************************************************************************/
/*!
        Based on the device number and the depth, can calculate the address for the device that is
        requesting a join. For a router, the address is allocated from the beginning of the address
    space and is based on the address and cskip of the device that is allowing the join. This
    is potentially tricky to understand so if all else fails, you can email me or post a message on the
    forum. If I successfully answer your question, you owe me a beer.
*/
/**************************************************************************/
U16 nwk_rte_tree_calc_rtr_addr(U16 cskip, U8 num)
{
    nwk_nib_t *nib = nwk_nib_get();
    return (U16)(nib->short_addr + (num * cskip) + 1);
}

/**************************************************************************/
/*!
        Based on the device number and the depth, can calculate the address for the device that is
        requesting a join. For an end device, the address is allocated at the end of the router's
    address space. This is because the beginning of the address space is dedicated for the routers
    that will potentially be joining that device. This is potentially tricky to understand so if
    all else fails, you can email me or post a message on the forum. If I successfully answer your
    question, you owe me a doughnut.

    \note num = the dev number that is requesting to join. The dev num is sequential in the
    order of joining. So the first device to join the router will be 1, the second will be 2, etc...
*/
/**************************************************************************/
U16 nwk_rte_tree_calc_ed_addr(U16 cskip, U8 num)
{
    nwk_nib_t *nib = nwk_nib_get();
    return (U16)(nib->short_addr + (cskip * nib->max_routers) + (num + 1));
}

/**************************************************************************/
/*!
    This is a helper function that will locate the next downstream hop in
    the routing tree based on the destination address.
*/
/**************************************************************************/
static U16 nwk_rte_tree_get_dwnstrm_rtr_addr(U16 dest_addr)
{
    U8 i;
    U16 lower_addr, upper_addr;
    nwk_nib_t *nib = nwk_nib_get();
    address_t addr;

    addr.mode       = SHORT_ADDR;
    addr.short_addr = dest_addr;

    // can we get an exact match for the child device?
    if (nwk_neighbor_tbl_addr_exists(&addr))
    {
        return dest_addr;
    }

    // no child exists. find the downstream rtr to send the frame to.
    // to get the downstream router, we need to find a router whose
    // address allocation contains the dest_addr. That means that dest_addr must
    // lie between the upper and lower addresses in the rtr's addr allocation range.
    for (i=0; i<nib->max_routers; i++)
    {
        lower_addr = (U16)((nib->cskip * i) + 1);
        upper_addr = lower_addr + nib->cskip;

        if ((dest_addr > lower_addr) && (dest_addr < upper_addr))
        {
            return lower_addr;
        }
    }
    return INVALID_NWK_ADDR;
}

/**************************************************************************/
/*!
Decides whether to route up or down the tree. If we route up, then we just
return the next parent that we find. If we route down, then we search for either
a child with the exact address, or a router with an address that is in the address
sub-block range:
        - router_addr < dest_addr < (router_addr + cskip)

If it satisfies that criteria, then that means that the device will be somewhere
down that branch of the network.
*/
/**************************************************************************/
U16 nwk_rte_tree_calc_next_hop(U16 dest)
{
        U16 cskip_parent;
    U16 dest_addr;
        bool route_down;
    nwk_nib_t *nib = nwk_nib_get();

    // if cskip is uninitialized, then calculate the value for this device
    // and save it in the nib.
    if (nib->cskip == 0xFFFF)
    {
        nib->cskip = nwk_rte_tree_calc_cskip(nib->depth);
    }

    // we need the depth one layer up to calculate the parent's cskip value.
    // if we're the coordinator, then we don't care because we will route down
    // no matter what. otherwise, we use the parent's cskip value to calc
    // whether to route up or down.
    cskip_parent = (nib->depth != 0) ? nwk_rte_tree_calc_cskip(nib->depth - 1) : 0;
        route_down = ((dest > nib->short_addr) && (dest < (nib->short_addr + cskip_parent)));

    // if we're the coordinator or we calculate that we need to route down, then
    // look for the child that will be our next hop. otherwise, just send it to
    // our parent.
    if ((nib->short_addr == 0) || route_down)
    {
        dest_addr = nwk_rte_tree_get_dwnstrm_rtr_addr(dest);
    }
    else
    {
        dest_addr = nwk_neighbor_tbl_get_parent();
    }
    return dest_addr;
}




