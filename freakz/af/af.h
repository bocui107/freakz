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
    \file af.h
    \ingroup af
 
    \brief Application Framework header file
    This file is the main header file for all the source files in the
    Application Framework.
*/
/**************************************************************************/
#ifndef AF_H
#define AF_H

#include "aps.h"
#include "buf.h"

#define CONF_ENTRY(m) ((af_conf_entry_t *)MMEM_PTR(&m->mmem_ptr))         ///< De-reference the mem ptr and cast it as an conf queue entry
#define CONF_TBL_ENTRY(m) ((af_conf_tbl_entry_t *)MMEM_PTR(&m->mmem_ptr)) ///< De-reference the mem ptr and cast it as an conf table entry
#define EP_ENTRY(m) ((ep_entry_t *)MMEM_PTR(&m->mmem_ptr))                ///< De-reference the mem ptr and cast it as an ep list entry
#define RX_ENTRY(m) ((af_rx_entry_t *)MMEM_PTR(&m->mmem_ptr))             ///< De-reference the mem ptr and cast it as an rx queue entry
#define TX_ENTRY(m) ((af_tx_entry_t *)MMEM_PTR(&m->mmem_ptr))             ///< De-reference the mem ptr and cast it as an tx queue entry

/****************************************************************/
/*!
    enumerated status definitions for the application framework
*/
/****************************************************************/
enum AF_STATUS_ENUMS
{
    // af status enums
    AF_SUCCESS              = 0x00,     ///< Application request successfully executed
    AF_INV_REQUESTTYPE      = 0x80,     ///< Invalid application request
    AF_DEVICE_NOT_FOUND     = 0x81,     ///< Device does not exist (for request to a child device)
    AF_INVALID_EP           = 0x82,     ///< Destination endpoint is invalid
    AF_NOT_ACTIVE           = 0x83,     ///< Destination endpoint does not have a simple descriptor
    AF_NOT_SUPPORTED        = 0x84,     ///< Requested service not supported on this device
    AF_TIMEOUT              = 0x85,     ///< Timeout occurred on requested operation
    AF_NO_MATCH             = 0x86,     ///< End device bind request failed due to no clust match found
    AF_NO_ENTRY             = 0x88,     ///< Unbind request failed due to no entry to unbind
    AF_NO_DESCRIPTOR        = 0x89,     ///< No descriptor available for child node
    AF_INSUFFICIENT_SPACE   = 0x8a,     ///< No storage space left for requested operation
    AF_NOT_PERMITTED        = 0x8b,     ///< Requested operation is not permitted
    AF_TABLE_FULL           = 0x8c,     ///< No table space to support requested operation
    AF_NOT_AUTHORIZED       = 0x8d,     ///< Insufficient permissions to support requested operation
    AF_NO_FREE_BUFS         = 0x90,     ///< No free buffers for requested operation
    AF_ERROR                = 0xA0      ///< Generic error for requested operation
};

/****************************************************************/
/*!
    Miscellaneous enumerated definitions for the application framework
*/
/****************************************************************/
enum AF_MISC_ENUMS
{
    AF_EP_RANGE_MAX                     = 240,      ///< Max valid endpoint num
    AF_BASE_SIMPLE_DESC_SIZE            = 8,        ///< Size of simple desc without cluster lists
};

/****************************************************************/
/*!
    Endpoint structure used to build the endpoint list.
*/
/****************************************************************/
typedef struct _ep_entry_t
{
    U8                  ep_num;             ///< Endpoint number of this endpoint
    mem_ptr_t           *simple_desc;       ///< Simple descriptor for this endpoint
    U8                  simple_desc_size;   ///< Size of simple descriptor for this endpoint
    bool                zcl;                ///< True if this endpoint supports the ZCL
    void (*ep_rx)       (U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id);  ///< Rx data callback registered with this endpoint
    void (*ep_conf)     (U8 status, U8 handle);     ///< Confirm callback registered with this endpoint
} ep_entry_t;

/****************************************************************/
/*!
    Defines an application framework RX queue element. By queuing the
    received frames in the AF, we can break up the latency in
    processing received frames so that we won't hog the MCU processing
    each received frame.
*/
/****************************************************************/
typedef struct _af_rx_entry_t
{
    buffer_t    *buf;               ///< Received data buffer
    U8          dest_ep;            ///< Destination endpoint
    U8          src_ep;             ///< Source endpoint
    U16         src_addr;           ///< Source address
    U16         clust_id;           ///< Cluster ID
    U16         grp_id;             ///< The group ID of this frame
    bool        grp_mode;           ///< Use the group address instead of the dest ep
} af_rx_entry_t;

