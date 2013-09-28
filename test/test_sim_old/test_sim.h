/*******************************************************************
    Copyright (C) 2008 FreakLabs

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Originally written by Christopher Wang.
    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title:

    Description:
*******************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include <pthread.h>
#include "types.h"

#define BUFSIZE 128
#define NAMESIZE 50

typedef struct
{
    int pipe;
    char name[NAMESIZE];
    pthread_t thread;
} pipe_t;

typedef struct _sim_node_t
{
    struct _sim_node_t  *next;
    int                 pid;
    int                 index;
    U16                 addr;
    pipe_t              data_in;
    pipe_t              data_out;
    pipe_t              cmd_in;
    pipe_t              cmd_out;
    U8                  buf[BUFSIZE];
} sim_node_t;

void sim_pipe_data_out(U8 *data, U8 len);
void sim_pipe_cmd_out(U8 *data, U8 len);
sim_node_t *node_get();
#endif

