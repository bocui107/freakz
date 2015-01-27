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
    \file test_raven.c
    \ingroup
*/
/*******************************************************************/
#include "freakz.h"
#include "test_avr_raven.h"
#include "raven-lcd.h"

#define ASSOC_LIST_ENTRY(m)   ((assoc_dev_t *)MMEM_PTR(&m->mmem_ptr))

// simple descriptor for this ep
static U8 test_raven_simple_desc[] = {
    TEST_EP,                    // ep
    TEST_PROFILE_ID,            // profile id
    0,                          // dev id
    1,                          // dev ver
    1,                          // num in clusters
    TEST_IN_CLUST,              // in clusters
    0                           // num out clusters
};

static zdo_cb_t test_raven_cb =
{
    test_raven_nwk_form_conf,
    test_raven_nwk_join_conf,
    test_raven_nwk_join_ind,
    NULL,
    NULL
};

LIST(assoc_list);
static U8 index;

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_init()
{
    aps_aib_t *aib = aps_aib_get();

    // register the callbacks with the zdo layer
    zdo_reg_cb(&test_raven_cb);

    // register the endpoint with the ZDO
    af_ep_add(TEST_EP, test_raven_simple_desc, AF_BASE_SIMPLE_DESC_SIZE + 4, false, test_raven_rx_handler, test_raven_conf_handler);

    // init the associated devices list
    list_init(assoc_list);

    // set the device index to zero. we will use this to cross reference and find a device to send to
    index = 0;

    aib->desig_parent = 0x0;
    aib->use_desig_parent = true;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
mem_ptr_t *test_raven_assoc_alloc()
{
    mem_ptr_t *mem_ptr;

    if ((mem_ptr = mem_heap_alloc(sizeof(assoc_dev_t))) != NULL)
    {
        list_add(assoc_list, mem_ptr);
    }
    return mem_ptr;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
bool test_raven_assoc_addr_exists(U16 nwk_addr)
{
    mem_ptr_t *mem_ptr;
    bool match;

    for (mem_ptr = list_head(assoc_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        match = (nwk_addr == ASSOC_LIST_ENTRY(mem_ptr)->nwk_addr);
    }
    return match;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static mem_ptr_t *test_raven_assoc_find(U8 index)
{
    mem_ptr_t *mem_ptr;

    for (mem_ptr = list_head(assoc_list); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        if (index == ASSOC_LIST_ENTRY(mem_ptr)->index)
        {
            return mem_ptr;
        }
    }
    return NULL;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
static void test_raven_assoc_add(U16 nwk_addr)
{
    mem_ptr_t *mem_ptr;

    if (!test_raven_assoc_addr_exists(nwk_addr))
    {
        if ((mem_ptr = test_raven_assoc_alloc()) != NULL)
        {
            ASSOC_LIST_ENTRY(mem_ptr)->index = index++;
            ASSOC_LIST_ENTRY(mem_ptr)->nwk_addr = nwk_addr;
        }
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 dest_clust)
{
    switch (dest_clust)
    {
    case TEST_IN_CLUST:
        send_frame(REPORT_RX_DATA, 1, data);
        break;

    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_conf_handler(U8 status, U8 in_handle)
{
//    DBG_PRINT("Conf received: Handle = %02X, Status = %02X.\n", in_handle, status);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_zdo_start(bool coord)
{
    aps_aib_t *aib = aps_aib_get();

    aib->desig_coord = coord;
    zdo_nwk_start();
}

/**************************************************************************/
/*!
    This is the nwk formation confirm callback which is called from the ZDO
    layer. This goes into the callback structure.
*/
/**************************************************************************/
void test_raven_nwk_form_conf(U8 status)
{
    U8 form_ok = (status == NWK_SUCCESS);
    send_frame(REPORT_NWK_FORM_CONF, 1, &form_ok);
}

/**************************************************************************/
/*!
    This is the callback function for the nwk join confirm that comes from
    the ZDO layer.
*/
/**************************************************************************/
void test_raven_nwk_join_conf(U8 status, U16 nwk_addr, U16 parent_addr)
{
    U8 join_ok = (status == NWK_SUCCESS);
    U8 payload[3];

    payload[0] = join_ok;
    if (join_ok)
    {
        test_raven_assoc_add(parent_addr);
        memcpy(&payload[1], &nwk_addr, 2);
        send_frame(REPORT_NWK_JOIN_CONF, 3, payload);
    }
    else
    {
        send_frame(REPORT_NWK_JOIN_CONF, 1, payload);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_tx_data(U8 len, U8 *data)
{
    U8 data_handle, index;
    mem_ptr_t *mem_ptr;
    U16 nwk_addr;

    index = *data++;
    len--;

    if ((mem_ptr = test_raven_assoc_find(index)) != NULL)
    {
        data_handle = af_handle_get();
        nwk_addr = ASSOC_LIST_ENTRY(mem_ptr)->nwk_addr;
        af_tx(data, len, TEST_EP, nwk_addr, TEST_EP, TEST_IN_CLUST, TEST_PROFILE_ID, APS_DEST_ADDR_16_EP_PRESENT, (1<<APS_TX_RTE_DISC_DISABLE), ZIGBEE_DEFAULT_RADIUS, data_handle);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_nwk_join_ind(U16 nwk_addr)
{
    test_raven_assoc_add(nwk_addr);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void test_raven_set_addr()
{
    mac_pib_t *pib = mac_pib_get();

    drvr_set_rand_seed();
    pib->ext_addr = drvr_get_rand();
    drvr_set_ext_addr(pib->ext_addr);
}