/****************************************************************/
/*!
    Defines an application framework confirmation queue element.
*/
/****************************************************************/
typedef struct _af_conf_entry_t
{
    U8      status;                 ///< Status of confirmation
    U8      handle;                 ///< Handle ID of confirmation - used to locate endpoint to route confirm to
} af_conf_entry_t;

/****************************************************************/
/*!
    Defines an application framework confirm table element. The confirmation
    table tracks the handle ID of an outbound frame and its associated
    endpoint. When the data confirmation comes back, we know where it
    needs to go.
*/
/****************************************************************/
typedef struct _af_conf_tbl_entry_t
{
    U8      handle;                     ///< Handle ID for outbound frame
    U8      src_ep;                     ///< Src endpoint of outbound frame
    U8      expiry;                     ///< Time remaining before expiration of this entry
} af_conf_tbl_entry_t;

/****************************************************************/
/*!
    Defines an application framework TX queue element. The TX queue
    is used to buffer the outbound data frame. By doing this, we can
    break up the latency to transmit a frame.
*/
/****************************************************************/
typedef struct _af_tx_entry_t
{
    buffer_t    *buf;               ///< Data buffer to transmit
    U8          src_ep;             ///< Source endpoint of this frame
    U16         dest_addr;          ///< Dest address of this frame
    U8          dest_ep;            ///< Dest endpoint of this frame
    U16         clust;              ///< Dest cluster ID of this frame
    U16         prof_id;            ///< Profile ID
    U8          mode;               ///< Delivery mode for this frame
    U8          tx_opt;             ///< Transmit options for this frame
    U8          radius;             ///< Max number of hops for this frame
    U8          handle;             ///< Handle ID for this frame
    U16         grp_id;
} af_tx_entry_t;

/*************** Function Protostypes *********************************/
// af
void af_init();
void af_rx(buffer_t *buf, aps_hdr_t *hdr);
void af_tx(U8 *data, U8 len, U8 src_ep, U16 dest_addr, U8 dest_ep, U16 clust, U16 prof_id, U8 mode, U8 tx_opt, U8 radius, U8 handle);
void aps_conf(U8 status, U8 handle);
U8 af_handle_get();

// af_ep
void af_ep_init();
void af_ep_clear_all();
void af_ep_add(U8 ep_num, U8 *simple_desc, U8 desc_size, bool zcl,
             void (*ep_rx)(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust_id),
             void (*ep_conf)(U8, U8));
mem_ptr_t *af_ep_find(U8 ep_num);
void af_ep_rx(U16 dest_ep, buffer_t *buf, U8 len, U16 src_addr, U8 src_ep, U16 clust_id);
void af_ep_conf(U16 dest_ep, U8 status, U8 handle);
U8 af_ep_get_active(U8 *active_list);
U8 af_ep_find_matches(U16 prof_id, const clust_list_t *in_list, const clust_list_t *out_list, U8 *eps);

// af_rx
void af_rx_init();
void af_rx_add(buffer_t *buf, const aps_hdr_t *hdr);
void af_rx_free(mem_ptr_t *mem_ptr);
mem_ptr_t *af_rx_pop();

// af_tx
void af_tx_init();
void af_tx_add(buffer_t *buf, U8 src_ep, U16 dest_addr, U8 dest_ep, U16 clust, U16 prof_id, U8 mode, U8 tx_opt, U8 radius, U8 handle);
void af_tx_free(mem_ptr_t *mem_ptr);
mem_ptr_t *af_tx_pop();

// af_conf
void af_conf_init();
void af_conf_add(U8 status, U8 handle);
void af_conf_free(mem_ptr_t *mem_ptr);
mem_ptr_t *af_conf_pop();

// af_conf_tbl
void af_conf_tbl_init();
void af_conf_tbl_add(U8 src_ep, U8 handle);
void af_conf_tbl_free(mem_ptr_t *mem_ptr);
mem_ptr_t *af_conf_tbl_find(U8 handle);
void af_conf_tbl_periodic();
#endif
