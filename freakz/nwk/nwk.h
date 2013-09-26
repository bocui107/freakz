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
    \file nwk.h
    \ingroup nwk
    \brief Network layer header file

    This is the header file for the NWK layer. It defines the constants
    data structures, and prototypes used in the NWK layer.
*/
/*******************************************************************/
#ifndef NWK_H
#define NWK_H

#include "types.h"
#include "mac.h"

#define NWK_PEND_TIMEOUT                    3   ///< Pending entry timeout value

// this define is just used to make the code more comprehensible. otherwise,
// you'd have to stare at these monsters all over the place.
#define ADDR_MAP_ENTRY(m)   ((nwk_addr_map_t *)MMEM_PTR(&m->mmem_ptr))   ///< De-reference the mem ptr and cast it as an addr map entry
#define BRC_ENTRY(m)        ((nwk_brc_t *)MMEM_PTR(&m->mmem_ptr))        ///< De-reference the mem ptr and cast it as a broadcast table entry
#define NBOR_ENTRY(m)       ((nbor_tbl_entry_t *)MMEM_PTR(&m->mmem_ptr)) ///< De-reference the mem ptr and cast it as a neighbor table entry
#define PEND_ENTRY(m)       ((nwk_pend_t *)MMEM_PTR(&m->mmem_ptr))       ///< De-reference the mem ptr and cast it as a pending list entry
#define DISC_ENTRY(m)       ((disc_entry_t *)MMEM_PTR(&m->mmem_ptr))     ///< De-reference the mem ptr and cast it as a discovery table entry
#define RTE_ENTRY(m)        ((rte_entry_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as a route table entry
#define RREQ_ENTRY(m)       ((rreq_t *)MMEM_PTR(&m->mmem_ptr))          ///< De-reference the mem ptr and cast it as a rte request entry

/****************************************************************/
/*!
 Enumerated definitions for network layer command frames
*/
/****************************************************************/
enum NWK_CMD_FRAME_ENUMS
{
    // nwk command id's
    NWK_CMD_RTE_REQ                 = 0x1,      ///< Route request command ID
    NWK_CMD_RTE_REP                 = 0x2,      ///< Route reply command ID
    NWK_CMD_NWK_STATUS              = 0x3,      ///< Network status command ID
    NWK_CMD_LEAVE                   = 0x4,      ///< Network leave command ID
    NWK_CMD_RTE_REC                 = 0x5,      ///< Route record command ID
    NWK_CMD_REJOIN_REQ              = 0x6,      ///< Rejoin request command ID
    NWK_CMD_REJOIN_RESP             = 0x7,      ///< Rejoin response command ID
    NWK_CMD_LINK_STATUS             = 0x8,      ///< Link status command ID
    NWK_CMD_NWK_REPORT              = 0x9,      ///< Network report command ID
    NWK_CMD_NWK_UPDATE              = 0xa,      ///< Network update command ID

    // nwk command payload sizes
    NWK_CMD_SIZE_RTE_REQ            = 6,        ///< Route request frame size
    NWK_CMD_SIZE_RTE_REP            = 8,        ///< Route reply frame size
    NWK_CMD_SIZE_STATUS             = 4,        ///< Network status frame size
    NWK_CMD_SIZE_LEAVE              = 2,        ///< Network leave frame size
    NWK_CMD_SIZE_RTE_REC            = 2,        ///< Route record frame size
    NWK_CMD_SIZE_REJOIN_REQ         = 2,        ///< Rejoin request frame size
    NWK_CMD_SIZE_REJOIN_RESP        = 4,        ///< Rejoin response frame size
    NWK_CMD_SIZE_LINK_STATUS        = 2,        ///< Network link status frame size
    NWK_CMD_SIZE_NWK_REPORT         = 10,       ///< Network report frame size
    NWK_CMD_SIZE_NWK_UPDATE         = 11,       ///< Network update frame size
};

