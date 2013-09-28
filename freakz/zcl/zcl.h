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
    \file zcl.h
    \ingroup zcl
    \brief Zigbee Cluster Library header file

        This is the main header file for the Zigbee Cluster Library and provides
        general definitions. For cluster specific definitions, please check the
        header file associated with each specific cluster group.
*/
/*******************************************************************/
#ifndef ZCL_H
#define ZCL_H

#include "types.h"

#define ZCL_END_MARKER          0xffff          ///< Used to define the end of a list or table.
#define ZCL_MAX_DISC_ATTRIBS    20              ///< Maximum discovered attributes. 
#define ZCL_MAX_STR_SZ          32              ///< Maximum string size.
#define ZCL_MAX_PAYLOAD_SIZE    80              ///< Maximum payload size.
#define ZCL_MAX_RPT_SZ          20              ///< Maximum report size.
#define ZCL_HDR_SZ              3               ///< ZCL Header is 3 bytes

// mem pointer macros
#define ZCL_RPT(m)        ((zcl_rpt_entry_t *)MMEM_PTR(&m->mmem_ptr))   ///< De-reference the mem ptr and cast it as a zcl report entry

/**************************************************************************/
/*!
        Enumeration of the ZCL frame fields.
*/
/**************************************************************************/
typedef enum _zcl_frm_enums_t
{
    ZCL_FRM_TYPE_OFF    = 0,                        ///< Frame type offset  
    ZCL_MANUF_SPEC_OFF  = 2,                        ///< Manuf specification offset
    ZCL_DIR_OFF         = 3,                        ///< Direction offset
    ZCL_DIS_DEF_OFF     = 4,                        ///< Disable default response offset
                                                   
    ZCL_FRM_TYPE_MASK   = (3<<ZCL_FRM_TYPE_OFF),    ///< Frame type mask
    ZCL_MANUF_SPEC_MASK = (1<<ZCL_MANUF_SPEC_OFF),  ///< Manufacturing spec mask
    ZCL_DIR_MASK        = (1<<ZCL_DIR_OFF),         ///< Direction mask
    ZCL_DIS_DEF_MASK    = (1<<ZCL_DIS_DEF_OFF),     ///< Disable default response mask
                                                   
    ZCL_FRM_TYPE_GENERAL    = 0,                    ///< Frame type - general
    ZCL_FRM_TYPE_CLUST_SPEC = 1,                    ///< Frame type - cluster specific
                                                
    ZCL_FRM_DIR_TO_SRVR     = 0,                    ///< Direction - To server
    ZCL_FRM_DIR_TO_CLI      = 1,                    ///< Direction - To client
                                                   
    ZCL_RPT_DIR_SEND_RPT    = 0                     ///< Report Direction - Send report
} zcl_frm_enums_t;

/**************************************************************************/
/*!
        Enumeration of the ZCL cluster IDs
*/
/**************************************************************************/
typedef enum _zcl_clust_id_t
{
    ZCL_BASIC_CLUST_ID          = 0x0000,   ///< Basic cluster ID
    ZCL_PWR_CFG_CLUST_ID        = 0x0001,   ///< Power configuration cluster ID
    ZCL_DEV_TMP_CFG_CLUST_ID    = 0x0002,   ///< Device temperature cluster ID
    ZCL_IDENTIFY_CLUST_ID       = 0x0003,   ///< Identify cluster ID
    ZCL_SCENES_CLUST_ID         = 0x0005,   ///< Scenes cluster ID
    ZCL_ON_OFF_CLUST_ID         = 0x0006,   ///< On/Off cluster ID
    ZCL_LEVEL_CLUST_ID          = 0x0008    ///< Level Control cluster ID
} zcl_clust_id_t;

