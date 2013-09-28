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
    \file cdc.h
    \ingroup cdc_class
*/
/*******************************************************************/
#ifndef CDC_H
#define CDC_H

#define NUM_EPS             4
#define MAX_BUF_SZ          64
#define LINE_CODE_SZ        7

// cdc definitions
#define SEND_ENCAPS_CMD     0x00
#define GET_ENCAPS_CMD      0x01
#define SET_COMM_FEATURE    0x02
#define GET_COMM_FEATURE    0x03
#define SET_LINE_CODING     0x20
#define GET_LINE_CODING     0x21
#define SET_CTRL_LINE_STATE 0x22
#define SEND_BRK            0x23

// line coding
#define FMT_1_STOP_BIT      0
#define FMT_1_5_STOP_BITS   1
#define FMT_2_STOP_BITS     2
#define PARITY_NONE         0
#define PARITY_ODD          1
#define PARITY_EVEN         2
#define PARITY_MARK         3
#define PARITY_SPACE        4

// ep definitions
#define CDC_EP_IN           1
#define CDC_EP_OUT          3
#define CDC_EP_INTP         2

// misc
#define LINE_CODE_LEN       7

void cdc_init();
void cdc_req_handler();
void cdc_rx_handler();
void cdc_reg_rx_handler(void (*rx)());

#endif // CDC_H