/****************************************************************/
/*!
    Enumerated definitions for the network frame control field in the network header
*/
/****************************************************************/
enum NWK_FRAME_CTRL_ENUMS
{
    // nwk frame control field bit offsets
    NWK_PROT_VER_OFF                = 2,        ///< Protocol version bit offset in FCF
    NWK_DISC_RTE_OFF                = 6,        ///< Discover route bit offset in FCF
    NWK_MCAST_FLAG_OFF              = 8,        ///< Multicast flag bit offset in FCF
    NWK_SEC_OFF                     = 9,        ///< Security bit offset in FCF
    NWK_SRC_RTE_OFF                 = 10,       ///< Source route bit offset in FCF
    NWK_DEST_IEEE_ADDR_OFF          = 11,       ///< Dest IEEE address flag bit offset in FCF
    NWK_SRC_IEEE_ADDR_OFF           = 12,       ///< Src IEEE address flag bit offset in FCF

    // nwk frame type
    NWK_DATA_FRM                    = 0x0,      ///< Data frame type
    NWK_CMD_FRM                     = 0x1,      ///< Command frame type

    // nwk route discovery
    NWK_SUPRESS_RTE_DISC            = 0x0,      ///< Route discovery not allowed for this frame
    NWK_ENB_RTE_DISC                = 0x1,      ///< Route discovery enabled
};

/****************************************************************/
/*!
    Enumerated definitions used in the network tables
*/
/****************************************************************/
enum NWK_TABLE_ENUMS
{
    // nwk device type
    NWK_COORDINATOR                 = 0x0,      ///< This device is a network coordinator
    NWK_ROUTER                      = 0x1,      ///< This device is a router
    NWK_END_DEVICE                  = 0x2,      ///< This device is an end device
    NWK_INVALID_DEVICE              = 0x3,      ///< Invalid device

    // nwk relationship
    NWK_PARENT                      = 0x0,      ///< This neighbor is a parent of this device
    NWK_CHILD                       = 0x1,      ///< This neighbor is a child of this device
    NWK_SIBLING                     = 0x2,      ///< This neighbor is a sibling of this device
    NWK_NONE                        = 0x3,      ///< This neighbor has no relationship to this device
    NWK_PREV_CHILD                  = 0x4,      ///< This neighbor is a previous child of this device

    // nwk routing status
    NWK_ACTIVE                      = 0x0,      ///< This route entry is active
    NWK_DISCOVERY_UNDERWAY          = 0x1,      ///< This route entry is currently being discovered
    NWK_DISCOVERY_FAILED            = 0x2,      ///< This route entry has failed route discovery
    NWK_INACTIVE                    = 0x3,      ///< This route entry is inactive
    NWK_VALIDATION_UNDERWAY         = 0x4,      ///< This route entry is being validated
};

/****************************************************************/
/*!
    Enumerated definitions for network command frames
*/
/****************************************************************/
enum NWK_CMD_FRM_ENUMS
{
    // nwk join definitions
    NWK_JOIN_NORMAL                 = 0x00,     ///< A standard network join has been requested
    NWK_JOIN_DIRECT_ORPHAN          = 0x01,     ///< Do a direct join on the orphan device
    NWK_JOIN_REJOIN                 = 0x02,     ///< A device has requested to rejoin

    // network capability info
    NWK_CAPAB_DEV_TYPE_OFFSET       = 1,
    NWK_CAPAB_POWER_SRC_OFFSET      = 2,
    NWK_CAPAB_RX_ON_IDLE_OFFSET     = 3,
    NWK_CAPAB_SECURITY_OFFSET       = 6,
    NWK_CAPAB_ALLOC_ADDR_OFFSET     = 7,
    NWK_CAPAB_DEV_TYPE_MASK         = (1 << NWK_CAPAB_DEV_TYPE_OFFSET),     ///< Capability info - device type field
    NWK_CAPAB_POWER_SRC_MASK        = (1 << NWK_CAPAB_POWER_SRC_OFFSET),    ///< Capability info - power source type field
    NWK_CAPAB_RX_ON_WHEN_IDLE_MASK  = (1 << NWK_CAPAB_RX_ON_IDLE_OFFSET),   ///< Capability info - receiver always on field
    NWK_CAPAB_SECURITY_MASK         = (1 << NWK_CAPAB_SECURITY_OFFSET),     ///< Capability info - security enabled field
    NWK_CAPAB_ALLOC_ADDR_MASK       = (1 << NWK_CAPAB_ALLOC_ADDR_OFFSET),   ///< Capability info - address allocation type field

    // network leave fields
    NWK_LEAVE_REJOIN_OFFSET         = 5,
    NWK_LEAVE_REQUEST_OFFSET        = 6,
    NWK_LEAVE_REM_CHILDREN_OFFSET   = 7,

