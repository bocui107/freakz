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
    \file zcl_parse.c
    \ingroup zcl
    \brief ZCL frame parser

    Each layer contains two files called xxx_gen
    and xxx_parse. These files are used to generate and parse the
    layer formatting.

    This file handles the parsing of the ZCL frame.
*/
/*******************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
    Parse the ZCL into a structure that makes it easier for us to process.
*/
/**************************************************************************/
void zcl_parse_hdr(U8 *data, U8 len, zcl_hdr_t *hdr)
{
    U8 *data_ptr;

    if (data)
    {
        data_ptr = data;

        // get the fcf for further decoding
        hdr->fcf = *data_ptr++;

        // decode the fcf and fill out the frm ctrl structure
        hdr->frm_ctrl.frm_type      = (hdr->fcf & ZCL_FRM_TYPE_MASK)    >> ZCL_FRM_TYPE_OFF;
        hdr->frm_ctrl.manuf_spec    = (hdr->fcf & ZCL_MANUF_SPEC_MASK)  >> ZCL_MANUF_SPEC_OFF;
        hdr->frm_ctrl.dir           = (hdr->fcf & ZCL_DIR_MASK)         >> ZCL_DIR_OFF;
        hdr->frm_ctrl.dis_def_resp  = (hdr->fcf & ZCL_DIS_DEF_MASK)     >> ZCL_DIS_DEF_OFF;

        // get the rest of the fields
        if (hdr->frm_ctrl.manuf_spec)
        {
            hdr->manuf_code = *(U16 *)data_ptr;
            data_ptr += sizeof(U16);
        }

        hdr->seq_num        = *data_ptr++;
        hdr->cmd            = *data_ptr++;
        hdr->payload        = data_ptr;
        hdr->payload_len    = len - (data_ptr - data);
    }
}
