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
    \file mac.h
    \ingroup mac
    \brief MAC header file

    Header file for the MAC layer of the FreakZ stack.
*/
#ifndef MAC_H
#define MAC_H
#include "contiki.h"
#include "ctimer.h"
#include "types.h"
#include "buf.h"
#include "mem_heap.h"

/* Calculate amount of time to perform a scan */
#define MAC_SCAN_TIME(duration) (((aBaseSuperframeDuration << duration) + aBaseSuperframeDuration) >> 6)

/*
 * this define is just used to make the code more comprehensible.
 * otherwise, you'd have to stare at these monsters all over the
 * place.
 */

#define INDIR_ENTRY(m) ((mac_indir_t *)MMEM_PTR(&m->mmem_ptr))
#define RETRY_ENTRY(m) ((mac_retry_t *)MMEM_PTR(&m->mmem_ptr))
#define SCAN_ENTRY(m)  ((pan_descr_t *)MMEM_PTR(&m->mmem_ptr))

/* Enumerated definitions for the MAC Frame Control Field in the MAC Header */
typedef enum
{
	/* Data pending flag offset in FCF */
	MAC_FRM_PEND_OFF	= 4,
	/* Ack request flag offset in FCF */
	MAC_ACK_REQ_OFF		= 5,
	/* Pan ID compression offset in FCF */
	MAC_PAN_ID_COMPR_OFF	= 6,
	/* Destination address mode offset in FCF */
	MAC_DEST_ADDR_MODE_OFF	= 10,
	/* Source address offset in FCF */
	MAC_SRC_ADDR_MODE_OFF	= 14,

	/* MAC frame control field bit masks */
	/* Data, command, ACK, or beacon frame indicator */
	MAC_FRAME_TYPE_MASK	= 0x0003,
	/* Indirect data awaiting retrieval for dest address */
	MAC_PENDING_FRAME_MASK	= 0x0010,
	/* Ack required for this frame */
	MAC_ACK_REQ_MASK	= 0x0020,
	/*
	 * If src and dest PAN ID are same, then include
	 * once and set this flag
	 */
	MAC_PAN_ID_COMPR_MASK	= 0x0040,
	/*
	 * Address mode in address field for
	 * dest - network or extended addr
	 */
	MAC_DEST_ADDR_MODE_MASK	= 0x0C00,
	/*
	 * Address mode in address field for
	 * src - network or extended addr
	 */
	MAC_SRC_ADDR_MODE_MASK	= 0xC000,

	/* frame type for FCF enums */
	/* Beacon frame indicator */
	MAC_BEACON		= 0x0,
	/* Data frame indicator */
	MAC_DATA		= 0x1,
	/* ACK frame indicator */
	MAC_ACK			= 0x2,
	/* Command frame indicator */
	MAC_COMMAND		= 0x3,

	/* address mode enums */
	/* No address and no PAN ID in address field */
	NO_PAN_ID_ADDR		= 0x0,
	/* Address field contains network (short) address */
	SHORT_ADDR		= 0x2,
	/* Address field contains IEEE (long) address */
	LONG_ADDR		= 0x3
} mac_frm_ctrl_enums_t;