    // nwk permit join defines
    NWK_PERMIT_JOIN_DISABLE         = 0x00,     ///< Network permit join - permanently disable joining
    NWK_PERMIT_JOIN_ENABLE          = 0xFF,     ///< Network permit join - permanently enable joining
};

/****************************************************************/
/*!
    Enumerated definitions for network beacon info
*/
/****************************************************************/
enum NWK_BEACON_ENUMS
{
    // nwk beacon frame
    NWK_STACK_PROFILE_MASK                  = 0x0F,     ///< Field for this device's stack profile
    NWK_PROTOCOL_VERSION_MASK       = 0xF0,     ///< Field for this device's protocol version
    NWK_ROUTER_CAPACITY_MASK        = 0x04,     ///< Field for this device has capacity for routers to join
    NWK_DEVICE_DEPTH_MASK                   = 0x78,     ///< Field for this device's network depth
    NWK_END_DEVICE_CAPACITY_MASK    = 0x80,     ///< Field for this device has capacity for end devices to join

    NWK_PROTOCOL_VERSION_OFF        = 0x4,
    NWK_ROUTER_CAPACITY_OFF         = 0x2,
    NWK_DEVICE_DEPTH_OFF            = 0x3,
    NWK_END_DEVICE_CAPACITY_OFF     = 0x7,

    NWK_BEACON_PAYLOAD_SIZE         = 0xB,
};

/****************************************************************/
/*!
    Enumerated definitions for the network status values
*/
/****************************************************************/
enum NWK_STATUS_ENUMS
{
    // nwk status
    NWK_SUCCESS                     = 0x00, ///< A request has been executed successfully
    NWK_INVALID_PARAMETER           = 0xC1, ///< Invalid argument passed to function
    NWK_INVALID_REQUEST             = 0xC2, ///< Upper layer requested service that cannot be performed
    NWK_NOT_PERMITTED               = 0xC3, ///< Join request not allowed
    NWK_STARTUP_FAILURE             = 0xC4, ///< Network formation request failed
    NWK_ALREADY_PRESENT             = 0xC5, ///< Device requesting direct join is already in nbor tbl
    NWK_SYNC_FAILURE                = 0xC6, ///< Sync request has failed
    NWK_TABLE_FULL                  = 0xC7, ///< Join request failed because nbor table full
    NWK_UNKNOWN_DEVICE              = 0xC8, ///< Leave request failed because device not in nbor tbl
    NWK_UNSUPPORTED_ATTRIBUTE       = 0xC9, ///< NIB request failed get/set
    NWK_NO_NETWORKS                 = 0xCA, ///< Join request issued where no networks available
    NWK_MAX_FRM_CNTR                = 0xCC, ///< Security failed because frame counter hit max val
    NWK_NO_KEY                      = 0xCD, ///< Security failed because no key available
    NWK_BAD_CCM_OUTPUT              = 0xCE, ///< Security failed because security engine error
    NWK_NO_ROUTING_CAPACITY         = 0xCF, ///< Discover route failed due to lack of routing or disc tbl entries
    NWK_ROUTE_DISC_FAILED           = 0xD0, ///< Discover route failed due to any other reason
    NWK_ROUTE_ERROR                 = 0xD1, ///< Data transmit request failed due to routing failure on other device
    NWK_BT_TBL_FULL                 = 0xD2, ///< Broadcast failed due to no room in broadcast table
    NWK_FRAME_NOT_BUFFERED          = 0xD3  ///< Data transmit request failed due to lack of pending entries
};

/****************************************************************/
/*!
    Miscellaneous enumerated definitions used in the network layer
*/
/****************************************************************/
enum NWK_MISC_ENUMS
{
    // nwk tx dest addr mode
    NWK_TX_MCAST_ADDR               = 0x1,      ///< Network data request transmit multicast
    NWK_TX_UNICAST_BROADCAST        = 0x2,      ///< Network data request transmit unicast/broadcast

    // address allocation
    NWK_ADDR_ALLOC_TREE             = 0x0,      ///< Use tree addressing for addr allocation
    NWK_ADDR_ALLOC_STOCHASTIC       = 0x2,      ///< Use stochastic addressing for addr allocation

