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
    \file zdo.h
    \ingroup zdo
    \brief Zigbee Device Object header file

    This is the header file for the ZDO endpoint.
*/
/*******************************************************************/
#ifndef ZDO_H
#define ZDO_H

#define ZDO_REQ_MAX_BUF_SIZE        50      ///< The max buf size that we will use
#define ZDO_MAX_CLUST               10      ///< The maximum amount of clusters that we will include in any discovery cluster list
#define ZDO_RMT_NWK_DISC_TIMEOUT    30      ///< The amount of seconds before we clear the data from a remote nwk discovery operation
#define ZDO_PROFILE_ID              0x0000

#define ED_BIND(m)  ((ed_bind_t *)MMEM_PTR(&m->mmem_ptr))       ///< De-reference the mem ptr and cast it as an ed_bind structure
#define RMT_DISC(m) ((rmt_nwk_disc_t *)MMEM_PTR(&m->mmem_ptr))  ///< De-reference the mem ptr and cast it as an zdo nwk disc structure

/****************************************************************/
/*!
    Enumerated definitions for the ZDO clusters
*/
/****************************************************************/
enum ZDO_CLUSTER_ENUMS
{
    // zdo_clusters
    NWK_ADDR_REQ_CLUST                  = 0x0000,   ///< Network address request cluster ID
    IEEE_ADDR_REQ_CLUST                 = 0x0001,   ///< IEEE address request cluster ID
    NODE_DESC_REQ_CLUST                 = 0x0002,   ///< Node descriptor request cluster ID
    PWR_DESC_REQ_CLUST                  = 0x0003,   ///< Power descriptor request cluster ID
    SIMPLE_DESC_REQ_CLUST               = 0x0004,   ///< Simple descriptor request cluster ID
    ACTIVE_EP_REQ_CLUST                 = 0x0005,   ///< Active endpoint list request cluster ID
    MATCH_DESC_REQ_CLUST                = 0x0006,   ///< Match descriptor request cluster ID
    DEV_ANNCE_REQ_CLUST                 = 0x0013,   ///< End device announce request cluster ID
    END_DEV_BIND_REQ_CLUST              = 0x0020,   ///< End device vind request clust ID
    BIND_REQ_CLUST                      = 0x0021,   ///< Bind request clust ID
    UNBIND_REQ_CLUST                    = 0x0022,   ///< Unbind request clust ID
    NWK_DISC_REQ_CLUST                  = 0x0030,   ///< Network discovery request cluster ID
    NWK_LQI_REQ_CLUST                   = 0x0031,   ///< Network LQI request cluster ID
    NWK_LEAVE_REQ_CLUST                 = 0x0034,   ///< Network leave request cluster ID
    NWK_PERMIT_JOIN_REQ_CLUST           = 0x0036,   ///< Network permit join request cluster ID
    NWK_UPDATE_REQ_CLUST                = 0x0038,   ///< Network update request cluster ID

    NWK_ADDR_RESP_CLUST                 = 0x8000,   ///< Network address response cluster ID
    IEEE_ADDR_RESP_CLUST                = 0x8001,   ///< IEEE address response cluster ID
    NODE_DESC_RESP_CLUST                = 0x8002,   ///< Node descriptor response cluster ID
    PWR_DESC_RESP_CLUST                 = 0x8003,   ///< Power descriptor response cluster ID
    SIMPLE_DESC_RESP_CLUST              = 0x8004,   ///< Simple descriptor response cluster ID
    ACTIVE_EP_RESP_CLUST                = 0x8005,   ///< Active endpoint list response cluster ID
    MATCH_DESC_RESP_CLUST               = 0x8006,   ///< Match descriptor response cluster ID
    END_DEV_BIND_RESP_CLUST             = 0x8020,   ///< End device bind response clust ID
    BIND_RESP_CLUST                     = 0x8021,   ///< Bind response clust ID
    UNBIND_RESP_CLUST                   = 0x8022,   ///< Unbind response clust ID
    NWK_DISC_RESP_CLUST                 = 0x8030,   ///< Network discovery response cluster ID
    NWK_LQI_RESP_CLUST                  = 0x8031,   ///< Network LQI response cluster ID
    NWK_LEAVE_RESP_CLUST                = 0x8034,   ///< Network Leave response cluster ID
    NWK_UPDATE_RESP_CLUST               = 0x8038,   ///< Network update response cluster ID
    NWK_PERMIT_JOIN_RESP_CLUST          = 0x8036,   ///< Network permit join response cluster ID