/**************************************************************************/
/*!
        Enumeration of the ZCL foundation command IDs.
*/
/**************************************************************************/
typedef enum _zcl_gen_cmd_t
{
    ZCL_CMD_READ_ATTRIB             = 0x00,      ///< Read attributes foundation command ID 
    ZCL_CMD_READ_ATTRIB_RESP        = 0x01,      ///< Read attributes response foundation command ID 
    ZCL_CMD_WRITE_ATTRIB            = 0x02,      ///< Write attributes foundation command ID 
    ZCL_CMD_WRITE_ATTRIB_RESP       = 0x03,      ///< Write attributes response foundation command ID 
    ZCL_CMD_WRITE_ATTRIB_UNDIV      = 0x04,      ///< Write attributes undivided foundation command ID 
    ZCL_CMD_WRITE_ATTRIB_NO_RESP    = 0x05,      ///< Write attributes no response foundation command ID 
    ZCL_CMD_CONFIG_REPORT           = 0x06,      ///< Configure reporting foundation command ID 
    ZCL_CMD_CONFIG_REPORT_RESP      = 0x07,      ///< Configure reporting response foundation command ID 
    ZCL_CMD_READ_REPORT_CFG         = 0x08,      ///< Read reporting config foundation command ID 
    ZCL_CMD_READ_REPORT_CFG_RESP    = 0x09,      ///< Read reporting config response foundation command ID 
    ZCL_CMD_REPORT_ATTRIB           = 0x0a,      ///< Report attribute foundation command ID 
    ZCL_CMD_DEFAULT_RESP            = 0x0b,      ///< Default response foundation command ID 
    ZCL_CMD_DISC_ATTRIB             = 0x0c,      ///< Discover attributes foundation command ID 
    ZCL_CMD_DISC_ATTRIB_RESP        = 0x0d       ///< Discover attributes response foundation command ID 
} zcl_gen_cmd_t;

/**************************************************************************/
/*!
        Enumeration of the ZCL attribute data type values.
*/
/**************************************************************************/
typedef enum _zcl_attrib_type_enum_t
{
    ZCL_TYPE_NULL           = 0x00,     ///< Null data type
    ZCL_TYPE_8BIT           = 0x08,     ///< 8-bit value data type
    ZCL_TYPE_16BIT          = 0x09,     ///< 16-bit value data type
    ZCL_TYPE_32BIT          = 0x0b,     ///< 32-bit value data type
    ZCL_TYPE_BOOL           = 0x10,     ///< Boolean data type
    ZCL_TYPE_8BITMAP        = 0x18,     ///< 8-bit bitmap data type
    ZCL_TYPE_16BITMAP       = 0x19,     ///< 16-bit bitmap data type
    ZCL_TYPE_32BITMAP       = 0x1b,     ///< 32-bit bitmap data type
    ZCL_TYPE_U8             = 0x20,     ///< Unsigned 8-bit value data type
    ZCL_TYPE_U16            = 0x21,     ///< Unsigned 16-bit value data type
    ZCL_TYPE_U32            = 0x23,     ///< Unsigned 32-bit value data type
    ZCL_TYPE_BYTE_ARRAY     = 0x41,     ///< Byte array data type
    ZCL_TYPE_CHAR_STRING    = 0x42,     ///< Charactery string (array) data type
    ZCL_TYPE_IEEE_ADDR      = 0xf0,     ///< IEEE address (U64) type
    ZCL_TYPE_INVALID        = 0xff      ///< Invalid data type
} zcl_attrib_type_enum_t;

/**************************************************************************/
/*!
        Enumeration of the attribute access values.
*/
/**************************************************************************/
typedef enum _zcl_attrib_access_enum_t
{
    ZCL_ACCESS_READ_ONLY    = 0x00,     ///< Attribute is read only
    ZCL_ACCESS_READ_WRITE   = 0x01      ///< Attribute is read/write
} zcl_attrib_access_enum_t;