    // default non member radius value
    NWK_NON_MEMB_RADIUS_INFIN       = 0x7,      ///< Constant for infinite nonmember radius

    // default static path cost
    NWK_STATIC_PATH_COST            = 0x7,      ///< Constant for path cost. \todo This needs to be changed.

    // network broadcast values
    NWK_BROADCAST_ALL               = 0xFFFF,   ///< Broadcast address for all devices
    NWK_BROADCAST_RXONIDLE          = 0xFFFD,   ///< Broadcast address for devices whose receiver is always on
    NWK_BROADCAST_ROUTERS_COORD     = 0xFFFC,   ///< Broadcast address for only routers and coordinators
    NWK_BROADCAST_MASK              = 0xFFF0,   ///< Broadcast address mask - used to identify any type of brc address

    // nlme states
    NLME_IDLE                       = 0x0,      ///< Network management state machine - idle state
    NLME_NWK_DISC                   = 0x1,      ///< Network management state machine - network discovery state
    NLME_NWK_FORMATION              = 0x2       ///< Network management state machine - network formation state
};

/*******************************************************************/
/*!
    Holds the data to assemble the frame control field.
*/
/*******************************************************************/
typedef struct
{
    U8      frame_type;             ///< Frame type of this frame
    U8      protocol_ver;           ///< Protocol version of this frame
    U8      disc_route;             ///< Discover route enabled
    bool    mcast_flag;             ///< Multicast enabled
    bool    security;               ///< Security enabled
    bool    src_rte;                ///< Source route enabled
    bool    dest_ieee_addr_flag;    ///< Dest IEEE addr present
    bool    src_ieee_addr_flag;     ///< Src IEEE addr present
} nwk_frm_ctrl_t;

/*******************************************************************/
/*!
    The main network header structure. The NWK header is assembled from
    the info in this structure on outgoing frames. It holds the extracted
    information about the NWK header on incoming frames.
*/
/*******************************************************************/
typedef struct
{
    mac_hdr_t       *mac_hdr;           ///< Pointer to MAC header
    nwk_frm_ctrl_t  nwk_frm_ctrl;       ///< Network frame control data structure
    U16             nwk_fcf;            ///< Network frame control hex value
    U16             dest_addr;          ///< Destination network address
    U16             src_addr;           ///< Source network address
    U8              radius;             ///< Max number of hops this frame can travel
    U8              seq_num;            ///< Sequence number identifier for this frame
    U64             dest_ieee_addr;     ///< Dest IEEE address - if dest ieee addr flag set
    U64             src_ieee_addr;      ///< Src IEEE address - if src ieee addr flag set
    U8              mcast_ctrl;         ///< Multicast control field
    U8              handle;             ///< Data handle identifier
    U8              *src_rte_subfrm;    ///< Source route subframe - holds source route hop addresses
} nwk_hdr_t;

/*******************************************************************/
/*!
    Data struct to hold the route request command frame arguments.
*/
/*******************************************************************/
typedef struct
{
    U8  cmd_opts;   ///< Route request command options
    U8  rreq_id;    ///< Route request unique identifier
    U16 dest_addr;  ///< Final destination being searched for
    U8  path_cost;  ///< Route request total path cost
} nwk_cmd_rreq_t;

/*******************************************************************/
/*!
    Data struct to hold info the route reply command frame arguments.
*/
/*******************************************************************/
typedef struct
{
    U8  cmd_opts;   ///< Route reply command options
    U8  rreq_id;    ///< Route reply unique identifier
    U16 originator; ///< Route request originator (source of route request)
    U16 responder;  ///< Route reply responder (destination of route request)
    U8  path_cost;  ///< Route reply total path cost back to source node
} nwk_cmd_rrep_t;

/*******************************************************************/
/*!
    Data struct to hold the NWK leave command frame args.
*/
/*******************************************************************/
typedef struct
{
    U8 cmd_opts;    ///< Leave command options
} nwk_cmd_leave_t;

/*******************************************************************/
/*!
    Data struct to hold the rejoin request command frame args.
*/
/*******************************************************************/
typedef struct
{
    U8 capab_info;  ///< Capability info
} nwk_cmd_rejoin_req_t;