    // response sizes
    NWK_ADDR_RESP_BASE_SIZE             = 12,       ///< Network address response base size in bytes
    NODE_DESC_RESP_BASE_SIZE            = 17,       ///< Node descriptor response base size in bytes
    PWR_DESC_RESP_BASE_SIZE             = 6,        ///< Power descriptor response base size in bytes

    ZDO_SINGLE_DEV_RESP                 = 0x0,      ///< Zigbee NWK address request only for target device
    ZDO_EXT_DEV_RESP                    = 0x1       ///< Zigbee NWK address request for target and all children
};

/****************************************************************/
/*!
    Enumerated definitions for the descriptors
*/
/****************************************************************/
enum ZDO_DESC_ENUMS
{
// node descriptor enums
    NODE_DESC_COMPLEX_DESC_AVAIL_OFF    = 3,
    NODE_DESC_USER_DESC_AVAIL_OFF       = 4,
    NODE_DESC_FREQ_BAND_OFF             = 3,

    NODE_DESC_LOGICAL_TYPE_MASK         = 0x7,  ///< Node desc - logical type field (coord, router, end device)
    NODE_DESC_COMPLEX_DESC_AVAIL_MASK   = (1 << NODE_DESC_COMPLEX_DESC_AVAIL_OFF),  ///< Node desc - complex desc avail field
    NODE_DESC_USER_DESC_AVAIL_MASK      = (1 << NODE_DESC_USER_DESC_AVAIL_OFF),     ///< Node desc - user desc avail field
    NODE_DESC_APS_FLAGS_MASK            = 0x7,                                      ///< Node desc - APS flags (not supported in spec)
    NODE_DESC_FREQ_BAND_MASK            = (0x1F << NODE_DESC_FREQ_BAND_OFF),        ///< Node desc - Freq band (868, 915, 2400 MHz)

    LOGICAL_TYPE_COORDINATOR            = 0x0,  ///< Coordinator value in logical type field
    LOGICAL_TYPE_ROUTER                 = 0x1,  ///< Router value in logical type field
    LOGICAL_TYPE_END_DEVICE             = 0x2,  ///< End device value in logical type field

    FREQ_BAND_868_MHZ                   = 0x0,  ///< 868 MHz frequency band for this device
    FREQ_BAND_915_MHZ                   = 0x2,  ///< 915 MHz frequency band for this device
    FREQ_BAND_2400_MHZ                  = 0x3,  ///< 2400 MHz frequency band for this device

    SERVER_MASK_PRIM_TRUST_CTR          = 0x0,  ///< This device is the primary trust center
    SERVER_MASK_BACKUP_TRUST_CTR        = 0x1,  ///< This device is backup trust center
    SERVER_MASK_PRIM_BIND_TBL_CACHE     = 0x2,  ///< This device is primary binding table
    SERVER_MASK_BACKUP_BIND_TBL_CACHE   = 0x3,  ///< This device is backup binding table
    SERVER_MASK_PRIM_DISC_CACHE         = 0x4,  ///< This device is primary discovery cache
    SERVER_MASK_BACKUP_DISC_CACHE       = 0x5,  ///< This device is backup discovery cache
    SERVER_MASK_NWK_MGR                 = 0x6,  ///< This device is network manager

    DESC_CAPAB_EXT_ACTIVE_EP_LIST_OFF   = 0,    ///< Extended active endpoint list available
    DESC_CAPAB_EXT_SIMPLE_DESC_LIST_OFF = 1,    ///< Extended simple descriptor list available

    // power desc enums
    POWER_DESC_CURR_PWR_MODE_OFF        = 0,    ///< Power desc - current power mode offset
    POWER_DESC_AVAIL_PWR_SRC_OFF        = 4,    ///< Power desc - avail power source offset
    POWER_DESC_CURR_PWR_SRC_OFF         = 8,    ///< Power desc - current power src offset
    POWER_DESC_CURR_PWR_SRC_LEVEL_OFF   = 12,   ///< Power desc - power source level offset

