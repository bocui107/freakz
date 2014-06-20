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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "type.h"
#include "sim.h"

static struct pipe_t pp;
extern int errno;
static LIST_HEAD(node_list);

static U8 conn_map[MAXNODES][MAXNODES] = {
	{0, 1, 1, 0, 0, 0, 0,},   // node 1
	{1, 0, 0, 0, 0, 1, 1,},   // node 2
	{1, 0, 0, 1, 1, 0, 0,},   // node 3
	{0, 0, 1, 0, 0, 0, 0,},   // node 4
	{0, 0, 1, 0, 0, 0, 0,},   // node 5
	{0, 1, 0, 0, 0, 0, 0,},   // node 6
	{0, 1, 0, 0, 0, 0, 0,}    // node 7
};

void *sim_data_out_thread(void *node)
{
	struct sim_node_t nd;
	struct sim_node_t *sibling;
	U8 i;

	/* copy the node data into the node structure. */
	memcpy(&nd, node, sizeof(struct sim_node_t));

	while (1)
	{
		if (read(nd.data_out.pipe, nd.buf, sizeof(nd.buf)) == -1)
			perror("read pipe");

		/*
		 * create a separate statement block to print out
		 * the contents of the data to the sim console.
		 */
		printf("SIM: Data out from node %d.\n", nd.index);
		for (i = 0; i < nd.buf[0]; i++)
			printf("%02x ", nd.buf[i]);
		printf("\n");

		/* this is where the magic happen */
		list_for_each_entry(sibling, &node_list, list)
		{
			/*
			 * if the indices are less than maxnodes, then process them
			 * according to the connection map otherwise, just skip over them.
			 */
			if ((nd.index - 1 <= MAXNODES) && (sibling->index - 1 <= MAXNODES))
				if (conn_map[nd.index - 1][sibling->index - 1])
					if ((write(sibling->data_in.pipe, nd.buf, nd.buf[0])) == -1)
						perror("write");
		}
	}
}

void *sim_cmd_out_thread(void *node)
{
	int status;
	struct sim_node_t nd;
	struct sim_node_t *sibling;
	U8 len;
	struct cli_buf_t *cli_buf = cli_buf_get();

	/* copy the node data into the node structure. */
	memcpy(&nd, node, sizeof(struct sim_node_t));

	while (1)
	{
		/* block until data comes into the sim from the node */
		if (read(nd.cmd_out.pipe, nd.cmdbuf, sizeof(nd.cmdbuf)) == -1)
			perror("cmd read pipe");

		/*
		 * sleep for a while after something is read. this will give the
		 * script the chance to get the next instruction and prepare the
		 * wait statement if it exists. this delay was added because of a
		 * race condition in between executing the script command and preparing
		 * for checking the wait condition.
		 */
		usleep(100000);

		/*
		 * lock the mutex and copy the data into the client buffer.
		 * once thats finished, signal the cli that data has arrived
		 * and then unlock the mutex
		 */
		status = pthread_mutex_lock(&cli_buf->mutex);
		if (status)
			perror("cmd out lock mutex");

		/*
		 * the first byte of the message distinguishes whether
		 * its a cmd string or data. Choose the len based on this.
		 */
		if (nd.cmdbuf[0] == 0xff)
		{
			len = sizeof(nd.cmdbuf);
			cli_buf->data = false;
		} else {
			len = nd.cmdbuf[0];
			cli_buf->data = true;
		}
		memcpy(cli_buf->buf, &nd.cmdbuf[1], len);

		/* finish off the thread conditional wait pattern */
		if ((status = pthread_cond_signal(&cli_buf->cond)) != 0)
			perror("cmd out signal cond");

		if ((status = pthread_mutex_unlock(&cli_buf->mutex)) != 0)
			perror("cmd out unlock mutex");

		/* debug dump of the data */
		printf("DEBUG: ");
		if (nd.cmdbuf[0] == 0xff)
		{
			printf("%s", cli_buf->buf);
		} else {
			U8 i;

			for (i = 0; i < len; i++)
				printf("%02x ", cli_buf->buf[i]);
		}

		printf("\n");
		fflush(stdout);
	}
}

void sim_send_data(char *msg, pid_t sender)
{
	struct sim_node_t *nd;

	list_for_each_entry(nd, &node_list, list)
	{
		/* the length of the transfer is in the 1st byte of the frame */
		if ((write(nd->data_in.pipe, msg, strlen(msg) + 1)) == -1)
			perror("write");
	}
}

void sim_send_cmd(char *msg, U8 index)
{
	struct sim_node_t *nd;

	list_for_each_entry(nd, &node_list, list)
	{
		if (nd->index == index)
		{
			if ((write(nd->cmd_in.pipe, msg, strlen(msg) + 1)) == -1)
				printf("PID %d CMD Write Failed.\n", getpid());
			return;
		}
	}
}

void sim_list_print(void) {
	struct sim_node_t *node;

	printf("Current node PIDs are:\n");
	list_for_each_entry(node, &node_list, list)
		printf("Node Index = %d, PID = %d.\n", node->index, node->pid);

	printf("\n");
}