/*******************************************************************/
/*!
    Data struct to hold the rejoin response command frame args.
*/
/*******************************************************************/
typedef struct
{
    U16 nwk_addr;   ///< Network address for device requesting rejoin
    U8 status;      ///< Rejoin status
} nwk_cmd_rejoin_resp_t;

/*******************************************************************/
/*!
    This is a command frame container. Its a union of all the command
    frame data structures. The member that this struct refers to is
    identified by the command frame ID.
*/
/*******************************************************************/
typedef struct
{
    U8 cmd_frm_id;                              ///< NWK cmd frame command ID
    union
    {
        nwk_cmd_rreq_t          rreq;           ///< Route request cmd frame
        nwk_cmd_rrep_t          rrep;           ///< Route reply cmd frame
        nwk_cmd_leave_t         leave;          ///< Network leave cmd frame
        nwk_cmd_rejoin_req_t    rejoin_req;     ///< Network rejoin request cmd frame
        nwk_cmd_rejoin_resp_t   rejoin_resp;    ///< Network rejoin response cmd frame
    };
} nwk_cmd_t;

/*******************************************************************/
/*!
    Route request entry which is to be used in the route request list.
    It holds information about route discoveries in progress.
*/
/*******************************************************************/
typedef struct _rreq_t
{
    U8              retries;    ///< Number of retries for the route request
    U8              radius;     ///< Radius of the rreq
    U16             originator; ///< Originator of the rreq
    nwk_cmd_t       cmd;        ///< CMD data for the rreq
    U16             expiry;     ///< Expiration of the route request
} rreq_t;

/*******************************************************************/
/*!
    Routing table entry. Used to look up a route to the
    specified destination address. The routing table will only
    reveal the next hop towards the destination.
*/
/*******************************************************************/
typedef struct _rte_tbl_entry_t
{
    U16     dest_addr;              ///< Dest address of the route
    U8      status;                 ///< Indicator of the health of the route
    U16     next_hop;               ///< Next hop towards the destination
} rte_entry_t;

/*******************************************************************/
/*!
    Route discovery table entry. The discovery table is used to keep
    track of route request information during a route discovery operation.
*/
/*******************************************************************/
typedef struct _rte_disc_tbl_entry_t
{
    U8      rreq_id;                    ///< Route request ID that this entry is tracking
    U16     src_addr;                   ///< Src addr of route request
    U16     sender_addr;                ///< Address of previous hop node that relayed this route request
    U16     dest_addr;                  ///< Dest addr of this route request
    U8      fwd_cost;                   ///< Forward path cost of this route from source to this node
    U8      resid_cost;                 ///< Reverse path cost of this route from dest to this node
    U16     expiry;                     ///< Amount of time left before this entry expires
} disc_entry_t;

/*******************************************************************/
/*!
    Neighbor table entry. The neighbor table records all nodes within
    listening distance.
*/
/*******************************************************************/
typedef struct _nbor_tbl_entry_t
{
    U64     ext_addr;           ///< Extended (IEEE) address of neighbor
    U64     ext_pan_id;         ///< Extended PAN ID of the neighbor
    U16     nwk_addr;           ///< Short (network) address of neighbor
    U16     pan_id;             ///< Short PAN ID
    U8      device_type;        ///< Device type of neighbor
    bool    rx_on_when_idle;    ///< Neighbor's receiver always on
    U8      relationship;       ///< Relationship with this neighbor - parent, child, sibling, none
    U8      channel;            ///< Logical channel that the neighbor is on
    U8      depth;              ///< Network depth of the neighbor
    bool    permit_join;        ///< Whether the neighbor is allowing joins
    bool    pot_parent;         ///< Whether the neighbor is a potential parent
    U8      transmit_fail;      ///< Number of failed transmissions to this neighbor
    U8      lqi;                ///< Link quality indicator value - can be used to determine path cost
} nbor_tbl_entry_t;