    POWER_DESC_CURR_PWR_MODE_MASK       = 0xf,  ///< Current power savings mode
    POWER_DESC_AVAIL_POWER_SRC_MASK     = (0xf << POWER_DESC_AVAIL_PWR_SRC_OFF),    ///< Available power sources
    POWER_DESC_CURR_PWR_SRC_MASK        = (0xf << POWER_DESC_CURR_PWR_SRC_OFF),         ///< Power sources available
    POWER_DESC_CURR_PWR_SRC_LEVEL_MASK  = (0xf << POWER_DESC_CURR_PWR_SRC_LEVEL_OFF),   ///< Amount of power left

    CURR_PWR_MODE_RX_ON_IDLE            = 0x0,  ///< Receiver always on
    CURR_PWR_MODE_RX_ON_PERIOD          = 0x1,  ///< Receiver periodically turned on
    CURR_PWR_MODE_RX_ON_EVENT           = 0x2,  ///< Receiver turned on by some external event

    MAINS_OFF                           = 0,    ///< Mains power source bit offset
    RECHARGEABLE_BATT_OFF               = 1,    ///< Rechargeable batt bit offset
    DISPOSABLE_BATT_OFF                 = 2,    ///< Disposable batt bit offset

    CURR_PWR_SRC_LVL_CRITICAL           = 0x0,  ///< Current power level is at critical
    CURR_PWR_SRC_LVL_33_PCT             = 0x4,  ///< Current power level is at 33%
    CURR_PWR_SRC_LVL_66_PCT             = 0x8,  ///< Current power level is at 66%
    CURR_PWR_SRC_LVL_100_PCT            = 0xC   ///< Current power level is at 100%
};

/*!
    Miscellaneous enumerated definitions used in the ZDO
*/
enum ZDO_MISC_ENUMS{
    ZDO_EP                              = 0,    ///< ZDO endpoint number
    NWK_LIST_INFO1_ZIGBEE_VERSION_OFF   = 4,    ///< Network list record format Zigbee version bit offset
    NWK_LIST_INFO2_SUPERFRAME_ORDER_OFF = 4,    ///< Network list record format superframe order bit offset
    ZDO_LQI_INFO1_DEV_TYPE_OFF          = 0,    ///< LQI neighbor table list format - device type bit offset
    ZDO_LQI_INFO1_RX_ON_IDLE_OFF        = 2,    ///< LQI neighbor table list format - rx on idle bit offset
    ZDO_LQI_INFO1_RELATIONSHIP_OFF      = 4,    ///< LQI neighbor table list format - relationship bit offset
    ZDO_LQI_INFO1_DEV_TYPE_MASK         = 0x3,  ///< LQI neighbor table list format - device type field
    ZDO_LQI_INFO1_RX_ON_IDLE_MASK       = 0x3 << ZDO_LQI_INFO1_RX_ON_IDLE_OFF,      ///< LQI neighbor table list format - rx on idle field
    ZDO_LQI_INFO1_RELATIONSHIP_MASK     = 0x7 << ZDO_LQI_INFO1_RELATIONSHIP_OFF,    ///< LQI neighbor table list format - relationship field
    ZDO_LQI_PERMIT_JOIN_MASK            = 0x3,  ///< LQI neighbor table list format - permit join field
    ZDO_LQI_PERMIT_JOIN_UNKNOWN         = 0x2,  ///< LQI neighbor table list format - permit join unknown
    ZDO_LQI_NBOR_ENTRY_SZ               = 0x22, ///< LQI neighbor entry size
    BIND_GRP_ADDR                       = 0x1,  ///< Bind group address enumerated val
    BIND_EXT_ADDR                       = 0x3   ///< Bind extended address enumerated val
};

/*******************************************************************/
/*!
    ZDO Protocol Block. This holds various variables used in the ZDO.
*/
/*******************************************************************/
typedef struct
{
    mem_ptr_t *curr_descr;  ///< This is the descr who we're currently trying to join
    mem_ptr_t *descr_list;  ///< This is the first descr in the pan descr list
} zdo_pcb_t;

/**************************************************************************/
/*!
    This structure holds info that we will need to keep through a remote network
    discovery operation.
*/
/**************************************************************************/
typedef struct _rmt_nwk_disc_t
{
    U8      start_idx;  ///< Remote nwk discovery start index - where we start in the scan descr list
    U8      seq;        ///< Remote nwk discovery sequence number
    U16     src_addr;   ///< Remote nwk discovery source address
} rmt_nwk_disc_t;