void sim_add_node(U8 index)
{
	pid_t pid, w;
	int status;
	struct sim_node_t *nd, *child;
	char msg[ARGVMAX];

	list_for_each_entry(nd, &node_list, list)
	{
		if (nd->index == index)
		{
			printf("ADD_NODE: Duplicate index. Cannot add.\n");
			return;
		}
	}

	/* alloc the node descriptor */
	nd = (struct sim_node_t *)malloc(sizeof(struct sim_node_t));
	if (!nd) {
		printf("Malloc failed.\n");
		return;
	}

	memset(nd, 0, sizeof(struct sim_node_t));

	/* fork the process */
	pid = fork();

	switch (pid)
	{
	case -1:
		printf("Failed to fork.\n");
		exit(13);
		break;
	case 0:
		sprintf(msg, "xterm -title 'Node %d' -e ./test_sim.native %d", index, index);
		/* system: run the shell script */
		system(msg);
		exit(EXIT_SUCCESS);
		break;
	default:
		if ((pp.pipe = open(pp.name, O_RDONLY)) < 0)
			perror("open public pipe");

		if (read(pp.pipe, msg, ARGVMAX) == -1)
			perror("read public pipe");

		close(pp.pipe);

		/* write the pid contents to the node struct */
		nd->pid = strtol(msg, NULL, 10);
		nd->index = index;
		printf("PID = %d. Index = %d.\n", nd->pid, nd->index);

		/* delay for one second so that node can create the pipes */
		sleep(1);

		/* open the pipes for the child node */
		sprintf(nd->data_in.name, "./fifo/fifo_in_%d", nd->pid);
		if ((nd->data_in.pipe = open(nd->data_in.name, O_WRONLY)) < 0)
			perror("data_in open pipe");

		sprintf(nd->cmd_in.name, "./fifo/fifo_cmd_in_%d", nd->pid);
		if ((nd->cmd_in.pipe = open(nd->cmd_in.name, O_WRONLY)) < 0)
			perror("cmd_in open pipe");

		sprintf(nd->data_out.name, "./fifo/fifo_out_%d", nd->pid);
		if ((nd->data_out.pipe = open(nd->data_out.name, O_RDONLY)) < 0)
			perror("data_out open pipe");

		sprintf(nd->cmd_out.name, "./fifo/fifo_cmd_out_%d", nd->pid);
		if ((nd->cmd_out.pipe = open(nd->cmd_out.name, O_RDONLY)) < 0)
			perror("cmd_out open pipe");

		/* create read thread for the child node's out pipe */
		if (pthread_create(&nd->data_out.thread, NULL, sim_data_out_thread, nd) > 0)
			perror("pthread_create");
		if (pthread_create(&nd->cmd_out.thread, NULL, sim_cmd_out_thread, nd) > 0)
			perror("pthread_create");

		list_add(&node_list, &nd->list);
		break;
	}
}

void static kill_a_node(struct sim_node_t *nd)
{
	kill(nd->pid, SIGTERM);
	pthread_cancel(nd->cmd_out.thread);
	pthread_cancel(nd->data_out.thread);
	pthread_join(nd->cmd_out.thread, NULL);
	pthread_join(nd->data_out.thread, NULL);

	close(nd->data_out.pipe);
	close(nd->data_in.pipe);
	close(nd->cmd_in.pipe);
	close(nd->cmd_out.name);

	/* sleep 1s to wait the process of the pid to exit */
	sleep(1);

	unlink(nd->data_in.name);
	unlink(nd->data_out.name);
	unlink(nd->cmd_in.name);
	unlink(nd->cmd_out.name);

	list_remove(&node_list, &nd->list);
	free(nd);

	printf("Node %d was terminated.\n", nd->index);
}


void sim_kill_nodes(U8 index)
{
	struct sim_node_t *nd;

	list_for_each_entry(nd, &node_list, list)
	{
		if (nd->index == index)
			kill_a_node(nd);
	}
}

static void sim_kill_all_nodes()
{
	char msg[30];
	struct sim_node_t *nd;

	close(pp.pipe);
	unlink(pp.name);
	list_for_each_entry(nd, &node_list, list)
	{
		kill_a_node(nd);
	}
	exit(EXIT_SUCCESS);
}

void sigchld_handler()
{
	struct sim_node_t *nd;
	pid_t pid;
	int status;

	pid = waitpid(-1, &status, WNOHANG);

	list_for_each_entry(nd, &node_list, list)
	{
		if (nd->pid == pid)
			break;
	}

	/*
	 * WIFSIGNALED: 如果子进程是因为信号而结束则此宏值为真
	 * WTERMSIG(status)取得子进程因信号而中止的信号代码，一般会先用WIFSIGNALED 来判断后才使用此宏
	 */
	if(WIFSIGNALED(status) && (WTERMSIG(status) == SIGSEGV))
	{
		if(nd == NULL)
			printf("A node crashed, but it wasn't even started \
				 by the system. Something weird is going on!\n");
		else
			printf("Contiki node %d crashed - Segmentation fault\n", nd->index);
	}
}

int main (int argc, char *argv[])
{
	char msg[50];

	INIT_LIST_HEAD(&node_list);

	/*
	 * register the abort function
	 */
	atexit(sim_kill_all_nodes);

	/*
	 * When the program is abort(interrupt) signal, the user input
	 * INTR character(Ctrl + c) to notify the font process group.
	 */
	signal(SIGINT, sim_kill_all_nodes);

	/*
	 * When a thread is stop or abort, the thread will send
	 * the SIGCHLD to it's parent
	 */
	signal(SIGCHLD, sigchld_handler);

	/*
	 * The directory's permission is 775
	 */
	mkdir("./log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sprintf(msg, "./log/sim.txt");
	/*
	 * reallocal the file point or redirect the point.
	 */
	freopen(msg, "w", stderr);

	/* create the  public pipe and open it for reading */
	mkdir("./fifo", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	strcpy(pp.name, "./fifo/PUBLIC");

	/*
	 * mknod: Create a pipe of the name
	 */
	if (mknod(pp.name, S_IFIFO | 0666, 0) == -1)
		perror("mknod");

	/* go to the command line interface.*/
	cli();

	return(0);
}
