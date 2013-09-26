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

    Title: cli.c

    Description:
    The sim command line interface. The command functions in here
    are just shell functions to call the real functions that do the
    work.
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <pthread.h>

#include "type.h"
#include "sim.h"
#include "node_list.h"
#include "cli.h"

// main file pointer to the current script
FILE *fp;
static cli_buf_t cli_buf =
{
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    0
};

// Main command table
static cmd_t cmd_tbl[] =
{
    {"send",    send_data,},
    {"cmd",     send_cmd,},
    {"add",     add_node,},
    {"kill",    kill_node,},
    {"list",    list_nodes,},
    {"script",  process_script,},
    {"quit",    quit_sim,},
    {NULL, NULL}
};

/**************************************************************************
Function Name: write_cli_buf

Description:

**************************************************************************/
void write_cli_buf(U8 *msg, U8 len)
{
    if (msg[0] == 0xff)
    {
        memcpy(cli_buf.buf, &msg[1], len);
        //printf(cli_buf.buf);
    }
}

/**************************************************************************
Function Name: check_cli_buf

Description:

**************************************************************************/
bool check_cli_buf(U8 *msg)
{
    U8 i, data[2], tmp[256];

    if (cli_buf.data == true)
    {
        for (i=0; i<(strlen(msg)/2); i++)
        {
            sprintf(data, "%02x", cli_buf.buf[i]);
            strncpy(&tmp[i*2], data, 2);
        }
        tmp[i*2] = '\0';
        //printf(tmp);
        return (strcmp(tmp, msg) != 0);
    }
    else
    {
        //printf(cli_buf.buf);
        return (strcmp(cli_buf.buf, msg) != 0);
    }
}

/**************************************************************************
Function Name: parse_string

Description:
Here, we parse the string by comparing it to the commands in the cmd table.
If there is a match, then it will call the function listed, and the function
will take over and process the action.
**************************************************************************/
void parse_string(char *str)
{
    char *cmd;
    U8 i = 0;

    cmd = strtok(str, " ");
    for (i=0; cmd_tbl[i].cmd != NULL; i++)
    {
        if (!strcmp(cmd, cmd_tbl[i].cmd))
        {
            // pass the address of string after the cmd (and the trailing space after the cmd)
            cmd_tbl[i].func(str + strlen(cmd) + 1);
            return;
        }
    }
    printf("Command '%s' not recognized.\n", cmd);
}

/**************************************************************************
Function Name: cli_buf_get

Description:

**************************************************************************/
cli_buf_t *cli_buf_get()
{
    return &cli_buf;
}

/**************************************************************************
Function Name: cli

Description:
This is the main command line interface. It prints the prompt and then waits
in the input. If the input is not empty, then it will call parse_string to
process the command.
**************************************************************************/
void cli()
{
    char msg[MSGBUFSIZE];
    char *msg_ptr = NULL;
    sim_node_t *nd;

    while (1)
    {
        printf("FreakZim>> ");
        fflush(stdout);
        fgets(msg, sizeof(msg), stdin);
        if ((msg_ptr = strchr(msg, '\n')) != NULL)
        {
            *msg_ptr = '\0';
        }

        // if msg ptr is empty, then
        if (strcmp(msg, "\0"))
        {
            parse_string(msg);
        }
    }
}

/**************************************************************************
Function Name: send_data

Description:

**************************************************************************/
void send_data(char *str)
{
    char *msg;

    msg = str;
    sim_send_data(msg, 0);
}

/**************************************************************************
Function Name: send_cmd

Description:

**************************************************************************/
void send_cmd(char *str)
{
        U8 index;
        char *num, *msg;

        num = strtok(str, " ");
        msg = str + strlen(num) + 1;
        index = strtol(num, NULL, 10);
        sim_send_cmd(msg, index);
}