/*******************************************************************/
/*!
    Cluster list. This is a list of clusters in the format defined
    by the Zigbee spec. It is used exensively in the requests and
    responses such as the match descriptor request and simple descriptor
    response.
*/
/*******************************************************************/
typedef struct
{
    U8 clust_cnt;       ///< Number of clusters in this list
    U16 clust_list[];   ///< Mark the position of the start of the cluster list
} clust_list_t;

/*******************************************************************/
/*!
    Node descriptor structure. Holds the information contained in the
    Node descriptor.
*/
/*******************************************************************/
typedef struct
{
    U8  node_desc_field1;   ///< Node descriptor byte 1 - logical type, complex desc avail, user desc avail
    U8  node_desc_field2;   ///< Node descriptor byte 2 - APS flags, freq band
    U8  mac_capab_flags;    ///< MAC capability flags
    U16 manuf_code;         ///< Unique manufacturer code allocated by the Zigbee Alliance
    U8  max_buf_size;       ///< Max buf size of the APS data frame
    U16 max_in_xfer;        ///< Max incoming payload size
    U16 server_mask;        ///< Supported server functions
    U16 max_out_xfer;       ///< Max outgoing payload size
    U8  desc_capab;         ///< Extended endpoint or simple descriptor
} node_desc_t;

/*******************************************************************/
/*!
    Simple descriptor. Holds the simple descriptor in the format
    laid out in the Zigbee spec.
*/
/*******************************************************************/
typedef struct
{
    U8  ep;             ///< Endpoint number
    U16 prof_id;        ///< Zigbee profile ID
    U16 dev_id;         ///< Zigbee device ID (as defined by the Zigbee Alliance)
    U8  dev_ver;        ///< Zigbee device version
    U8  clust_info[];   ///< Placeholder for start of in cluster list
} simple_desc_t;


/*******************************************************************/
/*!
    Nwk address request. Used to get the nwk address of a node when
    only its IEEE address is known.
*/
/*******************************************************************/
typedef struct
{
    U64 ext_addr;       ///< Extended address
    U8  req_type;       ///< Only this device's address or all associated device addresses
    U8  start_index;    ///< Start index if multiple device addresses
} nwk_addr_req_t;

/*******************************************************************/
/*!
    IEEE address request. Used to get the IEEE address of a node
    when only its nwk address is known.
*/
/*******************************************************************/
typedef struct
{
    U16 nwk_addr;       ///< Network address of target device
    U8  req_type;       ///< Only this device's address or all associated device addresses
    U8  start_index;    ///< Start index if multiple device addresses
} ieee_addr_req_t;

/*******************************************************************/
/*!
    Node descriptor, Power descriptor, and Active EP request descriptor.
    Holds info contained in these requests. They all share the same
    format so they are grouped together.
*/
/*******************************************************************/
typedef struct _node_req_t
{
    U16 nwk_addr;       ///< Network address of target device
}   node_desc_req_t, pwr_desc_req_t, active_ep_req_t;

/*******************************************************************/
/*!
    Simple descriptor request. Used to request the simple descriptor
    from a remote node.
*/
/*******************************************************************/
typedef struct
{
    U16 nwk_addr;       ///< Network address of target device
    U8  ep;             ///< Endpoint number
} simple_desc_req_t;

/*******************************************************************/
/*!
    Match descriptor request. Used to locate devices with the same
    profile ID and clusters as the sending device.
*/
/*******************************************************************/
typedef struct
{
    U16 nwk_addr;       ///< Network address of target device
    U16 prof_id;        ///< Profile ID
    U8  clust_list[];   ///< Marker for start of cluster list
} match_desc_req_t;

/*******************************************************************/
/*!
    Device announce request. Used to broadcast an announcement that
    a new device has joined the network.
*/
/*******************************************************************/
typedef struct
{
    U16 nwk_addr;       ///< Network address of end device
    U64 ext_addr;       ///< Extended address of end device
    U8  capab_info;     ///< Capability info of end device
} dev_annce_req_t;