/*******************************************************************/
/*!
    NWK Information Base. Holds all the configuration information
    for the NWK layer.
*/
/*******************************************************************/
typedef struct _nwk_nib_t
{
    U8                  seq_num;                ///< Current sequence number
    U8                  max_children;           ///< Max children this device will support (used to determine tree addr allocation)
    U8                  max_depth;              ///< Max network depth this device will support (used to determine tree addr allocation)
    U8                  max_routers;            ///< Max routers this device will support (used to determine tree addr allocation)
    bool                report_const_cost;      ///< Use constant path cost for all links
    bool                sym_link;               ///< Enable symmetric links
    U8                  capability_info;        ///< Capability info for this device
    U8                  traxn_persist_time;     ///< Length of time before an indirect transaction is expired
    U16                 short_addr;             ///< Network address for this device
    U8                  stack_profile;          ///< Stack profile (Zigbee or Zigbee pro)
    U64                 ext_pan_ID;             ///< Extended PAN ID
    U8                  depth;                  ///< Device depth
    U8                  rtr_cap;                ///< Remaining available addresses for routers to join
    U8                  ed_cap;                 ///< Remaining available addreses for end devices to join
    U16                 cskip;                  ///< CSkip value for tree addressing
    U8                  dev_type;               ///< Device type (coordinator, router, end device)
    U8                  rreq_id;                ///< Current route request ID number (used to select the rreq ID)
    bool                joined;                 ///< Joined to the network
    U8                  nwk_scan_attempts;      ///< Number of scan attempts
    U16                 time_betw_scans;        ///< Time between scan attempts
    U16                 nwk_mngr;               ///< Address of this network's network manager
} nwk_nib_t;

/*******************************************************************/
/*!
    Capability information. Shows the capabilities of this node. It is
    used when requesting to join a parent.
*/
/*******************************************************************/
typedef struct
{
    bool dev_type;      ///< Device type
    bool pwr_src;       ///< Power source (Mains, battery)
    bool rx_on_idle;    ///< Receive always on
    bool security;      ///< Security enabled
    bool alloc_addr;    ///< Allocate address
} nwk_capab_info_t;

/*******************************************************************/
/*!
    Data request service arguments. This is filled out and passed
    to the NWK data request service. Most of the information will
    be used to generate the NWK header and put it in the specified
    buffer.
*/
/*******************************************************************/
typedef struct
{
    U8          nwk_dest_addr_mode;     ///< Dest address mode
    U16         dest_addr;              ///< Dest short address
    buffer_t    *buf;                   ///< APS data frame
    U8          nsdu_handle;            ///< Data handle identifier
    U8          radius;                 ///< Max number of hops for this frame
    U8          non_memb_radius;        ///< Max number of hops if multicast and non-member
    bool        disc_rte;               ///< Discover route allowed for this frame
    bool        security_enb;           ///< Security enabled for this frame
} nwk_data_req_t;

/*******************************************************************/
/*!
    Join request arguments. This is filled out and passed to the
    NWK join request service. It will be used to generate an
    association request command frame when requesting to join a
    parent node.
*/
/*******************************************************************/
typedef struct
{
    U64             ext_pan_id;     ///< Extended PAN ID
    bool            join_as_rtr;    ///< Join network as a router
    U8              rejoin_nwk;     ///< Rejoin network
    mem_ptr_t       *desc;          ///< PAN descriptor
} nwk_join_req_t;

/*******************************************************************/
/*!
    Pending queue entry. The pending queue is used to hold data
    frames that are awaiting a route discovery operation to be finished
    before they can be forwarded.
*/
/*******************************************************************/
typedef struct _nwk_pend_t
{
    nwk_frm_ctrl_t      frm_ctrl;   ///< Frame control field for this frame
    U16                 src_addr;   ///< Source address
    U16                 dest_addr;  ///< Dest address
    U8                  radius;     ///< Max number of hops
    U8                  seq;        ///< Sequence number
    U8                  handle;     ///< Data handle for this frame
    U8                  expiry;     ///< Time remaining before expiration for this entry
    buffer_t            *buf;       ///< Frame to be transmitted
} nwk_pend_t;

/*******************************************************************/
/*!
    Broadcast table. The broadcast table is used to implement the
    passive ACK system. Passive ACK'ing just means that after you send
    out the broadcast, you collect the relayed broadcasts from all
    neighbors to make sure that everyone has received the frame.
*/
/*******************************************************************/
typedef struct _nwk_brc_t
{
    U16                 nwk_addr;   ///< Network address of received broadcast frame
    U8                  seq_id;     ///< Sequence ID of received broadcast frame
    U8                  expiry;     ///< Time remaining before expiring this entry
} nwk_brc_t;

