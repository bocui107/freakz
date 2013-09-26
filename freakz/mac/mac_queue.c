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
    \file mac_queue.c
    \ingroup mac
    \brief MAC rx queue controller

    This file handles the MAC's rx queue. When a valid frame comes into the
    radio, the rx ISR will be called and the frame will be added to the queue.
    After that, an event will be posted to the mac process and the mac's eventhandler
    will pull the frame from the rx queue.
*/
/**************************************************************************/
#include "freakz.h"

/**************************************************************************/
/*!
        List head for the MAC rx queue. When an inbound frame arrives, it will
        get put in this queue. An indication to the MAC process is then sent
        and the MAC will pull out the frame from the queue and begin processing it.
*/
/**************************************************************************/
LIST(mac_rx_queue);

/**************************************************************************/
/*!
    Initialize the list (queue).
*/
/**************************************************************************/
void mac_queue_init()
{
    list_init(mac_rx_queue);
}

/**************************************************************************/
/*!
    Insert a received frame buffer into the queue.
*/
/**************************************************************************/
void mac_queue_buf_insert(buffer_t *buf)
{
    if (buf)
    {
        list_add(mac_rx_queue, buf);
        debug_dump_buf(buf->dptr, buf->len);
    }
}

/**************************************************************************/
/*!
    Remove a received frame buffer from the queue.
*/
/**************************************************************************/
buffer_t *mac_queue_buf_pop()
{
    buffer_t *tmp;

    if ((tmp = list_head(mac_rx_queue)) != NULL)
    {
        list_remove(mac_rx_queue, tmp);
    }
    return tmp;
}

/**************************************************************************/
/*!
    Return a pointer to the queue head.
*/
/**************************************************************************/
buffer_t *mac_queue_get_head()
{
    return list_head(mac_rx_queue);
}
/**************************************************************************/
/*!
    Check if there are any frames in the queue. If there are, the function
    returns false. Otherwise, return true for an empty queue.
*/
/**************************************************************************/
bool mac_queue_is_empty()
{
    return (list_head(mac_rx_queue) == NULL);
}

/**************************************************************************/
/*!
    Remove all entries from the queue.
*/
/**************************************************************************/
void mac_queue_clear()
{
    buffer_t *buf;

    for (buf = list_chop(mac_rx_queue); buf != NULL; buf = list_chop(mac_rx_queue))
    {
        buf_free(buf);
    }
}