/*******************************************************************/
/*!
    Network Discovery Request. Used to request a remote device to
    perform a network discovery and send back the results.
*/
/*******************************************************************/
typedef struct
{
    U32 scan_channels;  ///< Scan channels for network discovery
    U8  scan_duration;  ///< Scan duration for network discovery
    U8  start_index;    ///< Start index for returned network descriptor list
} nwk_disc_req_t;

/*******************************************************************/
/*!
    LQI request. Used to retrieve the contents of a remote node's
    neighbor table.
*/
/*******************************************************************/
typedef struct
{
    U8 start_index;     ///< Start index for returned neighbor list
} nwk_lqi_req_t;

/**************************************************************************/
/*!
        Network leave request. Used by an external device to request this device
        to generate a leave request.
*/
/**************************************************************************/
typedef struct
{
    U64 addr;                           ///< Extended address of device we want to leave
    bool rem_children;          ///< Indicate if we want the device to remove children as well
    bool rejoin;                        ///< Indicate if we want the device to be able to rejoin the network
} nwk_leave_req_t;

/*******************************************************************/
/*!
    NWK Update request. Used to change the operating channel, channel
    mask, or perform an energy scan on a remote node.
*/
/*******************************************************************/
typedef struct
{
    U32 scan_channels;   ///< Updated scan channel mask
    U8  scan_duration;   ///< Updated scan duration
    U8  scan_cnt;        ///< Number of energy scans to be conducted and reported
    U8  nwk_update_id;   ///< Unique ID for this network update request
    U16 nwk_mngr_addr;   ///< Address of the network manager
} nwk_update_req_t;

/*******************************************************************/
/*!
    Permit join request. Used to set the permit join duration on a
    remote node.
*/
/*******************************************************************/
typedef struct
{
    U8 duration;    ///< Duration to permit join
    U8 tc_sig;      ///< Trust center significance
} nwk_permit_join_req_t;

/**************************************************************************/
/*!
        Bind and Unbind request. Used to request a device to add/remove an entry 
        in its binding table. The src addr, ep, and clust will be used by the device
        that holds the binding table. The src addr will always be the address of the
        device that contains the binding table. The endpoint and cluster will be
        used to determine the specific binding entry in the table. The destination
        address and endpoint are the destination values that any frame coming from
        the particular src endpoint and cluster will be sent to.  
*/
/**************************************************************************/
typedef struct _bind_req_t
{
    U64         src_addr;               ///< Source address of device holding the binding table
    U8          src_ep;         ///< Source endpoint of device holding the binding table
    U16         clust;          ///< Cluster ID of device holding the binding table
    address_t   dest_addr;      ///< Destination address to be bound
    U8          dest_ep;        ///< Destination endpoint to be bound
} bind_req_t, unbind_req_t;

/**************************************************************************/
/*!
        End device bind request. Used to request an end device bind operation between
        two devices. End device binding is used to perform binding between two 
        devices when no commissioning device is present.
*/
/**************************************************************************/
typedef struct
{
    U16     target;                             ///< Binding target. The address of this device
    U64     src_ext_addr;       ///< Extended address of device generating request
    U8      src_ep;             ///< Endpoint of device generating request
    U16     prof_id;            ///< Profile ID of device generating request
    U8      num_in_clust;       ///< Number of in clusters in list
    U8      *in_clust;          ///< In cluster list
    U8      num_out_clust;      ///< Number of out clusters in list
    U8      *out_clust;         ///< Out cluster list
} end_dev_bind_req_t;

/*******************************************************************/
/*!
    Main request data structure. This is a generic request structure that
    contains a union of all the ZDO requests.
*/
/*******************************************************************/
typedef struct _zdo_req_t
{
    U8 seq;             ///< Sequence ID of request (the response will later have same ID)
    U16 clust;          ///< Cluster ID of the request
    union               ///< Union of requests for the ZDO
    {
        nwk_addr_req_t          nwk_addr;       ///< Network address request
        ieee_addr_req_t         ieee_addr;      ///< IEEE address request
        node_desc_req_t         node_desc;      ///< Node descriptor request
        pwr_desc_req_t          pwr_desc;       ///< Power descriptor request
        active_ep_req_t         active_ep;      ///< Active endpoint list request
        simple_desc_req_t       simple_desc;    ///< Simple descriptor request
        match_desc_req_t        match_desc;     ///< Match descriptor request
        dev_annce_req_t         dev_annce;      ///< End device announce request
        nwk_disc_req_t          nwk_disc;       ///< Network discovery request
        nwk_lqi_req_t           nwk_lqi;        ///< Network LQI request
        nwk_leave_req_t         leave;          ///< Network Leave request
        nwk_permit_join_req_t   permit_join;    ///< Network permit join request
        nwk_update_req_t        nwk_update;     ///< Network update request
        bind_req_t              bind;           ///< Bind request
        unbind_req_t            unbind;         ///< Unbind request
        end_dev_bind_req_t      ed_bind;        ///< End device bind request
    } type;                                     ///< Type qualifier for the ZDO request
} zdo_req_t;

