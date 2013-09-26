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
    \file aps.h
    \ingroup aps
    \brief Application Sublayer header file

    This is the main header file for all source files in the APS layer.
*/
/**************************************************************************/
#ifndef APS_H
#define APS_H

#include "types.h"
#include "nwk.h"
#include "list.h"

#define MAX_GROUP_NAME_SIZE 16          ///< User definable max group name string size

#define DUPE_ENTRY(m)       ((aps_dupe_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as an dupe table entry
#define APS_RETRY_ENTRY(m)  ((aps_retry_t *)MMEM_PTR(&m->mmem_ptr))     ///< De-reference the mem ptr and cast it as an aps retry entry
#define BIND_ENTRY(m)       ((aps_bind_t *)MMEM_PTR(&m->mmem_ptr))      ///< De-reference the mem ptr and cast it as a binding entry
#define GROUP_ID_ENTRY(m)   ((aps_grp_id_t *)MMEM_PTR(&m->mmem_ptr))    ///< De-reference the mem ptr and cast it as a group ID table entry
#define GROUP_ENTRY(m)      ((aps_grp_t *)MMEM_PTR(&m->mmem_ptr))       ///< De-reference the mem ptr and cast it as a group table entry

/*!
    Enumerated definitions for the APS header
*/
typedef enum
{
    // aps frame control offsets
    APS_DELIV_MODE_OFF          = 2,    ///< Delivery mode field offset
    APS_ACK_FMT_OFF             = 4,    ///< ACK format field offset
    APS_SEC_OFF                 = 5,    ///< Security field offset
    APS_ACK_REQ_OFF             = 6,    ///< APS ACK request field offset
    APS_EXT_HDR_OFF             = 7,    ///< Extended header indication field offset

    // aps frame types
    APS_DATA_FRM                = 0x0,  ///< Data frame type
    APS_CMD_FRM                 = 0x1,  ///< Command frame type
    APS_ACK_FRM                 = 0x2,  ///< ACK frame type

    // aps delivery modes
    APS_UNICAST                 = 0x0,  ///< Unicast transmission
    APS_INDIRECT                = 0x1,  ///< Indirect transmission
    APS_BROADCAST               = 0x2,  ///< Broadcast transmission
    APS_GROUP                   = 0x3,  ///< Group transmission

    // indirect address modes
    APS_IND_DEST_EP_PRESENT     = 0x0,  ///< Indirect addressing destination endpoint present
    APS_IND_SRC_EP_PRESENT      = 0x1,  ///< Indirect addressing source endpoint present

    // dest addr modes
    APS_DEST_ADDR_EP_NONE       = 0x0,  ///< No dest addr or endpoint
    APS_GROUP_ADDR_PRESENT      = 0x1,  ///< Group address present
    APS_DEST_ADDR_16_EP_PRESENT = 0x2,  ///< 16-bit dest address present
    APS_DEST_ADDR_64_EP_PRESENT = 0x3,  ///< 64-bit dest address present

    // tx options
    APS_TX_SECURITY_ENB_OFF    = 0,     ///< Security flag offset in Tx options
    APS_TX_USE_NWK_KEY_OFF     = 1,     ///< Use NWK key flag offset in Tx options
    APS_TX_REQ_ACK_TX_OFF      = 2,     ///< ACK request offset in Tx options
    APS_TX_RTE_DISC_DISABLE    = 3      ///< Route discovery disable offset in Tx options
} aps_hdr_enums_t;

