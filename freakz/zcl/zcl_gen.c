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
    \file zcl_gen.c
    \ingroup zcl
    \brief ZCL frame generator

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file is used to generate ZCL specific commands.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        Generate the ZCL frame control field.
*/
/**************************************************************************/
U8 zcl_gen_fcf(zcl_hdr_t *hdr)
{
    U8 fcf = 0;

    fcf |= (hdr->frm_ctrl.frm_type & 0x3);
    fcf |= (hdr->frm_ctrl.manuf_spec & 0x1)     << ZCL_MANUF_SPEC_OFF;
    fcf |= (hdr->frm_ctrl.dir & 0x1)            << ZCL_DIR_OFF;
    fcf |= (hdr->frm_ctrl.dis_def_resp & 0x1)   << ZCL_DIS_DEF_OFF;
    return fcf;
}

/**************************************************************************/
/*!
        Generate the ZCL frame header. The empty data array that will be used for
        the command gets passed in and it will be filled out. The size of the
        header gets passed back to the caller.
*/
/**************************************************************************/
U8 zcl_gen_hdr(U8 *data, zcl_hdr_t *hdr)
{
    U8 *data_ptr;

    data_ptr = data;
    *data_ptr++ = zcl_gen_fcf(hdr);

    if (hdr->frm_ctrl.manuf_spec)
    {
        *(U16 *)data_ptr = hdr->manuf_code;
        data_ptr += sizeof(U16);
    }

    *data_ptr++ = hdr->seq_num;
    *data_ptr++ = hdr->cmd;

    return (data_ptr - data);
}

/**************************************************************************/
/*!
    Generate the ZCL Read Attributes command. An empty array is passed in
    and it gets filled out in the function. An attribute ID list and the number
        of attributes is required to be passed into the function. Only the ID from
        the attrib list is used and are used to specify the attributes that are to be read.

        The command data is contained in the zcl header that's passed in. When
        finished, the function will return the length of the data.
*/
/**************************************************************************/
U8 zcl_gen_read_attrib(U8 *data, zcl_hdr_t *hdr, U16 *attrib_list, U8 attrib_num)
{
    U8 i, len, *data_ptr;

    // gen the zcl frame header
    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    // gen the payload
    for (i=0; i<attrib_num; i++)
    {
        *(U16 *)data_ptr = attrib_list[i];
        data_ptr += sizeof(U16);
    }

    return (data_ptr - data);
}

/**************************************************************************/
/*!
    Generate the ZCL Write attributes command. The length of the write attribute
        command frame gets passed back to the caller. An attrib list and number of
        attributes needs to be passed into the function. The attrib IDs and values will
        be used from the attrib list and will specify the attrib ID to be written to
        as well as the value to write.
*/
/**************************************************************************/
U8 zcl_gen_write_attrib(U8 *data, zcl_hdr_t *hdr, zcl_attrib_t *attrib_list, U8 attrib_num)
{
    U8 i, len, attrib_len, *data_ptr;
    zcl_attrib_t *attrib;

    // gen the zcl frame header
    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    for (i=0; i<attrib_num; i++)
    {
        attrib = &attrib_list[i];

        // copy the attrib id and the attrib data type
        *(U16 *)data_ptr = attrib->id;
        data_ptr += sizeof(U16);
        *data_ptr++ = attrib->type;

        // get the len of the attribute and then copy it into the data ptr
        attrib_len = zcl_get_attrib_size(attrib);
        memcpy(data_ptr, attrib->data, attrib_len);
        data_ptr += attrib_len;
    }

    return (data_ptr - data);
}

/**************************************************************************/
/*!
        Generate a discover attributes command frame. The starting attribute is the
        attribute ID to start discovery from and the attribute number is the number
        of attributes that you're hoping to discover. Even if you set this value to
        0xff, only the attributes that exist will be returned in the response.
*/
/**************************************************************************/
U8 zcl_gen_disc_attrib(U8 *data, zcl_hdr_t *hdr, U16 start_attrib, U8 attrib_num)
{
    U8 len, *data_ptr;

    // gen the zcl frame header
    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    *(U16 *)data_ptr = start_attrib;
    data_ptr += sizeof(U16);
    *data_ptr++ = attrib_num;

    return (data_ptr - data);
}

/**************************************************************************/
/*!
        Generate a configure report command frame. This frame is used to configure an
        attribute for reporting. The attribute ID, min/max interval, the change value
        must be specified. The min/max interval are the intervals that will be used
        to specify the reporting time. Only the max interval is used for the reporting
        interval so it's recommended to set the min and max interval to the same value.
        The change value is the amount the value needs to change in order to send the report.
        A value of 0 for the change signifies that the report will be sent regardless
        if the value changed or not.
*/
/**************************************************************************/
U8 zcl_gen_config_rpt(U8 *data, zcl_hdr_t *hdr, zcl_clust_t *clust, U16 attrib_id, U16 min_intv, U16 max_intv, U32 change)
{
    U8 len, *data_ptr;
    zcl_attrib_t *attrib;

    // gen the zcl frm hdr
    len = zcl_gen_hdr(data, hdr);
    data_ptr = data + len;

    if ((attrib = zcl_find_attrib(clust->attrib_list, attrib_id)) == NULL)
    {
        return 0;
    }

    *data_ptr++         = ZCL_RPT_DIR_SEND_RPT;
    *(U16 *)data_ptr    = attrib_id;
    data_ptr += sizeof(U16);
    *data_ptr++         = attrib->type;
    *(U16 *)data_ptr    = min_intv;
    data_ptr += sizeof(U16);
    *(U16 *)data_ptr    = max_intv;
    data_ptr += sizeof(U16);

    // report on level change - currently not supported
    //switch (attrib->type)
    //{
    //case ZCL_TYPE_8BIT:
    //case ZCL_TYPE_U8:
    //    *data_ptr++ = change;
    //    break;
    //
    //case ZCL_TYPE_16BIT:
    //case ZCL_TYPE_U16:
    //    *(U16 *)data_ptr = change;
    //    data_ptr += sizeof(U16);
    //    break;
    //
    //case ZCL_TYPE_32BIT:
    //case ZCL_TYPE_U32:
    //    *(U32 *)data_ptr = change;
    //    data_ptr += sizeof(U32);
    //    break;
    //
    //default:
    //    *data_ptr++ = 0;
    //    break;
    //}
    return (data_ptr - data);
}

/**************************************************************************/
/*!
    Send the default response. This is the response when no other response
    is specified and will just mirror the command and the status. It can be
    suppressed by specifying to disable the default response in the incoming
    header.
*/
/**************************************************************************/
U8 zcl_gen_def_resp(U8 *resp, U8 status, zcl_hdr_t *hdr)
{
    U8 *resp_ptr, len;
    zcl_hdr_t resp_hdr;

    resp_ptr = resp;

    resp_hdr.frm_ctrl.dir            = ZCL_FRM_DIR_TO_CLI;
    resp_hdr.frm_ctrl.dis_def_resp   = true;
    resp_hdr.frm_ctrl.frm_type       = ZCL_FRM_TYPE_GENERAL;
    resp_hdr.frm_ctrl.manuf_spec     = false;
    resp_hdr.cmd                     = ZCL_CMD_DEFAULT_RESP;
    resp_hdr.seq_num                 = hdr->seq_num;

    len = zcl_gen_hdr(resp_ptr, &resp_hdr);
    resp_ptr += len;

    // payload is here. 1 byte for cmd, 1 byte for status
    *resp_ptr++ = hdr->cmd;
    *resp_ptr++ = status;

    return resp_ptr - resp;
}

