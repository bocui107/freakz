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
    \file types.h
    \ingroup misc
    \brief Pre-defined data types

    These are the generically defined data types that will be used
    in the stack.
*/
/*******************************************************************/
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifndef DATA_TYPES
#define DATA_TYPES
// Standard data types
typedef uint8_t             U8;     ///< Generic 8 bit unsigned data type
typedef uint16_t            U16;    ///< Generic 16 bit unsigned data type
typedef uint32_t            U32;    ///< Generic 32 bit unsigned data type

typedef int8_t              S8;     ///< Generic 8 bit signed data type
typedef int16_t             S16;    ///< Generic 16 bit signed data type
typedef int32_t             S32;    ///< Generic 32 bit signed data type
#endif

typedef uint64_t            U64;    ///< Generic 64 bit unsigned data type

/**************************************************************************/
/*!
    Enumeration of TX and RX used in the stack.
*/
/**************************************************************************/
enum TXRX
{
    RX = 0,                     ///< RX enum value
    TX = 1                      ///< TX enum value
};

/**************************************************************************/
/*!
    Address type. This is a union containing both the short and long addresses.
    They can be differentiated by checking the mode.
*/
/**************************************************************************/
typedef struct
{
    U8 mode;                ///< Address mode: short or long address
    union
    {
        U16 short_addr;     ///< Short address value
        U64 long_addr;      ///< Long address value
    };
} address_t;

#endif // TYPES_H