/*!
    Enumerated status definitions for APS layer
*/
typedef enum
{
    // APS status enums
    APS_SUCCESS                 = 0x00, ///< Request executed successfully
    APS_ASDU_TOO_LONG           = 0xa0, ///< ASDU exceeded maximum size
    APS_DEFRAG_DEFERRED         = 0xa1, ///< Received fragmented frame could not be defrag'd now
    APS_DEFRAG_UNSUPPORTED      = 0xa2, ///< Device does not support fragmentation
    APS_ILLEGAL_REQUEST         = 0xa3, ///< Request not supported
    APS_INVALID_BINDING         = 0xa4, ///< Bind request failed due to non-existent entry
    APS_INVALID_GROUP           = 0xa5, ///< Group ID doesn't exist in the group table
    APS_INVALID_PARAMETER       = 0xa6, ///< Parameter valid out of range
    APS_NO_ACK                  = 0xa7, ///< ACK not received for transmission with ack request set
    APS_NO_BOUND_DEVICE         = 0xa8, ///< Indirect addressed frame tx failed due to no bound device
    APS_NO_SHORT_ADDRESS        = 0xa9, ///< No short address was found in the address map
    APS_NOT_SUPPORTED           = 0xaa, ///< Indirect tx failed due to no binding table support
    APS_SECURED_LINK_KEY        = 0xab, ///< Received ASDU secured with link key
    APS_SECURED_NWK_KEY         = 0xac, ///< Received ASDU secured with network key
    APS_SECURITY_FAIL           = 0xad, ///< Error occurred during APS security processing
    APS_TABLE_FULL              = 0xae, ///< Bind request or add group failed due to no free entries
    APS_UNSECURED               = 0xaf, ///< Received ASDU had no security
    APS_UNSUPPORTED_ATTRIB      = 0xb0, ///< Unknown attribute for get/set request
    APS_FAIL                    = 0xb1  ///< Request was complete and utter failure (defined by Chris)
} aps_status_enums_t;

/*!
    Miscellaneous enumerated definitions for the APS layer
*/
enum APS_MISC_ENUMS
{
    // duplicate rejection
    DUPE_REJECT_TIMEOUT         = 0x5   ///< Time until expiration for an entry in dupe rejection table
};

/**************************************************************************/
/*!
    Contains the bitfields of the APS frame control field.
*/
/**************************************************************************/
typedef struct
{
    U8      frm_type;       ///< Frame type (data or command)
    U8      delivery_mode;  ///< Indirect, unicast, broadcast, group delivery
    U8      ack_format;     ///< Dest ep, clust id, prof id, src ep present in ACK frame
    bool    security;       ///< Security enabled
    bool    ack_req;        ///< ACK required for this frame
    bool    ext_hdr;        ///< Extended header in use
} aps_frm_ctrl_t;

/**************************************************************************/
/*!
    Contains the fields of the APS header. Its used to generate the APS header
    for transmitted frames or store extracted APS header info from RX frames.
*/
/**************************************************************************/
typedef struct
{
    aps_frm_ctrl_t  aps_frm_ctrl;   ///< Frame control field data struct
    U8              aps_fcf;        ///< Frame control field value
    U8              dest_ep;        ///< Dest endpoint
    U16             grp_addr;       ///< Group address - Used if group addressing
    U16             clust_id;       ///< Cluster ID
    U16             prof_id;        ///< Profile ID
    U8              src_ep;         ///< Src endpoint
    U8              aps_ctr;        ///< APS counter value - used for dupe frame detection
    U16             dest_addr;      ///< Dest address
    U16             src_addr;       ///< Src address
    U8              radius;         ///< Max number of hops
    bool            disc_rte;       ///< Discover route enabled
    U8              handle;         ///< Data handle identifier
} aps_hdr_t;

/**************************************************************************/
/*!
    Contains the arguments needed for the APS data request function. The
    APS header will be constructed out of the arguments.
*/
/**************************************************************************/
typedef struct
{
    U8          dest_addr_mode; ///< Dest address mode (indirect, group, 16-bit, 64-bit address)
    address_t   dest_addr;      ///< Dest address
    U8          dest_ep;        ///< Dest endpoint
    U16         prof_id;        ///< Profile ID
    U16         clust_id;       ///< Cluster ID
    U8          src_ep;         ///< Source endpoint
    U8          asdu_len;       ///< Length of data frame
    buffer_t    *buf;           ///< Data buffer where payload is located
    U8          tx_opt;         ///< Transmit options (ACK, security, route discovery)
    U8          radius;         ///< Max number of hops
    U8          handle;         ///< Data handle ID
} aps_data_req_t;

/**************************************************************************/
/*!
    This is a binding table entry and is used to store src/dest addresses
    and endpoints for bound devices.
*/
/**************************************************************************/
typedef struct _bind_t
{
    U8                  src_ep;             ///< Src endpoint - used to identify destination addresses
    U16                 clust;              ///< Src cluster - used to identify destination addresses
    address_t           dest_addr;          ///< Dest addr
    U8                  dest_ep;            ///< Dest endpoint - used to identify where to send the frame
} aps_bind_t;

