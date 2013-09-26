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
    \defgroup zdo ZDO - Zigbee Device Object
    \file zdo.c
    \ingroup zdo
    \brief Zigbee Device Object toplevel file

    This file implements the Zigbee Device Object. The Zigbee Device Object
    is the default endpoint (ep 0) of all Zigbee devices and controls network
    management, service discovery, binding, and discovery cache management.

    It sits on top of the Application Framework and is treated as any other
    endpoint. This means that it still needs to register its simple descriptor
    and rx/confirm callbacks with the AF before any transactions can occur.
*/
/**************************************************************************/
#include "freakz.h"

#if (TEST_ZDO)
    #include "test_zdo.h"
#endif

/*********************** Global Variables **************************/
static zdo_pcb_t pcb;   ///< ZDO protocol control block instantiation

/*!
    Simple descriptor for this ep. The ZDO doesn't really need a simple
    descriptor, but it makes it easier to register it with the af_ep table.
    What I really care about is using the ZDO rx and conf callbacks.
*/
static U8 zdo_simple_desc[] = {
    ZDO_EP,                     // ep
    ZIGBEE_PROF_ID,             // profile id
    0,                          // dev id
    0,                          // dev ver
    0,                          // num in clusters
    0                           // num out clusters
};

/**************************************************************************/
/*!
    This is the ZDO's master cluster list. It is a ZDO request and response
    function pointer table. When a request/response comes in, it gets looked
    up in here and the frame will get routed to the function that matches the
    cluster ID.
*/
/**************************************************************************/
static zdo_handler_t zdo_handler_tbl[] =
{
    // requests
    {NWK_ADDR_REQ_CLUST,            zdo_disc_addr_req_handler          },
    {IEEE_ADDR_REQ_CLUST,           zdo_disc_addr_req_handler          },
    {NODE_DESC_REQ_CLUST,           zdo_disc_node_desc_req_handler     },
    {PWR_DESC_REQ_CLUST,            zdo_disc_pwr_desc_req_handler      },
    {SIMPLE_DESC_REQ_CLUST,         zdo_disc_simple_desc_req_handler   },
    {ACTIVE_EP_REQ_CLUST,           zdo_disc_active_ep_req_handler     },
    {MATCH_DESC_REQ_CLUST,          zdo_disc_match_desc_req_handler    },
    {DEV_ANNCE_REQ_CLUST,           zdo_disc_dev_annce_req_handler     },
    {END_DEV_BIND_REQ_CLUST,        zdo_bind_ed_req_handler            },
    {BIND_REQ_CLUST,                zdo_bind_unbind_req_handler        },
    {UNBIND_REQ_CLUST,              zdo_bind_unbind_req_handler        },
    {NWK_DISC_REQ_CLUST,            zdo_nwk_disc_req_handler           },
    {NWK_LQI_REQ_CLUST,             zdo_nwk_lqi_req_handler            },
    {NWK_LEAVE_REQ_CLUST,           zdo_nwk_leave_req_handler          },
    {NWK_PERMIT_JOIN_REQ_CLUST,     zdo_nwk_permit_join_req_handler    },
    {NWK_UPDATE_REQ_CLUST,          zdo_nwk_update_req_handler         },

    // responses
    {NWK_ADDR_RESP_CLUST,           zdo_disc_addr_resp_handler         },
    {IEEE_ADDR_RESP_CLUST,          zdo_disc_addr_resp_handler         },
    {NODE_DESC_RESP_CLUST,          NULL                               },
    {PWR_DESC_RESP_CLUST,           NULL                               },
    {SIMPLE_DESC_RESP_CLUST,        NULL                               },
    {ACTIVE_EP_RESP_CLUST,          NULL                               },
    {MATCH_DESC_RESP_CLUST,         NULL                               },
    {END_DEV_BIND_RESP_CLUST,       zdo_bind_ed_resp_handler           },
    {BIND_RESP_CLUST,               zdo_bind_unbind_resp_handler       },
    {UNBIND_RESP_CLUST,             zdo_bind_unbind_resp_handler       },
    {NWK_LQI_RESP_CLUST,            zdo_nwk_lqi_resp_handler           },
    {NWK_DISC_RESP_CLUST,           zdo_nwk_disc_resp_handler          },
    {NWK_LEAVE_RESP_CLUST,          zdo_nwk_leave_resp_handler         },
    {NWK_UPDATE_RESP_CLUST,         zdo_nwk_update_resp_handler        },

    {END_MARKER_CLUST,              NULL                               }
};

