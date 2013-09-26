/*******************************************************************
    Copyright (C) 2008 Christopher Wang

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

    Email: chris@freaklabs.org
*******************************************************************/
/*******************************************************************
    Author: Christopher Wang

    Title: node.c

    Description:
    The forked individual node.
*******************************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "sim.h"
#include "type.h"

static sim_node_t self_node;

sim_node_t *get_self()
{
	return &self_node;
}

void rcv_msg()
{
	struct timeval tv;
	fd_set fdset;
	int ret;

	while (1)
	{
		tv.tv_sec = 0;
		tv.tv_usec = 1000;

		FD_ZERO(&fdset);
		FD_SET(self_node.fd[0], &fdset);

		ret = select(self_node.fd[0] + 1, &fdset, NULL, NULL, &tv);
		if (FD_ISSET(self_node.fd[0], &fdset))
		{
			if ((read(self_node.fd[0], self_node.buf, 100)) != -1)
			{
				printf("PID %d Msg Rcvd: %s\n", getpid(), self_node.buf);
				fflush(stdout);
			}
		}
	}
}
