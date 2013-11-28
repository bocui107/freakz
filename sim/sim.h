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
/*******************************************************************
    Author: Christopher Wang

    Title:

    Description:
*******************************************************************/
#ifndef SIM_H
#define SIM_H

#include "type.h"
#include "cli.h"
#include "list.h"

#define MSGBUFSIZE	512
#define ARGVMAX		128
#define MAXNODES	7

struct pipe_t
{
	int pipe;
	char name[30];
	pthread_t thread;
};

struct sim_node_t
{
	struct list_head *list;
	int	pid;
	int	index;
	U16	addr;
	struct pipe_t	data_in;
	struct pipe_t	data_out;
	struct pipe_t	cmd_in;
	struct pipe_t	cmd_out;
	U8	buf[ARGVMAX];
	U8	cmdbuf[ARGVMAX];
};

enum {
	READPIPE,
	WRITEPIPE
};

void cli();
void sim_add_node(U8 index);
void sim_send_data(char *msg, pid_t sender);
void sim_send_cmd(char *msg, U8 index);
#endif