/*******************************************************************/
/*!
    Address Map. Given a nwk address, the extended address can be
    looked up using the address map. And vice versa.
*/
/*******************************************************************/
typedef struct _nwk_addr_map_t
{
    U64                     ext_addr;   ///< Node's extended address
    U16                     nwk_addr;   ///< Node's network address
    U8                      capab;      ///< Node's capability info
} nwk_addr_map_t;

/*******************************************************************/
/*!
    Network Protocol Block. This holds miscellaneous variables needed
    by the NWK layer.
*/
/*******************************************************************/
typedef struct _nwk_pcb_t
{
    // these are to handle broadcasts
    bool            brc_active;         ///< Broadcast in progress
    bool            brc_accept_new;     ///< Okay to accept new broadcasts
    U8              brc_retries;        ///< Number of retries remaining for broadcast frame
    U8              brc_seq;            ///< Sequence ID of current broadcast
    nwk_hdr_t       brc_nwk_hdr;        ///< Broadcast frame network header
    buffer_t        *brc_curr_frm;      ///< Keep broadcast frame here for retransmission until retries are finished
    struct ctimer   brc_tmr;            ///< Callback timer for broadcast

    // nwk discovery
    U32             channel_mask;       ///< Channel mask for network scan
    U8              duration;           ///< Duration for network scan

    // these are to handle network management
    U8              nlme_state;         ///< Network management state
    U8              *energy_list;       ///< Pointer to MAC layer energy list

    // nwk join
    mem_ptr_t       *curr_join_target;  ///< Current parent we are trying to join
    bool            join_as_rtr;        ///< Joining as router or end device

    // statistics (not implemented yet)
    U16 drop_data_frm;                  ///< Number of dropped data frames
    U16 drop_brc_frm;                   ///< Number of dropped broadcast frames
    U16 drop_rreq_frm;                  ///< Number of dropped route request frames
    U16 drop_rrep_frm;                  ///< Number of dropped route reply frames
    U16 failed_transfer;                ///< Number of failed transfers
    U16 failed_tree_rte;                ///< Number of failed tree routes
    U16 failed_alloc;                   ///< Number of failed buffer allocations
} nwk_pcb_t;

/*****************************************************************/
// Prototypes

// nwk
void nwk_init();
nwk_pcb_t *nwk_pcb_get();
nwk_nib_t *nwk_nib_get();
void nwk_data_req(const nwk_data_req_t *req);
void nwk_fwd(buffer_t *buf, nwk_hdr_t *hdr_in);
void mac_data_ind(buffer_t *buf, mac_hdr_t *mac_hdr);
void mac_data_conf(U8 status, U8 msdu_handle);
void mac_poll_conf(U8 status);
void mac_comm_status_ind(U16 pan_id, address_t dest_addr, U8 status, U8 handle);

// nwk_parse
void nwk_parse_hdr(buffer_t *buf, nwk_hdr_t *hdr);
void nwk_parse_cmd(buffer_t *buf, nwk_cmd_t *cmd);
void nwk_parse_beacon(buffer_t *buf, mem_ptr_t *mem_ptr);

// nwk_gen
U16 nwk_gen_frm_ctrl(const nwk_hdr_t *hdr);
void nwk_gen_header(buffer_t *buf, nwk_hdr_t *hdr);
void nwk_gen_beacon(buffer_t *buf);
void nwk_gen_cmd(buffer_t *buf, const nwk_cmd_t *cmd);
U8 nwk_gen_capab_info(const nwk_capab_info_t *info);

// nwk_rte (routing table)
void nwk_rte_tbl_init();
mem_ptr_t *nwk_rte_tbl_find(U16 dest_addr);
void nwk_rte_tbl_add_new(U16 dest_addr, U8 status);
void nwk_rte_tbl_free(mem_ptr_t *mem_ptr);
U16 nwk_rte_tbl_get_next_hop(U16 dest_addr);
bool nwk_rte_tbl_rte_exists(U16 dest_addr);
void nwk_rte_tbl_rem(U16 addr);
void nwk_rte_tbl_clear();
mem_ptr_t *nwk_rte_tbl_get_head();