/*******************************************************************/
/*!
    Address response. Used for both the nwk and IEEE address responses.
    Contains the addresses requested.
*/
/*******************************************************************/
typedef struct
{
    U8  status;             ///< Status of the address request
    U64 ext_addr_rmt;       ///< Remote device's extended address
    U16 nwk_addr_rmt;       ///< Remote device's network address
    U8 assoc_dev_list[];    ///< Remote device's associated device list
} addr_resp_t;

/*******************************************************************/
/*!
    Node descriptor response. Contains the node descriptor from the
    requested node.
*/
/*******************************************************************/
typedef struct
{
    U8          status;     ///< Status of the node descriptor request
    U16         nwk_addr;   ///< Network address of the remote device
    node_desc_t desc;       ///< Node descriptor of remote device
} node_desc_resp_t;

/*******************************************************************/
/*!
    Power descriptor response. Contains the power descriptor from
    the requested node.
*/
/*******************************************************************/
typedef struct
{
    U8  status;     ///< Status of the power descriptor request
    U16 nwk_addr;   ///< Network address of the remote device
    U16 pwr_desc;   ///< Power descriptor of remote device
} pwr_desc_resp_t;

/*******************************************************************/
/*!
    Simple descriptor response. Contains the simple descriptor from
    the requested node.
*/
/*******************************************************************/
typedef struct
{
    U8  status;     ///< Status of simple descriptor request
    U16 nwk_addr;   ///< Network address of the remote device
    U8  len;        ///< Length of simple descriptor
    U8  desc[];     ///< Simple descriptor
} simple_desc_resp_t;

/*******************************************************************/
/*!
    Active EP response. Contains a list of the active endpoints from
    the requested node.
*/
/*******************************************************************/
typedef struct
{
    U8  status;     ///< Status of active endpoint request
    U16 nwk_addr;   ///< Network address of the remote device
    U8  ep_cnt;     ///< Number of endpoints returned
    U8  eps[];      ///< Endpoint list
} active_ep_resp_t;

/*******************************************************************/
/*!
    Match descriptor response. Contains a list of the clusters that
    match the requested profile ID and in/out cluster lists from the
    match request.
*/
/*******************************************************************/
typedef struct
{
    U8  status;     ///< Status of the match descriptor request
    U16 nwk_addr;   ///< Network address of the remote device
    U8  match_len;  ///< Length of the matched cluster list
    U8  ep_list[];  ///< Matched cluster list
} match_desc_resp_t;

/*******************************************************************/
/*!
    NWK discovery response. Contains a list of the PAN descriptors
    from the active scan performed by the requested node.
*/
/*******************************************************************/
typedef struct
{
    U8 status;          ///< Status of the network discovery request
    U8 nwk_cnt;         ///< Number of networks found
    U8 start_index;     ///< Start index of the network list
    U8 nwk_list_cnt;    ///< Number of records in the network list
    U8 nwk_list[];      ///< Network list
} nwk_disc_resp_t;

/*******************************************************************/
/*!
    NWK LQI response. Contains a list of entries from the requested
    node's neighbor table.
*/
/*******************************************************************/
typedef struct
{
    U8 status;           ///< Status of the LQI request
    U8 nbor_tbl_cnt;     ///< Total number of neighbor table entries
    U8 start_index;      ///< Start index of neighbor list
    U8 nbor_list_cnt;    ///< Number of neighbor records in list
    U8 nbor_list[];      ///< Neighbor list
} nwk_lqi_resp_t;

