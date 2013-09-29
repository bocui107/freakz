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
/*******************************************************************
    Author: Christopher Wang

    Title:

    Description:
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "test_sim.h"
#include "test_app.h"
#include "contiki-main.h"
#include "freakz.h"
#include "sim_drvr.h"
#include "test_app.h"

extern process_event_t event_data_in;
extern process_event_t event_cmd_rcvd;
extern int errno;
static sim_node_t node;             // node struct that holds info related to node communications
static pipe_t pp;                   // public pipe used to initially communicate with sim shell
static char cmd[BUFSIZE];
FILE *fp;
FILE *fout;

/* Threads to handle the pipe communications */

/**************************************************************************
Function Name: sim_data_in_thread

Description:
This thread is used to monitor the data_in pipe for incoming frames.

CAUTION: Threads should only call reentrant functions or else bad things can happen.
**************************************************************************/
static void *sim_data_in_thread(void *dummy)
{
	static char msg[BUFSIZE];

	while (1)
	{
		if (read(node.data_in.pipe, msg, BUFSIZE) == -1)
			perror("sim_data_in_thread");

		/*
		 * write the received data into the input buffer
		 * of the stack and trigger the isr
		 */
		drvr_write_rx_buf((U8 *)msg, msg[0]);
		drvr_rx_isr();
	}
	return NULL;
}

/**************************************************************************
Function Name: sim_cmd_in_thread

Description:
This thread is used to monitor the cmd_in pipe for incoming commands issued
from the simulator shell. When the commands appear, it will save them in a static
variable and post an event to the zdo cmd process with a pointer to the cmd string.

CAUTION: Threads should only call reentrant functions or else bad things can happen.
**************************************************************************/
static void *sim_cmd_in_thread(void *dummy)
{
	while (1)
	{
		if (read(node.cmd_in.pipe, cmd, sizeof(cmd)) == -1)
			perror("sim_cmd_in_thread");
		test_app_parse(cmd);
	}
	return NULL;
}

/* Send the tx to the data_out pipe */
void sim_pipe_data_out(U8 *data, U8 len)
{
	if (write(node.data_out.pipe, data, len) == -1)
		perror("sim_pipe_data_out");
	usleep(30);
}

/* Send the cmd to the cmd_out pipe */
void sim_pipe_cmd_out(U8 *data, U8 len)
{
	if (write(node.cmd_out.pipe, data, len) == -1)
		perror("sim_pipe_cmd_out");
}

/* Take care of the signals that come in */
static void sigint_handler()
{
	switch (errno) {
	case EEXIST:
		perror("node eexist");
		break;
	case EINTR:
		close(node.data_in.pipe);
		close(node.data_out.pipe);
		close(node.cmd_in.pipe);
		pthread_cancel(node.data_in.thread);
		pthread_cancel(node.data_out.thread);
		pthread_cancel(node.cmd_in.thread);
		pthread_cancel(node.cmd_out.thread);
		fclose(fp);
		exit(EXIT_SUCCESS);
		break;
	default:
		perror("node other");
		break;
	}
}

/* Get a pointer to the sim node structure */
sim_node_t *node_get()
{
	return &node;
}

static void sigkill_handler()
{
	close(node.data_in.pipe);
	close(node.data_out.pipe);
	close(node.cmd_in.pipe);
	pthread_cancel(node.data_in.thread);
	pthread_cancel(node.data_out.thread);
	pthread_cancel(node.cmd_in.thread);
	pthread_cancel(node.cmd_out.thread);
	fclose(fp);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	char msg[BUFSIZE];
	int index = strtol(argv[1], NULL, 10);

	sprintf(msg, "./log/node_%03d.txt", index);
	fp = fopen(msg, "w");

	sprintf(msg, "./log/node_%03d_data.txt", index);
	fout = fopen(msg, "w");

	sprintf(msg, "./log/node_%03d_err.txt", index);
	freopen(msg, "w", stderr);

	/* opening public fifo */
	strcpy(pp.name, "./fifo/PUBLIC");
	pp.pipe = open(pp.name, O_WRONLY);
	if (pp.pipe == -1)
		perror("open public pipe");

	/* sending pid to public fifo */
	sprintf(msg, "%d", getpid());
	if (write(pp.pipe, msg, strlen(msg) + 1) == -1)
		perror("write");

	/* initialize the communication pipes */
	/* making private fifos */
	sprintf(node.data_in.name, "./fifo/fifo_in_%d", getpid());
	if (mknod(node.data_in.name, S_IFIFO | 0666, 0) < 0)
		perror("mknod");

	sprintf(node.data_out.name, "./fifo/fifo_out_%d", getpid());
	if (mknod(node.data_out.name, S_IFIFO | 0666, 0) < 0)
		perror("mknod");

	sprintf(node.cmd_in.name, "./fifo/fifo_cmd_in_%d", getpid());
	if (mknod(node.cmd_in.name, S_IFIFO | 0666, 0) < 0)
		perror("mknod");

	sprintf(node.cmd_out.name, "./fifo/fifo_cmd_out_%d", getpid());
	if (mknod(node.cmd_out.name, S_IFIFO | 0666, 0) < 0)
		perror("mknod");

	/* opening private fifos */
	if ((node.data_in.pipe  = open(node.data_in.name,   O_RDONLY)) == -1)
		perror("open data_in pipe");
	if ((node.cmd_in.pipe   = open(node.cmd_in.name,    O_RDONLY)) == -1)
		perror("open cmd_in pipe");
	if ((node.data_out.pipe = open(node.data_out.name,  O_WRONLY)) == -1)
		perror("open data_out pipe");
	if ((node.cmd_out.pipe  = open(node.cmd_out.name,   O_WRONLY)) == -1)
		perror("open cmd_out pipe");

	/* start a thread to wait for incoming messages */
	if (pthread_create(&node.data_in.thread, NULL, sim_data_in_thread, NULL) > 0)
		perror("pthread_create");

	if (pthread_create(&node.cmd_in.thread, NULL, sim_cmd_in_thread, NULL) > 0)
		perror("pthread_create");

	/* register the signal handler */
	signal(SIGINT, sigint_handler);
	signal(SIGKILL, sigkill_handler);

	/* set up node parameters */
	node.pid = getpid();
	node.index = index;

	/* jump to the main contiki loop */
	sprintf(msg, "node %d added\n", node.index);
	format_cmd_str((U8 *)msg);
	sim_pipe_cmd_out((U8 *)msg, strlen(msg) + 1);

	contiki_main();
	return 0;
}
