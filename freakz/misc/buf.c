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
        \file buf.c
                \ingroup misc
        \brief Frame buffer controller

        Manage the frame buffer pool.
    */
    /**************************************************************************/
#include <string.h>
#include "freakz.h"

static buffer_t buf_pool[MAX_BUF_POOL_SIZE];    ///< Creates the frame buffer pool

/**************************************************************************/
/*!
    Init the buffer pool.
*/
/**************************************************************************/
void buf_init()
{
    U8 i;

    for (i=0; i<MAX_BUF_POOL_SIZE; i++)
    {
        memset(&buf_pool[i], 0, sizeof(buffer_t));
    }
}

/**************************************************************************/
/*!
    Return the number of bufs currently allocated.
*/
/**************************************************************************/
U8 buf_get_cnt()
{
    U8 i;
    U8 buf_cnt = 0;

    for (i=0; i<MAX_BUF_POOL_SIZE; i++)
    {
        if (buf_pool[i].alloc)
        {
            buf_cnt++;
        }
    }
    return buf_cnt;
}

/**************************************************************************/
/*!
    Allocate and return a pointer to a frame buffer.
*/
/**************************************************************************/
buffer_t *buf_get(U8 tx_rx)
{
    U8 i;
    for (i=0; i<MAX_BUF_POOL_SIZE; i++)
    {
        if (!buf_pool[i].alloc)
        {
            if (tx_rx)
            {
                buf_pool[i].dptr = &buf_pool[i].buf[aMaxPHYPacketSize];
            }
            else
            {
                buf_pool[i].dptr = &buf_pool[i].buf[0];
            }
            buf_pool[i].len = 0;
            buf_pool[i].index = i;
            buf_pool[i].alloc = true;
            return &buf_pool[i];
        }
    }
    return NULL;
}

/**************************************************************************/
/*!
    Free a buffer that has been allocated.
*/
/**************************************************************************/
void buf_free(buffer_t *buf)
{
    if (buf)
    {
        buf->alloc = false;
    }
}