/* Enumerated definitions for the MAC Beacon frames */
typedef enum
{
	/* mac beacon fields */
	/* Guaranteed timeslot mask for beacon frame */
	MAC_GTS_MASK			= 0x7,
	/* Guaranteed timeslot descriptor size for beacon frame */
	MAC_GTS_DESC_SIZE		= 0x3,
	/* Guaranteed timeslot direction size for beacon frame */
	MAC_GTS_DIR_SIZE		= 0x1,
	/* Pending short address mask for beacon frame */
	MAC_PENDING_SHORT_ADDR_MASK	= 0x7,
	/* Pending long address offset for beacon frame */
	MAC_PENDING_LONG_ADDR_OFF	= 0x4,
	/* Pending long address mask for beacon frame */
	MAC_PENDING_LONG_ADDR_MASK	= 0x70,
	/* Beacon frame payload size */
	MAC_BEACON_PAYLOAD_SIZE		= 0x4,

	/* mac superframe spec fields */
	MAC_ASSOC_PERMIT_OFF		= 15,
	MAC_COORDINATOR_OFF		= 14,
	MAC_FINAL_CAP_SLOT_OFF		= 8,
	MAC_SUPERFRAME_ORDER_OFF	= 4,
	MAC_BEACON_ORDER_OFF		= 0,
	/* Indicate if association is permitted (superfrm spec) */
	MAC_ASSOC_PERMIT_MASK		= (0x1 << MAC_ASSOC_PERMIT_OFF),
	/* Indicate if coordinator (superfrm spec) */
	MAC_COORDINATOR_MASK		= (0x1 << MAC_COORDINATOR_OFF),
	/* Indicate beacon order (superfrm spec) */
	MAC_BEACON_ORDER		= (0xF << MAC_BEACON_ORDER_OFF),
	/* Indicate superfrm order (superfrm spec) */
	MAC_SUPERFRAME_ORDER		= (0xF << MAC_SUPERFRAME_ORDER_OFF),
	/* Indicate final contention access period slot (superfrm spec) */
	MAC_FINAL_CAP_SLOT		= (0xF << MAC_FINAL_CAP_SLOT_OFF)
} mac_beacon_enums_t;

/* Enumerated definitions for the MAC command frames. */
typedef enum
{
	/* mac command frames */
	MAC_ASSOC_REQ		= 0x1,  /* Association request command ID */
	MAC_ASSOC_RESP		= 0x2,  /* Association response command ID */
	MAC_DATA_REQ		= 0x4,  /* Data request command ID */
	MAC_ORPHAN_NOT		= 0x6,  /* Orphan notification command ID */
	MAC_BEACON_REQ		= 0x7,  /* Beacon request command ID */
	MAC_COORD_REALIGN	= 0x8,  /* Coordinator realignment command ID */

	MAC_ASSOC_REQ_LEN	= 0x2,
	MAC_ASSOC_RESP_LEN	= 0x4,
	MAC_COORD_REALIGN_LEN	= 0x8
} mac_cmd_frm_enums_t;

/* Enumerated definitions for the MAC status values */
typedef enum
{
    // mac status
    MAC_SUCCESS                     = 0x0,      ///< Transaction was successful
    MAC_BEACON_LOSS                 = 0xE0,     ///< Beacon was lost (used in beacon'd networks)
    MAC_CHANNEL_ACCESS_FAILURE      = 0xE1,     ///< Unable to transmit due to channel being busy
    MAC_COUNTER_ERROR               = 0xDB,     ///< Frame counter of received frame is invalid
    MAC_DENIED                      = 0xE2,     ///< GTS request denied
    MAC_DISABLE_TRX_FAILURE         = 0xE3,     ///< Failed to disable the transceiver
    MAC_SECURITY_ERROR              = 0xE4,     ///< Frame failed decryption
    MAC_FRAME_TOO_LONG              = 0xE5,     ///< Frame exceeded maximum size
    MAC_IMPROPER_KEY_TYPE           = 0xDC,     ///< Key not allowed to be used with this frame type
    MAC_IMPROPER_SECURITY_LEVEL     = 0xDD,     ///< Frame does not meet min security level expected
    MAC_INVALID_ADDRESS             = 0xF5,     ///< Data request failed because no src or dest address
    MAC_INVALID_GTS                 = 0xE6,     ///< Invalid timeslot requested (beacon'd networks)
    MAC_INVALID_HANDLE              = 0xE7,     ///< Invalid frame data handle
    MAC_INVALID_INDEX               = 0xF9,     ///< Invalid index when trying to write MAC PIB
    MAC_INVALID_PARAMETER           = 0xE8,     ///< Invalid parameter passed to service
    MAC_LIMIT_REACHED               = 0xFA,     ///< Scan terminated because max pan descriptors reached
    MAC_NO_ACK                      = 0xE9,     ///< ACK not received after tx with ack_req flag set
    MAC_NO_BEACON                   = 0xEA,     ///< Beacon not returned after beacon request
    MAC_NO_DATA                     = 0xEB,     ///< Data frame not returned after data request (indirect poll)
    MAC_NO_SHORT_ADDRESS            = 0xEC,     ///< No short address allocated to this device (due to lack of address space)
    MAC_ON_TIME_TOO_LONG            = 0xF6,     ///< Rx enable request failed. Spec'd number of symbols longer than beacon interval
    MAC_OUT_OF_CAP                  = 0xED,     ///< Association failed due to lack of capacity (no nbor tbl entry or no address)
    MAC_PAN_ID_CONFLICT             = 0xEE,     ///< Different networks within listening range have identical PAN IDs
    MAC_PAST_TIME                   = 0xF7,     ///< Rx enable failed. Too late for current superframe and unable to be deferred
    MAC_READ_ONLY                   = 0xFB,     ///< PIB attribute is read only
    MAC_REALIGNMENT                 = 0xEF,     ///< Coordinator realignment received
    MAC_SCAN_IN_PROGRESS            = 0xFC,     ///< Request to perform scan failed because scan already in progress
    MAC_SUPERFRAME_OVERLAP          = 0xFD,     ///< Start time of beacon overlapped transmission time of coordinator beacon
    MAC_TRACKING_OFF                = 0xF8,     ///< Device not tracking beacons but instructed to send beacons based on tracked beacons
    MAC_TRANSACTION_EXPIRED         = 0xF0,     ///< Frame buffered in indirect queue expired
    MAC_TRANSACTION_OVERFLOW        = 0xF1,     ///< Exceeded maximum amount of entries in indirect queue
    MAC_TX_ACTIVE                   = 0xF2,     ///< Transmission in progress
    MAC_UNAVAILABLE_KEY             = 0xF3,     ///< Security key unavailable
    MAC_UNSUPPORTED_ATTRIBUTE       = 0xF4,     ///< Requested PIB attribute is not supported
    MAC_UNSPECIFIED_FAILURE         = 0xF5,     ///< Unspecified failure occurs. This is catch all for any type of general failure.
    MAC_UNSUPPORTED_LEGACY          = 0xDE,     ///< 802.15.4 2003 security on frame, but not supported by device
    MAC_UNSUPPORTED_SECURITY        = 0xDF      ///< Security on received frame is not supported
} mac_status_enums_t;