/**************************************************************************
Function Name: add_node

Description:

**************************************************************************/
void add_node(char *str)
{
    U8 index;
    sim_node_t *nd;
    char *tmp;

    if ((tmp = strtok(str, " ")) == NULL)
    {
        printf("Please add an index number after the 'add' command.\n");
        return;
    }

    index = strtol(tmp, NULL, 10);
    for (nd = node_list_get_head(); nd != NULL; nd = nd->next)
    {
        if (nd->index == index)
        {
            printf("ADD_NODE: Duplicate index. Cannot add.\n");
            return;
        }
    }
    sim_add_node(index);
}

/**************************************************************************
Function Name: kill_node

Description:
NOT WORKING YET.
**************************************************************************/
void kill_node(char *str)
{
    U8 index;
    sim_node_t *nd;
    char *tmp;

    if ((tmp = strtok(str, " ")) == NULL)
    {
        printf("Please add an index number after the 'add' command.\n");
        return;
    }
    index = strtol(tmp, NULL, 10);

    for (nd = node_list_get_head(); nd != NULL; nd = nd->next)
    {
        if (nd->index == index)
        {
            if (pthread_kill(nd->data_out.thread, 0) != 0) perror("pthread_kill");
            if (pthread_kill(nd->cmd_out.thread, 0) != 0) perror("pthread_kill");

            close(nd->data_out.pipe);
            close(nd->data_in.pipe);
            close(nd->cmd_in.pipe);

            unlink(nd->data_in.name);
            unlink(nd->data_out.name);
            unlink(nd->cmd_in.name);

            kill(nd->pid, SIGTERM);
            node_list_remove(nd);
            printf("Node %d was terminated.\n", nd->index);
        }
    }
}

/**************************************************************************
Function Name: list_nodes

Description:

**************************************************************************/
void list_nodes(char *str)
{
    node_list_print();
}

/**************************************************************************
Function Name: process_script

Description:

**************************************************************************/
void process_script(char *str)
{
    char *name, *tmp, *cmdtype, *msg;
    char fcmd[ARGVMAX];
    struct timespec timeout;
    int status;

    name = strtok(str, " ");
    if ((fp = fopen(name, "r")) == NULL)
    {
        printf("PROCESS_SCRIPT: Cannot open file - %s.\n", name);
        return;
    }

    while (fgets(fcmd, ARGVMAX, fp) != NULL)
    {
        // null terminate the string and remove the extra newline
        tmp = strchr(fcmd, '\n');
        *tmp = '\0';

        if (fcmd[0] == '\0')
        {
            continue;
        }

        cmdtype = strtok(fcmd, " ");
        if (strcmp(cmdtype, "send") == 0)
        {
            // if its a command, send it to the parser to get executed
            msg = fcmd + strlen(cmdtype) + 1;
            parse_string(msg);
        }
        else if (strcmp(cmdtype, "wait") == 0)
        {
            //if its a wait instruction, then wait for the string specified in 'msg'
            msg = fcmd + strlen(cmdtype) + 1;

            // set up a timed blocking wait on the condition that data has arrived into the client buffer
            timeout.tv_sec = time(NULL) + 5;
            timeout.tv_nsec = 0;

            // lock the mutex and do a timed conditional wait. if the wait expires, then something went wrong so exit.
            // otherwise stay in the wait loop until we get a matching string.
            if ((status = pthread_mutex_lock(&cli_buf.mutex)) != 0) perror("process script lock mutex");

            while (check_cli_buf(msg))
            {
                if ((status = pthread_cond_timedwait(&cli_buf.cond, &cli_buf.mutex, &timeout)) != 0)
                {
                    perror("cond timed wait");
                }

                if (status == ETIMEDOUT)
                {
                    printf("ERROR - WAIT CONDITION TIMED OUT\n");
                    exit(EXIT_FAILURE);
                }
            }
            if ((status = pthread_mutex_unlock(&cli_buf.mutex)) != 0) perror("process script unlock mutex");
        }
    }

    fclose(fp);
    printf("SUCCESS: Tests passed and script file closed.\n");
}

/**************************************************************************
Function Name: quit_sim

Description:

**************************************************************************/
void quit_sim(char *str)
{
    exit(EXIT_SUCCESS);
}