/**************************************************************************/
/*!
    This the ZDO callback pointer. When an application registers itself, it
    needs to register its callbacks here. That way, when a nwk_join_conf
    or a nwk_leave_ind come through, we can route it to the appropriate
    handler in the application.
*/
/**************************************************************************/
static zdo_cb_t *zdo_cb;

/**************************************************************************/
/*!
    Start up the ZDO endpoint. The various modules making up the ZDO are
    started, variables are initialized, and finally, the endpoint is registered
    with the AF.
*/
/**************************************************************************/
void zdo_init()
{
        zdo_cb = NULL;
        aps_aib_t *aib = aps_aib_get();
    zdo_cfg_init();
    aib->seq = (U8)drvr_get_rand();
    memset(&pcb, 0, sizeof(zdo_pcb_t));
    af_ep_add(ZDO_EP, zdo_simple_desc, AF_BASE_SIMPLE_DESC_SIZE, false, zdo_rx_handler, zdo_conf_handler);

    zdo_bind_mgr_init();
}

/**************************************************************************/
/*!
    Register the callbacks from the ZDO to the application layer.
*/
/**************************************************************************/
void zdo_reg_cb(zdo_cb_t *cb)
{
    zdo_cb = cb;
}

/**************************************************************************/
/*!
    Return the callback structure to the calling function.
*/
/**************************************************************************/
zdo_cb_t *zdo_cb_get()
{
    return zdo_cb;
}

/**************************************************************************/
/*!
    Return a pointer to the ZDO Protocol Control Block structure.
*/
/**************************************************************************/
zdo_pcb_t *zdo_pcb_get()
{
    return &pcb;
}

/**************************************************************************/
/*!
    Return the next sequence number. The seq number is used to identify the
    ZDO frame.
*/
/**************************************************************************/
U8 zdo_seq_get()
{
    aps_aib_t *aib = aps_aib_get();
    return aib->seq++;
}

/**************************************************************************/
/*!
    This is the default TX function for the ZDO. It's mostly a wrapper function
    that forwards the data down to the AF's TX function.
*/
/**************************************************************************/
void zdo_tx(U8 *data, U8 len, U16 dest_addr, U16 clust_id, U8 tx_opt, U8 handle)
{
    af_tx(data, len, ZDO_EP, dest_addr, ZDO_EP, clust_id, ZDO_PROFILE_ID, APS_DEST_ADDR_16_EP_PRESENT, tx_opt, ZIGBEE_DEFAULT_RADIUS, handle);
}

/**************************************************************************/
/*!
    This is the default handler for RX data coming into the ZDO. This function must
    be registered with the AF in the endpoint list so that ZDO rx data gets routed
    here properly. It calls a req/resp handler which is located in a table and contains
    function pointers to the handler based on the cluster ID of the data.
*/
/**************************************************************************/
void zdo_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id)
{
    U8 i;

    for (i=0; zdo_handler_tbl[i].clust_id != END_MARKER_CLUST; i++)
    {
        if (zdo_handler_tbl[i].clust_id == clust_id)
        {
            if (zdo_handler_tbl[i].handler)
            {
                zdo_handler_tbl[i].handler(data, len, src_addr, src_ep, clust_id);
            }
        }
    }
}

/**************************************************************************/
/*!
    This is the default confirmation handler and this function must be registered in
    the AF's endpoint list. Confirmations will then get routed here and can be processed.
*/
/**************************************************************************/
//lint -e{715} Symbol 'handle' not referenced
void zdo_conf_handler(U8 status, U8 handle)
{
    // TODO: How do we want to handle the confirms?
}