/****************************************************************/
/*!
    Miscellaneous enumerated definitions used by the MAC layer
*/
/****************************************************************/
typedef enum
{
    // tx options
    MAC_ACK_REQUEST                     = 0x01,     ///< Ack is required for this transmission. MAC data request service.
    MAC_INDIRECT_TRANS                  = 0x04,     ///< Frame will be sent indirectly. MAC data request service.

    // broadcast address
    MAC_BROADCAST_ADDR                  = 0xFFFF,   ///< MAC broadcast address

    // 802.15.4 Frame Versions
    MAC_802_15_4_2003                   = 0x0,      ///< Compliant to 802.15.4 2003 frame format
    MAC_802_15_4_2006                   = 0x1,      ///< Compliant to 802.15.4 2006 frame format

    // scan types
    MAC_ENERGY_SCAN                     = 0x00,     ///< Energy scan identifier
    MAC_ACTIVE_SCAN                     = 0x01,     ///< Active scan identifier
    MAC_ORPHAN_SCAN                     = 0x03,     ///< Orphan scan identifier

    MAC_MAX_CHANNELS                    = 16,       ///< Max number of channels supported by MAC
    MAC_PHY_CHANNEL_OFFSET              = 11,       ///< Channel offset. 802.15.4 range is Ch 11-26
    MAC_FCS_LENGTH                      = 2,        ///< Size of the MAC frame checksum

    // mac management states
    MLME_IDLE                           = 0x0,      ///< Idle state for the MAC management state machine
    MLME_ASSOC_REQ                      = 0x1,      ///< Association request state of MAC management state machine
    MLME_DATA_REQ                       = 0x2,      ///< Data request state of MAC management state machine
    MLME_SCAN                           = 0x3       ///< Scan state of MAC management state machine
} mac_misc_enums_t;

