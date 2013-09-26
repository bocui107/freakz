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
    \defgroup zcl ZCL - The Zigbee Cluster Library
    \file zcl.c
    \ingroup zcl
    \brief Zigbee Cluster Library Toplevel file

        This is the main entry point into the Zigbee cluster library and where
        the foundation commands are implemented. The ZCL actually consists of
        the foundation commands which are used to access basic attributes of
        the clusters and then separate files where the application specific
        clusters are defined.
*/
/**************************************************************************/
#include "freakz.h"

static U8 seq_num;      ///< Instantiation of the ZCL sequence number.

/**************************************************************************/
/*!
    Initialize the Zigbee Cluster Library
*/
/**************************************************************************/
void zcl_init()
{
    seq_num = drvr_get_rand() & 0xff;
}

/**************************************************************************/
/*!
        Get the sequence number. Each ZCL frame consists of a sequence number
        which is used to match up the response to the initial ZCL frame.
*/
/**************************************************************************/
U8 zcl_get_seq_num()
{
    return seq_num++;
}

/**************************************************************************/
/*!
        Used to find the attribute from a list of attributes based on the
        attribute ID.
*/
/**************************************************************************/
zcl_attrib_t *zcl_find_attrib(zcl_attrib_t *attrib_list, U16 attrib_id)
{
    U8 i;

    if (attrib_list)
    {
        for (i=0; attrib_list[i].id != ZCL_END_MARKER; i++)
        {
            if (attrib_list[i].id == attrib_id)
            {
                return &attrib_list[i];
            }
        }
    }
    return NULL;
}

/**************************************************************************/
/*!
        Used to find a cluster from a list of clusters based on the cluster ID.
*/
/**************************************************************************/
zcl_clust_t *zcl_find_clust(zcl_clust_t **clust_list, U16 clust_id)
{
    U8 i;

    if (clust_list)
    {
        for (i=0; clust_list[i]->clust_id != ZCL_END_MARKER; i++)
        {
            if (clust_list[i]->clust_id == clust_id)
            {
                return clust_list[i];
            }
        }
    }
    return NULL;
}

/**************************************************************************/
/*!
        Decodes the attribute size and returns the size in bytes. This will
        be used when calculating the length of the following attribute value for
        either decoding of a ZCL frame or generating a ZCL response.
*/
/**************************************************************************/
U8 zcl_get_attrib_size(zcl_attrib_t *attrib)
{
    switch (attrib->type)
    {
    case ZCL_TYPE_8BIT:
    case ZCL_TYPE_U8:
        return sizeof(U8);

    case ZCL_TYPE_16BIT:
    case ZCL_TYPE_U16:
        return sizeof(U16);

    case ZCL_TYPE_32BIT:
    case ZCL_TYPE_U32:
        return sizeof(U32);

    case ZCL_TYPE_BOOL:
        return sizeof(bool);

    case ZCL_TYPE_CHAR_STRING:
        // the first byte of the string attrib has the size. so we need
        // to return the value of the first byte + 1 to account for using
        // a byte to hold the string size
        return ((*(U8 *)attrib->data) + 1);
    default:
        return 0xff;
    }
}