/*******************************************************************/
/*!
    NWK update notify. Contains a list of the energy values from
    the requested energy scan. Also can be used for other network
    maintenance functions.
*/
/*******************************************************************/
typedef struct
{
    U8 status;                   ///< Status of the network update request
    U32 scan_channels;           ///< List of channels scanned
    U16 total_xmits;             ///< Total transmissions recorded by device
    U16 xmit_failures;           ///< Total transmission failured recorded by device
    U8 scan_channel_list_cnt;    ///< Number of values in energy list
    U8 energy_values[MAC_MAX_CHANNELS]; ///< Energy list
} nwk_update_notify_t;

/**************************************************************************/
/*!
        These responses only contain the status.
*/
/**************************************************************************/
typedef struct _bind_resp_t
{
    U8 status;                  ///< Status of the response
} bind_resp_t, ed_bind_resp_t, unbind_resp_t, nwk_permit_join_resp_t, nwk_leave_resp_t;

/*******************************************************************/
/*!
    Main response structure. This is a generic response structure which
    contains a union of all the ZDO response structures implemented.
*/
/*******************************************************************/
typedef struct _zdo_resp_t
{
    U8 seq;                 ///< Sequence ID of request (used to correlate the response to the request)
    union                   ///< Union of responses for the ZDO
    {
        addr_resp_t             addr;           ///< Network/IEEE address response
        node_desc_resp_t        node_desc;      ///< Node descriptor response
        pwr_desc_resp_t         pwr_desc;       ///< Power descriptor response
        simple_desc_resp_t      simple_desc;    ///< Simple descriptor response
        active_ep_resp_t        active_ep;      ///< Active endpoint list response
        match_desc_resp_t       match_desc;     ///< Match descriptor response
        nwk_disc_resp_t         nwk_disc;       ///< Network discovery response
        nwk_lqi_resp_t          nwk_lqi;        ///< Network LQI response
        nwk_leave_resp_t        leave;          ///< Network Leave response
        nwk_permit_join_resp_t  permit_join;    ///< Network permit join response
        nwk_update_notify_t     nwk_update;     ///< Network Update notify
        ed_bind_resp_t          ed_bind;        ///< End device bind response
        bind_resp_t             bind;           ///< Bind response
        unbind_resp_t           unbind;         ///< Unbind response
    } type;                                     ///< Type qualifier for the ZDO response
    U16 clust;              ///< Cluster ID of the response
} zdo_resp_t;

/*******************************************************************/
/*!
    ZDO handler entry. This is used to generate the ZDO handler table
    which is used by the ZDO Rx handler to route the incoming data. The
    data is routed based on the cluster ID.
*/
/*******************************************************************/
typedef struct
{
    U16 clust_id;   ///< Cluster ID
    void (*handler)(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);  ///< Cluster handler function pointer
} zdo_handler_t;

/*******************************************************************/
/*!
    Network list. This is a structure that is used by the nwk discovery
    response to format the items being returned to the requestor.
*/
/*******************************************************************/
typedef struct
{
    U64 ext_pan_id;         ///< Extended PAN ID
    U8  logical_channel;    ///< Channel
    U8  nwk_disc_info1;     ///< Byte field 1 - stack profile, Zigbee version
    U8  nwk_disc_info2;     ///< Byte field 2 - beacon order, superframe order
    U8  permit_join;        ///< At least one router on the network permits joining
} zdo_nwk_list_t;

/**************************************************************************/
/*!
        Device structure for the end device request operation. This holds the
        info required for a device to perform an end device bind.
*/
/**************************************************************************/
typedef struct
{
    zdo_req_t req;                  ///< ZDO ED bind request info
    U8 in_clust[ZDO_MAX_CLUST];     ///< In cluster list for device
    U8 out_clust[ZDO_MAX_CLUST];    ///< Out cluster list for device
    U8 match_list[ZDO_MAX_CLUST];   ///< Matched clusters list
    U8 num_matches;                 ///< Number of matched clusters
    U16 src_addr;                   ///< Source address of this device
    U64 src_ext_addr;               ///< Source extended address of this device
    U8 src_ep;                      ///< Source endpoint for this device
} ed_bind_dev_t;