/*******************************************************************/
/*!
    This is the MAC Information Base. It holds all of the config
    info for the MAC layer.
*/
/*******************************************************************/
typedef struct
{
    U16         ack_wait_duration;      ///< Time to wait for ACK from a tx'd frame
    bool        assoc_permit;           ///< Association is allowed
    address_t   coord_addr;             ///< Address of parent this device is joined to
    U8          dsn;                    ///< Current data sequence number
    U8          min_be;                 ///< Minimum backoff interval exponent (interval ~ 2^min_be)
    U8          max_csma_backoffs;      ///< Max number of CSMA backoffs before the tx is failed
    U16         pan_id;                 ///< PAN ID that this device is joined to
    bool        rx_on_when_idle;        ///< Receiver is always on
    U16         resp_wait_time;         ///< Time to wait before polling potential parent for association response
    bool        security_enb;           ///< Security is enabled
    U16         short_addr;             ///< Short (network) address of this device
    U64         ext_addr;               ///< Extended (IEEE) address of this device
        bool        coord;                  ///< This device is the coordinator
    U8          curr_channel;           ///< Current active channel
} mac_pib_t;

/*******************************************************************/
/*!
    This struct holds the info for the MAC frame control field.
*/
/*******************************************************************/
typedef struct
{
    U8      frame_type;     ///< Frame type this frame
    bool    sec_enb;        ///< Security enabled
    bool    frame_pending;  ///< Pending frame in the indirect queue
    bool    ack_req;        ///< ACK requested for this frame
    bool    pan_id_compr;   ///< Compress PAN ID if PAN ID is same for src and dest
    U8      frame_ver;      ///< 802.15.4 version this frame supports (2003 or 2006)
} mac_frm_ctrl_t;

/*******************************************************************/
/*!
    This struct contains the fields for the MAC header.
*/
/*******************************************************************/
typedef struct
{
    mac_frm_ctrl_t  mac_frm_ctrl;   ///< Frame control field structure
    U16             mac_fcf;        ///< Frame control field hex value (value after bit fields are assembled)
    U8              dsn;            ///< Data sequence number
    U16             dest_pan_id;    ///< Destination network ID
    address_t       dest_addr;      ///< Destination address
    U16             src_pan_id;     ///< Source network ID
    address_t       src_addr;       ///< Source address
} mac_hdr_t;

/*******************************************************************/
/*!
    Association request command format
*/
typedef struct
{
    U8      cap_info;               ///< Capability info for this device
} cmd_assoc_req_t;

/*******************************************************************/
/*!
    Association response command format
*/
typedef struct
{
    U16     short_addr;             ///< Short address given to the device requesting association
    U8      assoc_status;           ///< Assocation request status
} cmd_assoc_resp_t;

/*******************************************************************/
/*!
    Coordinator realignment command format - Used for orphan responses
*/
typedef struct
{
    U16     pan_id;                 ///< Network ID for realignment frame_pending
    U16     coord_short_addr;       ///< Coordinator or parent's network address
    U8      channel;                ///< Channel the parent is operating on
    U16     short_addr;             ///< Network address of the orphan
} cmd_coord_realign_t;

/*******************************************************************/
/*!
    Union of MAC command formats that are used in this stack. Looks like
    only three are used. ha ha ha.
*/
typedef struct
{
    U8 cmd_id;                                  ///< MAC cmd frame command ID
    union
    {
        cmd_assoc_req_t         assoc_req;      ///< Assocation request cmd frame
        cmd_assoc_resp_t        assoc_resp;     ///< Association response cmd frame
        cmd_coord_realign_t     coord_realign;  ///< Coordinator realign cmd frame
    };
} mac_cmd_t;

/*******************************************************************/
/*!
    This is the PAN descriptor, aka scan descriptor. The PAN descriptors
    are used in the scan list when we do a MAC active scan. When we receive
    the beacon responses, the information in the beacons go into these
    fields.
*/
/*******************************************************************/
typedef struct _pan_descr_t
{
    address_t   coord_addr;     ///< Address of the router sending the beacon
    U16         coord_pan_id;   ///< Network ID of this network
    U8          channel;        ///< Channel this network is located on
    U16         superfrm_spec;  ///< Misc info for this network
    U8          link_quality;   ///< Quality of link to the sender of this beacon
    U8          prot_id;        ///< Protocol ID - always 0
    U8          stack_profile;  ///< Zigbee stack profile
    U8          prot_ver;       ///< Zigbee Protocol version
    bool        rtr_cap;        ///< Capacity available for routers to join
    U8          depth;          ///< Network depth of this router
    bool        end_dev_cap;    ///< Capacity available for end devices to join
    U64         ext_pan_id;     ///< Extended PAN ID
    bool        pot_parent;     ///< Potential Parent for this device
} pan_descr_t;