/**************************************************************************/
/*!
    Check if there is space in the response frame for the current attribute.
        The max length is based on the max application payload size.
*/
/**************************************************************************/
static bool zcl_attrib_check_space_avail(zcl_attrib_t *attrib, U8 resp_len)
{
    U8 size;

    if (attrib)
    {
        // need to add an extra 4 bytes to the size because we need to send the attrib id
        // type, and status
        size = zcl_get_attrib_size(attrib);
        return ((resp_len + size + 4) < ZCL_MAX_PAYLOAD_SIZE);
    }
    return false;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_set_string_attrib(U8 *attrib_data, U8 *val, U8 max_sz)
{
    U8 len;

    if ((len = strlen((char *)val)) < max_sz)
    {
        memcpy(attrib_data, &len, 1);
        memcpy(attrib_data + 1, val, len);
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_set_clust(zcl_clust_t *clust, U8 ep, U16 clust_id, zcl_attrib_t *attrib_list,
                   void (*rx_handler)(U8 *, U8 *, U16, U8, struct _zcl_clust_t *, zcl_hdr_t *),
                   void (*action_handler)(U8, void *))
{
    clust->ep               = ep;
    clust->clust_id         = clust_id;
    clust->attrib_list      = attrib_list;
    clust->rx_handler       = rx_handler;
    clust->action_handler   = action_handler;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void zcl_set_attrib(zcl_attrib_t *attrib, U16 id, U8 type, U8 access, void *data)
{
    attrib->id        = id;
    attrib->type      = type;
    attrib->access    = access;
    attrib->data      = data;
    attrib->rpt       = NULL;
}

/**************************************************************************/
/*!
        This function implements the ZCL read attribute foundation command.
        It can read either a single attribute or a list of attributes and returns
        a response of all the attribute values. The response can only be as long
        as the maximum application payload size.

    TODO: Rewrite this function to make it more concise
*/
/**************************************************************************/
static U8 zcl_cmd_read_attrib(U8 *resp, U16 src_addr, U8 src_ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    U8 i, str_len, len, status, *resp_ptr, *data_ptr;
    U16 attrib_id;
    zcl_hdr_t hdr_out;
    zcl_attrib_t *attrib = NULL;

    // save the original position so we can calculate the amount of data we wrote
    resp_ptr = resp;
    data_ptr = hdr->payload;

    // generate the header. most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(zcl_hdr_t));
    hdr_out.frm_ctrl.dir = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd = ZCL_CMD_READ_ATTRIB_RESP;

    // this is where the header actually gets generated
    len = zcl_gen_hdr(resp_ptr, &hdr_out);
    resp_ptr += len;

    // now iterate over all the attributes that are requesting to be read
    for (i=0; i<hdr->payload_len; i+=2)
    {
        // init the status to an inital value of SUCCESS
        status = ZCL_STATUS_SUCCESS;

        // get the attrib id from the frame
        attrib_id = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        // get attrib from attrib list
        if ((attrib = zcl_find_attrib(clust->attrib_list, attrib_id)) == NULL)
        {
            status = ZCL_STATUS_UNSUP_ATTRIB;
        }
        else
        {
            // check to make sure there is enough space in the frame to include the attribute
            if (!zcl_attrib_check_space_avail(attrib, resp_ptr - resp))
            {
                status = ZCL_STATUS_INSUFF_SPACE;
            }
        }

        // attrib id
        *(U16 *)resp_ptr = attrib_id;
        resp_ptr += sizeof(U16);
        *resp_ptr++ = status;

        // if we're out of room, then end the response frame here. otherwise, if there was a problem with the
        // attribute, then don't fill out the rest of the attrib fields and move on to the next one.
        if (status == ZCL_STATUS_INSUFF_SPACE)
        {
            break;
        }
        else if (status != ZCL_STATUS_SUCCESS)
        {
            continue;
        }

        *resp_ptr++ = attrib->type;
        switch (attrib->type)
        {
        case ZCL_TYPE_U8:
            *resp_ptr++ = *(U8 *)attrib->data;
            break;

        case ZCL_TYPE_U16:
            *(U16 *)resp_ptr = *(U16 *)attrib->data;
            resp_ptr += sizeof(U16);
            break;

        case ZCL_TYPE_U32:
            *(U32 *)resp_ptr = *(U32 *)attrib->data;
            resp_ptr += sizeof(U32);
            break;

        case ZCL_TYPE_BOOL:
            *(U8 *)resp_ptr++ = *(U8 *)attrib->data;
            break;

        case ZCL_TYPE_CHAR_STRING:
            // string length is contained in the first byte of data. So add 1 to str_len
            str_len = (*(U8 *)attrib->data) + 1;
            memcpy(resp_ptr, (U8 *)attrib->data, str_len);
            resp_ptr += str_len;
            break;
        }
    }

    // return the length of the response
    return (resp_ptr - resp);
}

/**************************************************************************/
/*!
        This function implements the ZCL write attribute foundation command. The
        write attribute command can either write a single attribute or a series
        of attributes and will return a response based on the write status.
*/
/**************************************************************************/
static U8 zcl_cmd_write_attrib(U8 *resp, U16 src_addr, U8 src_ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    U8 size, len, type, *data_ptr, status, *resp_ptr;
    U16 attrib_id;
    zcl_attrib_t *attrib;
    zcl_hdr_t hdr_out;

    // init the data and response pointers
    resp_ptr = resp;
    data_ptr = hdr->payload;

    // generate the header. most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(zcl_hdr_t));
    hdr_out.frm_ctrl.dir    = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd             = ZCL_CMD_WRITE_ATTRIB_RESP;

    // this is where the header actually gets generated
    len = zcl_gen_hdr(resp_ptr, &hdr_out);
    resp_ptr += len;

    while ((data_ptr - hdr->payload) < hdr->payload_len)
    {
        // init the status by setting an invalid value
        status = ZCL_STATUS_SUCCESS;

        attrib_id = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        // find attribute
        if ((attrib = zcl_find_attrib(clust->attrib_list, attrib_id)) == NULL)
        {
            status = ZCL_STATUS_UNSUP_ATTRIB;
            type = ZCL_TYPE_INVALID;
        }
        else
        {
            // check that the type matches the type on the attribute
            type = *(U8 *)data_ptr++;
            if (type != attrib->type)
            {
                status = ZCL_STATUS_INVALID_TYPE;
            }

            if (attrib->access == ZCL_ACCESS_READ_ONLY)
            {
                status = ZCL_STATUS_READ_ONLY;
            }

            if (!zcl_attrib_check_space_avail(attrib, resp_ptr - resp))
            {
                status = ZCL_STATUS_INSUFF_SPACE;
            }
        }

        // add status and attrib id to response
        *resp_ptr++        = status;
        *(U16 *)resp_ptr   = attrib_id;
        resp_ptr += sizeof(U16);

        // if we're out of room, then end the response frame here. otherwise if we ran into a problem
        // with the attrib, then update the status and move on to the next one.
        if (status == ZCL_STATUS_INSUFF_SPACE)
        {
            break;
        }
        else if (status != ZCL_STATUS_SUCCESS)
        {
            continue;
        }

        // update the attrib value. if the attrib was not found, then
        // don't update, but move the data pointer along to the next attrib to be udpated.
        switch (type)
        {
        case ZCL_TYPE_U8:
            *(U8 *)attrib->data = *data_ptr++;
            break;

        case ZCL_TYPE_U16:
            *(U16 *)attrib->data = *(U16 *)data_ptr;
            data_ptr += sizeof(U16);
            break;

        case ZCL_TYPE_U32:
            *(U32 *)attrib->data = *(U32 *)data_ptr;
            data_ptr += sizeof(U32);
            break;

        case ZCL_TYPE_BOOL:
            *(bool *)attrib->data = *(bool *)data_ptr++;
            break;

        case ZCL_TYPE_CHAR_STRING:
            // byte 0 is the length of the string
            size = *(U8 *)data_ptr++;

            // start the copy from byte 1
            memcpy(attrib->data, data_ptr, size);
            data_ptr += size;
            break;
        default:
            continue;
        }
    }

    // add the len to the response buffer and move the data pointer to the beginning of the data
    return (resp_ptr - resp);
}

/**************************************************************************/
/*!
        This is the discover attribute foundation ZCL command. It will discover
        the attributes that are available for a particular endpoint and return
        a response with the attributes that are present.

    Note: This one will always signal that all of the attributes have been
    discovered.
*/
/**************************************************************************/
static U8 zcl_cmd_disc_attrib(U8 *resp, U16 src_addr, U8 src_ep, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    U8 i, len, max_attribs, *data_ptr, *resp_ptr, *disc_comp_field;
    U16 attrib_id;
    zcl_attrib_t *attrib;
    zcl_hdr_t hdr_out;

    // init the data and response pointers
    resp_ptr = resp;
    data_ptr = hdr->payload;

    // generate the header. most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(zcl_hdr_t));
    hdr_out.frm_ctrl.dir    = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd             = ZCL_CMD_DISC_ATTRIB_RESP;

    // this is where the header actually gets generated
    len = zcl_gen_hdr(resp_ptr, &hdr_out);
    resp_ptr += len;

    // get the start attribute and max attribs
    attrib_id = *(U16 *)data_ptr;
    data_ptr += sizeof(U16);
    max_attribs = *data_ptr++;

    // keep a variable pointing to the state of the discovery status field. it might
    // need to change later on if we can't accomodate all the attribute responses
    disc_comp_field = resp_ptr++;
    *disc_comp_field = ZCL_STATUS_DISC_COMPLETE;

    // iterate through each requested attrib
    for (i=0; i<max_attribs; i++)
    {
        if ((resp_ptr - resp) >= (ZCL_MAX_PAYLOAD_SIZE - 3))
        {
            // we don't want to exceed the size of the response payload so terminate the response
            // if we exceed the max payload size
            *disc_comp_field = ZCL_STATUS_DISC_INCOMPLETE;
            break;
        }

        if ((attrib = zcl_find_attrib(clust->attrib_list, attrib_id)) != NULL)
        {
            *(U16 *)resp_ptr = attrib->id;
            resp_ptr += sizeof(U16);
            *resp_ptr++ = attrib->type;
        }
        attrib_id++;
    }
    return (resp_ptr - resp);
}

/**************************************************************************/
/*!
        This function configures a reporting entry. Reporting entries are used
        to report on an attribute value at specified time intervals. Multiple
        attributes can be configured for reporting. All attributes that are
        configured to report their status will go into a reporting table that is
        checked once every second and their expiry is decremented. When the expiry
        goes to zero, a reporting frame will be generated. Only one attribute is reported
        per report frame.
*/
/**************************************************************************/
static U8 zcl_cmd_cfg_rpt(U8 *resp, U16 src_addr, U8 src_ep, zcl_clust_t *clust, U16 prof_id, zcl_hdr_t *hdr)
{
    U8 *data_ptr, *resp_ptr, dir, type, len, status;
    U16 attrib_id, max_intv, min_intv;
    U32 change = 0;
    zcl_attrib_t *attrib;
    zcl_hdr_t hdr_out;

    data_ptr = hdr->payload;
    resp_ptr = resp;

    // first gen the response header most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(zcl_hdr_t));
    hdr_out.frm_ctrl.dir    = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd             = ZCL_CMD_CONFIG_REPORT_RESP;

    len = zcl_gen_hdr(resp_ptr, &hdr_out);
    resp_ptr += len;

    // parse the attrib fields
    while ((data_ptr - hdr->payload) < hdr->payload_len)
    {
        dir         = *data_ptr++;
        attrib_id   = *(U16 *)data_ptr;
        data_ptr += sizeof(U16);

        if ((attrib = zcl_find_attrib(clust->attrib_list, attrib_id)) == NULL)
        {
            // we can't figure out how to recover from a nonexistent attribute because the size of the change field
            // is dependent on the attribute's type.
            DBG_PRINT_RAW("ZCL CFG RPT: ERROR...nonexistent attribute.\n");
            status = ZCL_STATUS_UNSUP_ATTRIB;
        }

        // we are only expecting to send out reports. If they try and configure us to receive reports, then figure out
        // the size of the fields and then skip over them. otherwise, just return.
        if (dir)
        {
            if (attrib)
            {
                // move the data pointer up by the proper amount of bytes and then restart the loop
                len = zcl_get_attrib_size(attrib);
                data_ptr += len + 2;

                // fill out the resp payload
                *resp_ptr++         = ZCL_STATUS_MALFORMED_CMD;
                *resp_ptr++         = dir;
                *(U16 *)resp_ptr    = attrib_id;
                continue;
            }
            else
            {
                // if no attrib can be found and the direction is not zero, then we can't figure out the distance to move the
                // data ptr. in that case, just quit out of here.
                *resp_ptr++         = ZCL_STATUS_UNSUP_ATTRIB;
                *resp_ptr++         = dir;
                *(U16 *)resp_ptr    = attrib_id;
                return (resp_ptr - resp);
            }
        }

        type        = *data_ptr++;
        min_intv    = *(U16 *)data_ptr;
        data_ptr    += sizeof(U16);
        max_intv    = *(U16 *)data_ptr;
        data_ptr    += sizeof(U16);

        // report on change - this is currently not supported
        //switch (type)
        //{
        //case ZCL_TYPE_U8:
        //case ZCL_TYPE_BOOL:
        //case ZCL_TYPE_8BIT:
        //case ZCL_TYPE_8BITMAP:
        //    change = *data_ptr++;
        //    break;
        //
        //case ZCL_TYPE_U16:
        //case ZCL_TYPE_16BIT:
        //case ZCL_TYPE_16BITMAP:
        //    change = *(U16 *)data_ptr;
        //    data_ptr += sizeof(U16);
        //    break;
        //
        //case ZCL_TYPE_U32:
        //case ZCL_TYPE_32BIT:
        //case ZCL_TYPE_32BITMAP:
        //    change = *(U32 *)data_ptr;
        //    data_ptr += sizeof(U32);
        //    break;
        //}

        if (attrib)
        {
            status = zcl_rpt_add(src_addr, src_ep, clust->clust_id, attrib, prof_id, max_intv, change);
        }

        *resp_ptr++ = status;
        *resp_ptr++ = dir;
        *(U16 *)resp_ptr = attrib_id;
        resp_ptr += sizeof(U16);
    }

    // TODO: If all reports are configured successfully, then just return a single status

    return (resp_ptr - resp);
}

/**************************************************************************/
/*!
        This is the main ZCL command handler. When a ZCL frame arrives, it first
        comes here and then gets decoded and sent to the correct function for
        handling.
*/
/**************************************************************************/
void zcl_cmd_handler(U8 *resp, U8 *resp_len, U16 src_addr, U8 src_ep, U16 prof_id, zcl_clust_t *clust, zcl_hdr_t *hdr)
{
    // make sure that the buffers and clusters are present
    if ((clust == NULL) || (hdr == NULL))
    {
        return;
    }

    // decode the cmd and route it to the correct function
    switch (hdr->cmd)
    {
    case ZCL_CMD_READ_ATTRIB:
        *resp_len = zcl_cmd_read_attrib(resp, src_addr, src_ep, clust, hdr);
        break;

    case ZCL_CMD_WRITE_ATTRIB:
        *resp_len = zcl_cmd_write_attrib(resp, src_addr, src_ep, clust, hdr);
        break;

    case ZCL_CMD_WRITE_ATTRIB_NO_RESP:
        zcl_cmd_write_attrib(resp, src_addr, src_ep, clust, hdr);
        *resp_len = 0;
        break;

    case ZCL_CMD_DISC_ATTRIB:
        *resp_len = zcl_cmd_disc_attrib(resp, src_addr, src_ep, clust, hdr);
        break;

    case ZCL_CMD_CONFIG_REPORT:
        *resp_len = zcl_cmd_cfg_rpt(resp, src_addr, src_ep, clust, prof_id, hdr);
        break;
    }
    return;
}