/**************************************************************************/
/*!
        Enumeration of the ZCL status values.
*/
/**************************************************************************/
typedef enum _zcl_status_enum_t
{
    ZCL_STATUS_SUCCESS                  = 0x00,     ///< ZCL Success
    ZCL_STATUS_FAIL                     = 0x01,     ///< ZCL Fail
    ZCL_STATUS_MALFORMED_CMD            = 0x80,     ///< Malformed command
    ZCL_STATUS_UNSUP_CLUST_CMD          = 0x81,     ///< Unsupported cluster command
    ZCL_STATUS_UNSUP_GEN_CMD            = 0x82,     ///< Unsupported general command
    ZCL_STATUS_UNSUP_MANUF_CLUST_CMD    = 0x83,     ///< Unsupported manuf-specific clust command
    ZCL_STATUS_UNSUP_MANUF_GEN_CMD      = 0x84,     ///< Unsupported manuf-specific general command
    ZCL_STATUS_INVALID_FIELD            = 0x85,     ///< Invalid field
    ZCL_STATUS_UNSUP_ATTRIB             = 0x86,     ///< Unsupported attribute
    ZCL_STATUS_INVALID_VALUE            = 0x87,     ///< Invalid value
    ZCL_STATUS_READ_ONLY                = 0x88,     ///< Read only
    ZCL_STATUS_INSUFF_SPACE             = 0x89,     ///< Insufficient space
    ZCL_STATUS_DUPE_EXISTS              = 0x8a,     ///< Duplicate exists
    ZCL_STATUS_NOT_FOUND                = 0x8b,     ///< Not found
    ZCL_STATUS_UNREPORTABLE_ATTRIB      = 0x8c,     ///< Unreportable attribute
    ZCL_STATUS_INVALID_TYPE             = 0x8d,     ///< Invalid type
    ZCL_STATUS_HW_FAIL                  = 0xc0,     ///< Hardware failure
    ZCL_STATUS_SW_FAIL                  = 0xc1,     ///< Software failure
    ZCL_STATUS_CALIB_ERR                = 0xc2,     ///< Calibration error
    ZCL_STATUS_DISC_COMPLETE            = 0x01,     ///< Discovery complete
    ZCL_STATUS_DISC_INCOMPLETE          = 0x00      ///< Discovery incomplete
} zcl_status_enum_t;

/**************************************************************************/
/*!
        ZCL attribute structure. Contains all fields to address the attribute.
        Also, contains an optional field in case the attribute is set to 
        report.
*/
/**************************************************************************/
typedef struct _zcl_attrib_t
{
    U16         id;             ///< Attrib ID
    U8          type;           ///< Attrib data type
    U8          access;         ///< Attrib data access privileges
    void        *data;          ///< Ptr to data
    mem_ptr_t   *rpt;           ///< Placeholder in case this attrib requires a report
} zcl_attrib_t;

/**************************************************************************/
/*!
        ZCL frame control field.
*/
/**************************************************************************/
typedef struct _zcl_frm_ctrl_t
{
    U8      frm_type;       ///< ZCL Frame type 
    bool    manuf_spec;     ///< Manufacturer specific frame
    bool    dir;            ///< Direction
    bool    dis_def_resp;   ///< Disable default response
} zcl_frm_ctrl_t;

/**************************************************************************/
/*!
        ZCL frame header structure.
*/
/**************************************************************************/
typedef struct _zcl_hdr_t
{
    zcl_frm_ctrl_t frm_ctrl;    ///< Frame control field structure
    U8  fcf;                    ///< Frame control field - condensed into single byte
    U16 manuf_code;             ///< Manufacturer code
    U8  seq_num;                ///< Sequence number - used to identify response frame
    U8  cmd;                    ///< Command ID
    U8  *payload;               ///< Payload
    U8  payload_len;            ///< Payload length
} zcl_hdr_t;

/**************************************************************************/
/*!
        ZCL cluster structure. Each cluster contains endpoint, cluster ID, attribute list
        and callbacks to handle processing for that cluster. The RX handler is 
        for incoming frames addressed to that cluster. The action handler is 
        for further actions on that cluster which can be defined by the user:
        ie: Set a GPIO to turn on or off a light.
*/
/**************************************************************************/
typedef struct _zcl_clust_t
{
    U8 ep;                      ///< Endpoint that cluster belongs to
    U16 clust_id;               ///< Cluster ID
    zcl_attrib_t *attrib_list;  ///< Cluster attribute list

    /// Cluster rx handler callback. All frames targeting this cluster will go here to get processed
    void (*rx_handler)(U8 *resp, U8 *resp_len, U16 addr, U8 ep, struct _zcl_clust_t *clust, zcl_hdr_t *hdr);

    /// Cluster action handler. After going to the rx handler, any application specific actions can be defined
    /// in the action handler
    void (*action_handler)(U8 action, void *data);
} zcl_clust_t;