/**************************************************************************/
/*!
        Struct for group table entries.
*/
/**************************************************************************/
typedef struct _aps_grp_t
{
    U16 id;             ///< Group ID
    U8 ep;              ///< Endpoint associated with this Group ID
} aps_grp_t;

/**************************************************************************/
/*!
    This struct is an APS retry entry and is used to store a tx buffer and
    APS hdr info for frames that require an ACK. If an ACK does not arrive,
    the info in this entry will be used to re-transmit the frame.
*/
/**************************************************************************/
typedef struct _aps_retry_t
{
    buffer_t            *buf;       ///< Frame to retransmit
    aps_hdr_t           hdr;        ///< Header of frame to retransmit
    U8                  handle;     ///< Data handle ID
    U8                  retries;    ///< Number of retries left before purging frame
    U8                  expiry;     ///< Time remaining before this retry entry is retired
} aps_retry_t;

/**************************************************************************/
/*!
    This struct is used for the duplicate rejection table. When a frame arrives
    its address and aps counter value  will be entered into the dupe rejection
    table. If another frame arrives with the same address and counter value
    before expiration, it will be discarded.
*/
/**************************************************************************/
typedef struct _aps_dupe_t
{
    U16                 src_addr;   ///< Src address of received frame
    U8                  aps_ctr;    ///< APS counter value of received frame
    U8                  expiry;     ///< Time remaining before this entry is retired
} aps_dupe_t;

/**************************************************************************/
/*!
    This is the APS Information Base (AIB) structure. It contains config
    info for the whole application layer.
*/
/**************************************************************************/
typedef struct
{
    bool    desig_coord;        ///< Device is the designated coordinator for this network
    U16     desig_parent;       ///< Address of parent we're forcing the device to join
    U32     chan_mask;          ///< Channel mask for scans
    U64     use_ext_pan_id;     ///< Use this extended PAN ID (0 means use extended address)
    bool    use_desig_parent;   ///< Join designated parent only
    U8      nonmemb_radius;     ///< Maximum hops for a multicast frame
    bool    use_insec_join;     ///< Allow insecure joins
    U8      aps_ctr;            ///< Current APS counter value for this device
    U8      seq;                ///< Current seq value for this device
    //bind_t  *bind_tbl;        // not implemented yet
    //group_t *group_tbl;       // not implemented yet
} aps_aib_t;

/*************** Function Protostypes *********************************/
// aps
void aps_init();
aps_aib_t *aps_aib_get();
U8 aps_data_req(const aps_data_req_t *req);
void nwk_data_conf(U8 status, U8 handle);
void nwk_data_ind(buffer_t *buf, const nwk_hdr_t *nwk_hdr);
void aps_tx(buffer_t *buf, aps_hdr_t *hdr);

// aps_gen
U8 aps_gen_frm_ctrl(const aps_hdr_t *hdr);
void aps_gen_header(buffer_t *buf, aps_hdr_t *hdr);

// aps_parse
void aps_parse_hdr(buffer_t *buf, aps_hdr_t *hdr);

// aps_retry
void aps_retry_init();
void aps_retry_add(buffer_t *buf, aps_hdr_t *hdr, U8 handle);
bool aps_retry_handle_exists(U8 handle);
void aps_retry_ack_handler(const aps_hdr_t *hdr);
void aps_retry_expire(mem_ptr_t *mem_ptr);
void aps_retry_periodic();

// aps_bind
void aps_bind_init();
mem_ptr_t *aps_bind_find_dest(U8 src_ep, U16 clust);
mem_ptr_t *aps_bind_get_head();
U8 aps_bind_req(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust);
U8 aps_unbind_req(U8 src_ep, address_t *dest_addr, U8 dest_ep, U16 clust);

// aps_dupe
void aps_dupe_init();
void aps_dupe_add(U16 src_addr, U8 aps_ctr);
bool aps_dupe_reject(U16 src_addr, U8 aps_ctr);
void aps_dupe_periodic();

// aps group
void aps_grp_init();
U8 aps_grp_add(U16 id, U8 ep);
U8 aps_grp_rem(U16 id, U8 ep);
void aps_grp_rem_all();
mem_ptr_t *aps_grp_get_head();
bool aps_grp_exists(U16 id);

#endif // APS_H