// nwk_rte_disc (discovery table)
void nwk_rte_disc_tbl_init();
void nwk_rte_disc_free(mem_ptr_t *mem_ptr);
void nwk_rte_disc_clear();
mem_ptr_t *nwk_rte_disc_find(U8 rreq_id, U16 src_addr);
void nwk_rte_disc_add_new(U8 rreq_id, U16 src_addr, U16 sender_addr, U16 dest_addr, U8 cost);
void nwk_rte_disc_periodic();

// nwk_rte_mesh (mesh routing)
void nwk_rte_mesh_init();
void nwk_rte_mesh_rreq_handler(const nwk_hdr_t *hdr_in, nwk_cmd_t *cmd_in);
void nwk_rte_mesh_rrep_handler(const nwk_hdr_t *hdr_in, const nwk_cmd_t *cmd_in);
void nwk_rte_mesh_disc_start(U16 dest_addr);
void nwk_rte_mesh_resend_rreq(mem_ptr_t *mem_ptr);
void nwk_rte_mesh_periodic(void *ptr);
void nwk_rte_mesh_stop_tmr();

// nwk_rte_tree (tree routing)
void nwk_rte_tree_init();
U16 nwk_rte_tree_calc_next_hop(U16 dest);
U16 nwk_rte_tree_calc_cskip(U8 depth);
U16 nwk_rte_tree_calc_rtr_addr(U16 cskip, U8 num);
U16 nwk_rte_tree_calc_ed_addr(U16 cskip, U8 num);

// nwk_pend (pending queue)
void nwk_pend_init();
void nwk_pend_add_new(buffer_t *buf, nwk_hdr_t *hdr);
void nwk_pend_free(mem_ptr_t *mem_ptr);
void nwk_pend_send_pending();
void nwk_pend_clear();
void nwk_pend_periodic();

// nwk_neighbor_tbl (neighbor table)
void nwk_neighbor_tbl_init();
void nwk_neighbor_tbl_clear();
mem_ptr_t *nwk_neighbor_tbl_get_head();
void nwk_neighbor_tbl_add(nbor_tbl_entry_t *entry);
void nwk_neighbor_tbl_rem(address_t *addr);
U16 nwk_neighbor_tbl_get_parent();
bool nwk_neighbor_tbl_addr_exists(address_t *addr);
mem_ptr_t *nwk_neighbor_tbl_get_entry(address_t *addr);
U8 nwk_neighbor_get_cnt();

// nwk_brc (broadcast table)
void nwk_brc_init();
void nwk_brc_clear();
bool nwk_brc_check_dev_match(U16 dest_addr);
void nwk_brc_add_new_sender(const U16 src_addr, const U8 seq_num);
U8 nwk_brc_start(buffer_t *buf, nwk_hdr_t *hdr);
void nwk_brc_expire(void *ptr);
void nwk_brc_periodic();

// nwk discovery
void nwk_disc_req(U32 channel_mask, U8 duration);
void mac_scan_conf(const mac_scan_conf_t *args);
void mac_beacon_notify_ind(buffer_t *buf, mem_ptr_t *mem_ptr);

// nwk formation
void nwk_form_req(U32 channel_mask, U8 scan_duration);
void nwk_form_select_channel();

// nwk join
void nwk_join_req(const nwk_join_req_t *args);
void mac_assoc_conf(U16 addr, U8 status);
void mac_assoc_ind(mac_assoc_ind_t *args);
void mac_orphan_ind(U64 sender);

// nwk leave
void nwk_leave_req(U64 addr, bool remove_children, bool rejoin);
void nwk_leave_handler(const nwk_hdr_t *hdr, const nwk_cmd_t *cmd);

// nwk reset
void nwk_reset_req(bool cold_start);

// nwk sync
void nwk_sync_start();
void nwk_sync_periodic(void *ptr);
void nwk_sync_req();
void nwk_sync_stop_tmr();

// nwk permit join
void nwk_permit_join_req(U8 duration);
void nwk_permit_join_disable();

// nwk addr map
void nwk_addr_map_init();
void nwk_addr_map_add(U16 nwk_addr, U64 ext_addr, U8 capab);
void nwk_addr_map_clear();
U16 nwk_addr_map_get_nwk_addr(U64 ext_addr);
U64 nwk_addr_map_get_ext_addr(U16 nwk_addr);
void nwk_addr_map_rem_ext_addr(U64 ext_addr);
void nwk_addr_map_rem_nwk_addr(U16 nwk_addr);

#endif // NWK_H