/**************************************************************************/
/*!
        ZCL Write attribute request.
        Structure for a ZCL write attrib request command.
*/
/**************************************************************************/
typedef struct _zcl_write_attrib_rec_t
{
    U8 status;          ///< Write attribute status
    U16 attrib_id;      ///< Attribute ID to be written
} zcl_write_attrib_rec_t;

/**************************************************************************/
/*!
        ZCL Discover Attributes Report
        Structure for a ZCL discover attributes report.
*/
/**************************************************************************/
typedef struct _zcl_disc_attrib_rep_t
{
    U16 attrib_id;      ///< Attribute ID that was discovered
    U8 type;            ///< Data type of attribute
} zcl_disc_attrib_rep_t;

/**************************************************************************/
/*!
        ZCL Report entry
        This is the structure that goes into the ZCL reporting table. Multiple 
        attributes can be set to report at different time intervals. All reports
        will end up as entries in the reporting table which is scanned once per
        second for any pending attribs that need reporting.
*/
/**************************************************************************/
typedef struct _zcl_rpt_entry_t
{
    U16                     timeout;        ///< Max timeout value of the report
    U16                     expiry;         ///< Current value of the expiration. When 0, a report will be sent
    bool                    rpt_needed;     ///< Flag to indicate that a report is needed
    U32                     change;         ///< Min change required to report attribute
    zcl_attrib_t            *attrib;        ///< Pointer to the attribute that will be reported
    U16                     addr;           ///< The destination addr of the report
    U8                      ep;             ///< The destination endpoint of the report
    U16                     clust;          ///< The destination cluster ID
    U16                     prof_id;        ///< Profile ID of endpoint this is being sent from
} zcl_rpt_entry_t;

// ZCL prototypes
void zcl_init();
U8 zcl_get_seq_num();
zcl_attrib_t *zcl_find_attrib(zcl_attrib_t *attrib_list, U16 attrib_id);
zcl_clust_t *zcl_find_clust(zcl_clust_t **clust_list, U16 clust_id);
U8 zcl_get_attrib_size(zcl_attrib_t *attrib);
void zcl_cmd_handler(U8 *resp, U8 *resp_len, U16 src_addr, U8 src_ep, U16 prof_id, zcl_clust_t *clust, zcl_hdr_t *hdr);
void zcl_set_string_attrib(U8 *attrib_data, U8 *val, U8 max_sz);
void zcl_set_clust(zcl_clust_t *clust, U8 ep, U16 clust_id, zcl_attrib_t *attrib_list, void (*rx_handler)(U8 *, U8 *, U16, U8, struct _zcl_clust_t *, zcl_hdr_t *), void (*action_handler)(U8, void *));
void zcl_set_attrib(zcl_attrib_t *attrib, U16 id, U8 type, U8 access, void *data);
void zcl_parse_hdr(U8 *data, U8 len, zcl_hdr_t *hdr);

// zcl_gen
U8 zcl_gen_fcf(zcl_hdr_t *hdr);
U8 zcl_gen_hdr(U8 *data, zcl_hdr_t *hdr);
U8 zcl_gen_read_attrib(U8 *data, zcl_hdr_t *hdr, U16 *attrib_list, U8 attrib_num);
U8 zcl_gen_write_attrib(U8 *data,zcl_hdr_t *hdr, zcl_attrib_t *attrib_list, U8 attrib_num);
U8 zcl_gen_disc_attrib(U8 *data, zcl_hdr_t *hdr, U16 start_attrib, U8 attrib_num);
U8 zcl_gen_config_rpt(U8 *data, zcl_hdr_t *hdr, zcl_clust_t *clust, U16 attrib_id, U16 min_intv, U16 max_intv, U32 change);
U8 zcl_gen_def_resp(U8 *resp, U8 status, zcl_hdr_t *hdr);

// zcl_rpt
void zcl_rpt_init();
U8 zcl_rpt_add(U16 src_addr, U8 src_ep, U16 clust_id, zcl_attrib_t *attrib, U16 prof_id, U16 timeout, U32 change);
U8 zcl_rpt_remove(U16 src_addr, U8 src_ep, U16 clust, zcl_attrib_t *attrib);
void zcl_rpt_periodic();

#endif // ZCL_H