/*
 * The MAC data request structure is used to hold the arguments
 * for the MAC data request service. The arguments will be used
 * to generate the header which will then be stored in the buffer.
 *
 * src_addr: Src address (usually this device's address)
 * dest_addr: Dest address
 * src_pan_id: Src PAN ID
 * dest_pan_id: Dest PAN ID
 * buf: Data buffer to be transmitted
 * msdu_handle: Data handle identifier for this frame
 * tx_options: Transmission options (ack request, indirect)
 */
typedef struct
{
	address_t	src_addr;
	address_t	dest_addr;
	U16		src_pan_id;
	U16		dest_pan_id;
	buffer_t	*buf;
	U8		msdu_handle;
	U8		tx_options;
} mac_data_req_t;

/*
 * These are the arguments for the MAC association request function.
 *
 * channel: Channel of network to join
 * coord_addr: Address of parent to join
 * coord_pan_id: PAN ID of network to join
 * capability: Capability info of this device
 */
typedef struct
{
	U8		channel;
	address_t	coord_addr;
	U16		coord_pan_id;
	U8		capability;
} mac_assoc_req_t ;

/*
 * These are the arguments for the MAC association indication function.
 *
 * Address of device requesting association
 * Capability info of device requesting association
 */
typedef struct
{
	address_t	dev_addr;
	U8		capability;
} mac_assoc_ind_t;

/*
 * These are the arguments for the MAC association response function.
 *
 * dev_add: IEEE address of child
 * assoc_short_addr: Network address given to child
 * status: Status of association request
 */
typedef struct
{
	address_t	dev_addr;
	U16		assoc_short_addr;
	U8		status;
} mac_assoc_resp_t;

/*
 * These are the arguments for the MAC association confirm function.
 *
 * status: Status of scan request
 * scan_type: Scan type requested
 * energy_list: Pointer to the energy scan list
 */
typedef struct
{
	U8  status;
	U8  scan_type;
	U8  *energy_list;
} mac_scan_conf_t;

/*
 * These are the arguments for the MAC start request function
 *
 * pan_id: Start the MAC layer of this device with this PAN ID
 * channel: Start the MAC layer of this device on this channel
 */
typedef struct
{
	U16     pan_id;
	U8      channel;
} mac_start_req_t;

/*
 * This structure is used for the MAC retry queue. When an ACK fails
 * to arrive for a transmitted frame, the data from this struct is
 * used to re-send the data.
 *
 * buf: Data buffer to be retransmitted
 * dsn: Data sequence number of data
 * retries: Number of retries remaining
 * handle: Data handle identifier for confirmation
 * expiry: Time to expiration of this entry
 */
typedef struct _mac_retry_t
{
	buffer_t	*buf;
	U8		dsn;
	U8		retries;
	U8		handle;
	U8		expiry;
} mac_retry_t;

/*
 * This struct is used for the MAC indirect queue. Frames are buffered
 * in this struct until a sleeping node wakes up and polls this device
 * for its data.
 *
 * buf: Data buffer to be transmitted
 * dsn: Data sequence number
 * ack_req: ACK required for this transmission
 * addr: Destination address of this frame
 * handle: Data handle identifier for confirmation
 * expiry: Time to expiration of this entry
 */
typedef struct _mac_indir_t
{
	buffer_t	*buf;
	U8		dsn;
	bool		ack_req;
	address_t	addr;
	U8		handle;
	U8		expiry;
} mac_indir_t;

/*
 * This is the MAC protocol control block. It holds variables that
 * are used throughout the MAC layer.
 *
 * mac_state: Current state
 * mlme_tmr: Callback timer shared by MAC managment functions
 * parent_addr: Address of parent this device is requesting to join
 * assoc_req_dsn: DSN of association request
 * original_pan_id: Original PAN ID before channel scan
 * curr_scan_channel: Current channel being scanned
 * channel_mask: Channel mask for this scan
 * duration: Duration of this scan
 * nwk_cnt: Number of networks found
 * scan_type: ype of scan to be performed
 * coor_realign_rcvd: Coordinator alignment received for orphan scan
 * energy_list: Energy list to store values from energy scan
 * total_xmit: Total number of transmissions attempted
 * total_fail: Total number of transmissions failed
 */