/**************************************************************************/
/*!
        Main structure for end device binding. This holds the info for two 
        devices and state information.
*/
/**************************************************************************/
typedef struct
{
    ed_bind_dev_t dev0;     ///< End device bind device info for device 0
    ed_bind_dev_t dev1;     ///< End device bind device info for device 1
    U8 curr_dev;            ///< Current device that is being processed
    U8 curr_index;          ///< Current index that is being processed
    bool bind;              ///< Bind flag - True for bind process, false for unbind process
} ed_bind_t;

/*******************************************************************/
/*!
    Neighbor item. This is a neighbor entry formatted according to the
    Zigbee spec for the LQI response.
*/
/*******************************************************************/
typedef struct
{
    U64 ext_pan_id;     ///< Extended PAN ID of neighbor
    U64 ext_addr;       ///< Extended address of neighbor
    U16 nwk_addr;       ///< Network address of neighbor
    U8 zdo_nbor_info1;  ///< Byte field 1 - dev type, rx on idle, relationship of neighbor
    U8 permit_join;     ///< Permit join status of neighbor
    U8 depth;           ///< Network depth of neighbor
    U8 lqi;             ///< LQI of neighbor
} zdo_nbor_item_t;

/**************************************************************************/
/*!
    ZDO callback structure. The application needs to register these callbacks
    so they get sent to the appropriate handler.
*/
/**************************************************************************/
typedef struct _zdo_cb_t
{
    void (*nwk_form_conf)(U8 status);               ///< Nwk formation confirm callback
    void (*nwk_join_conf)(U8 status, U16 nwk_addr, U16 parent_addr);    ///< Nwk join confirm callback
    void (*nwk_join_ind)(U16 nwk_addr);             ///< Nwk join indication callback
    void (*nwk_leave_conf)(U64 addr, U8 status);    ///< Nwk leave confirm callback
    void (*nwk_leave_ind)(U64 addr, bool rejoin);   ///< Nwk leave indication callback
} zdo_cb_t;

PROCESS_NAME(zdo_process);  ///< This is the main process for the ZDO layer

// zdo
void zdo_init();
zdo_cb_t *zdo_cb_get();
zdo_pcb_t *zdo_pcb_get();
U8 zdo_seq_get();
void zdo_tx(U8 *data, U8 len, U16 dest_addr, U16 clust_id, U8 tx_opt, U8 handle);
void zdo_rx_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id);
void zdo_conf_handler(U8 status, U8 handle);
void zdo_reg_cb(zdo_cb_t *cb);

// zdo_parse
U8 zdo_parse_req(U16 addr, U8 *data_in, U16 clust, zdo_req_t *req);
U8 zdo_parse_resp(U8 *data_in, U16 clust, zdo_resp_t *resp);

// zdo_gen
U8 zdo_gen_req(U16 dest_addr, zdo_req_t *params);

// zdo nwk manager
void zdo_nwk_start();
void nwk_disc_conf(U8 status, mem_ptr_t *mem_ptr);
void nwk_join_conf(U8 status, U16 nwk_addr, U64 ext_pan_id, U8 channel);
void nwk_form_conf(U8 status);
void nwk_leave_conf(address_t *addr, U8 status);
void nwk_leave_ind(address_t *addr, bool rejoin);
void nwk_join_ind(U64 ext_addr, U16 nwk_addr);

void zdo_nwk_disc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_lqi_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_update_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_permit_join_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_lqi_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_disc_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_update_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_leave_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_leave_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_nwk_mgr_periodic();

// zdo_cfg
void zdo_cfg_init();
node_desc_t *zdo_cfg_node_desc_get();
U16 *zdo_cfg_pwr_desc_get();
void zdo_cfg_node_desc_set();
void zdo_cfg_pwr_desc_set();
void zdo_cfg_user_desc_set(char *msg);
void zdo_cfg_transaction_persist(U8 time);
void zdo_cfg_nwk_scan_attempts_set(U8 scan_attempts);
void zdo_cfg_nwk_time_betw_scans_set(U16 time);
void zdo_cfg_nwk_permit_join_set(U8 duration);

// zdo_disc
void zdo_disc_addr_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_node_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_pwr_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_simple_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_active_ep_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_match_desc_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_dev_annce_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_disc_addr_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);

// zdo_bind_mgr
void zdo_bind_mgr_init();
void zdo_bind_ed_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_bind_unbind_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_bind_ed_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
void zdo_bind_unbind_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust);
#endif


