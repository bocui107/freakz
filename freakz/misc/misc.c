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
        \defgroup misc MISC - Miscellaneous files
    \file misc.c
    \ingroup misc
    \brief Miscellaneous functions

    Miscellaneous functions used by the stack.
*/
/**************************************************************************/
#include "freakz.h"

static struct timer tmr;        ///< Just your ol' everyday software timer used for a busy wait

/**************************************************************************/
/*!
    Perform a busy wait. This wait hogs the CPU while waiting so it shouldn't
    be used for long periods or too often. The argument is in milliseconds.
*/
/**************************************************************************/
void busy_wait(U16 msec)
{
    timer_set(&tmr, msec);
    while (!timer_expired(&tmr))
    {
        ;
    }
}

///**************************************************************************/
///*!
//    Convert a 2-byte array to an int
//*/
///**************************************************************************/
//U16 array2int(U8 *array)
//{
//    U16 data = 0;
//
//    data = (array[1] << 8) | array[0];
//    return data;
//}
//
///**************************************************************************/
///*!
//    Convert an 8-byte array to an unsigned long long
//*/
///**************************************************************************/
//U64 array2ull(U8 *array)
//{
//    U8 i;
//    U64 data = 0;
//
//    for (i=7; i>=0; i--)
//    {
//        data |= array[i] << ((8 * i));
//    }
//    return data;
//}

#if (TEST_SIM == 1)
/**************************************************************************/
/*!
    This function is a compile option only if in simulator mode. It adds a
    null terminator to a string. Assumes that the string is initially terminated
    with a newline.
*/
/**************************************************************************/
void add_null_term(U8 *msg)
{
    U8 *p = msg;

    while (*p != '\n')
    {
        p++;
    }
    *p = '\0';
}

/**************************************************************************/
/*!
    This is a compile option and is only used for the simulator. Format a
    string to be sent out the cmd pipe. The first byte must contain the
    code 0xff and it must be null terminated.
*/
/**************************************************************************/
void format_cmd_str(U8 *msg)
{
	U8 tmp[128];

	add_null_term(msg);
	tmp[0] = 0xff;
	strcpy((char *)&tmp[1], (char *)msg);
	strcpy((char *)msg, (char *)tmp);
}
#endif