typedef struct
{
	U8		mac_state;
	struct ctimer	mlme_tmr;

	/* association */
	address_t	parent_addr;
	U8		assoc_req_dsn;

	/* channel scanning */
	U16		original_pan_id;
	U8		curr_scan_channel;
	U32		channel_mask;
	U8		duration;
	U8		nwk_cnt;
	U8		scan_type;
	bool		coor_realign_rcvd;
	U8		energy_list[MAC_MAX_CHANNELS];

	/* statistics */
	U16		total_xmit;
	U16		total_fail;
} mac_pcb_t;
/**********************************************************/

/*! Main process for the MAC layer */
PROCESS_NAME(mac_process);

// mac
void mac_init();
mac_pcb_t *mac_pcb_get();
mac_pib_t *mac_pib_get();
void mac_data_req(mac_data_req_t *req);
void mac_out(buffer_t *buf, bool ack_req, U8 dsn, U8 handle);
void mac_cmd_handler(mac_cmd_t *cmd, mac_hdr_t *hdr);

// mac reset
void mac_reset_req(bool set_default_pib);

// mac_queue
void mac_queue_init();
void mac_queue_buf_insert(buffer_t *buf);
buffer_t *mac_queue_buf_pop();
bool mac_queue_is_empty();
buffer_t *mac_queue_get_head();
void mac_queue_clear();

//  mac_retry
void mac_retry_init();
void mac_retry_clear();
void mac_retry_add(buffer_t *buf, U8 dsn, U8 handle);
void mac_retry_rem(U8 dsn);
void mac_retry_ack_handler(U8 dsn);
void mac_retry_periodic(void *ptr);
void mac_retry_stop_tmr();

// mac_gen
U16 mac_gen_frm_ctrl(const mac_hdr_t *hdr);
void mac_gen_header(buffer_t *buf, mac_hdr_t *hdr);
void mac_gen_cmd_header(buffer_t *buf, mac_hdr_t *hdr, bool ack_req, address_t *src_addr, address_t *dest_addr);
void mac_gen_ack(buffer_t *buf, bool frm_pend, U8 dsn);
void mac_gen_cmd(buffer_t *buf, mac_cmd_t *cmd);
U16 mac_gen_superfrm_spec();
void mac_gen_beacon_frm(buffer_t *buf, mac_hdr_t *hdr);

//mac_parse
void mac_parse_hdr(buffer_t *buf, mac_hdr_t *hdr);
void mac_parse_cmd(buffer_t *buf, mac_cmd_t *cmd);
void mac_parse_beacon(buffer_t *buf, mac_hdr_t *hdr);

// mac_indir
void mac_indir_init();
void mac_indir_add(buffer_t *buf, address_t *addr, bool ack_req, U8 dsn, U8 handle);
void mac_indir_data_req_handler(const address_t *addr);
bool mac_indir_frm_pend(const address_t *addr);
void mac_indir_clear();
void mac_indir_periodic();

// mac_scan
void mac_scan_init();
void mac_scan_descr_add(address_t *src_addr, U16 src_pan_id, U8 channel, U16 superframe_spec);
void mac_scan_descr_free(mem_ptr_t *mem_ptr);
mem_ptr_t *mac_scan_descr_get_head();
void mac_scan_descr_clear();
mem_ptr_t *mac_scan_descr_find_pan(U16 pan_id);
mem_ptr_t *mac_scan_descr_find_addr(address_t *addr);
U8 *mac_scan_get_energy_list();
void mac_scan_energy();
void mac_scan(void *ptr);
U8 mac_scan_get_entry_cnt();

// mac_assoc
void mac_assoc_req(mac_assoc_req_t *args);
void mac_assoc_resp(mac_assoc_resp_t *args);
void mac_orphan_resp(U64 orphan_addr, U16 short_addr);

// mac_poll
void mac_poll_req();
void mac_poll_timeout();

// mac start
void mac_start_req(const mac_start_req_t *args);

#endif // MAC_H

