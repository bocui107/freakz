/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: process.c,v 1.5 2007/04/04 09:19:18 nifi Exp $
 */

/**
 * \addtogroup process
 * @{
 */

/**
 * \file
 *         Implementation of the Contiki process kernel.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */


// suppress all lint messages for this file
//lint --e{*}

#include <stdio.h>

#include "sys/process.h"
#include "sys/arg.h"

/*
 * Pointer to the currently running process structure.
 */
struct process *process_list = NULL;
struct process *process_current = NULL;

static process_event_t lastevent;

/*
 * Structure used for keeping the queue of active events.
 */
struct event_data {
  process_event_t ev;
  process_data_t data;
  struct process *p;
};

#ifdef PROCESS_CONF_FASTPOLL
#define NPOLLS PROCESS_CONF_FASTPOLL
static volatile unsigned npolls;
static struct process *needpoll[NPOLLS];
#endif
static process_num_events_t nevents, fevent;
static struct event_data events[PROCESS_CONF_NUMEVENTS];

static volatile unsigned char poll_requested;

#define PROCESS_STATE_NONE        0
#define PROCESS_STATE_INIT        1
#define PROCESS_STATE_RUNNING     2
#define PROCESS_STATE_NEEDS_POLL  3

static void call_process(struct process *p, process_event_t ev, process_data_t data);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
process_event_t
process_alloc_event(void)
{
  return lastevent++;
}
/*---------------------------------------------------------------------------*/
void
process_start(struct process *p, char *arg)
{
  struct process *q;

  /* First make sure that we don't try to start a process that is
     already running. */
  for(q = process_list; q != p && q != NULL; q = q->next);

  /* If we found the process on the process list, we bail out. */
  if(q == p) {
    return;
  }
  /* Put on the procs list.*/
  p->next = process_list;
  process_list = p;

  p->state = PROCESS_STATE_INIT;

  PT_INIT(&p->pt);

  /* Post an asynchronous event to the process. */
  process_post(p, PROCESS_EVENT_INIT, (process_data_t)arg);
}
/*---------------------------------------------------------------------------*/
static void
exit_process(struct process *p, struct process *fromprocess)
{
  register struct process *q;
  struct process *old_current = process_current;

  if(p->state != PROCESS_STATE_NONE) {
    /* Process was running */
    p->state = PROCESS_STATE_NONE;

    /*
     * Post a synchronous event to all processes to inform them that
     * this process is about to exit. This will allow services to
     * deallocate state associated with this process.
     */
    for(q = process_list; q != NULL; q = q->next) {
      if(p != q) {
	call_process(q, PROCESS_EVENT_EXITED, (process_data_t)p);
      }
    }

    if(p->thread != NULL && p != fromprocess) {
      /* Post the exit event to the process that is about to exit. */
      process_current = p;
      p->thread(&p->pt, PROCESS_EVENT_EXIT, NULL);
    }
  }

  if(p == process_list) {
    process_list = process_list->next;
  } else {
    for(q = process_list; q != NULL; q = q->next) {
      if(q->next == p) {
	q->next = p->next;
	break;
      }
    }
  }

  {
    int n;
    int i = fevent;
    for(n = nevents; n > 0; n--) {
      if(events[i].p == p) {
	events[i].p = PROCESS_ZOMBIE;
#if 0
	printf("soft panic: exiting process has remaining event 0x%x\n",
	       events[i].ev);
#endif
      }
      i = (i + 1) % PROCESS_CONF_NUMEVENTS;
    }
#ifdef NPOLLS
    for(i = 0; i < NPOLLS && i < npolls; i++) {
      if(needpoll[i] == p) {
	needpoll[i] = PROCESS_ZOMBIE;
      }
    }
#endif
  }
  process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void
call_process(struct process *p, process_event_t ev, process_data_t data)
{
  int ret;

  if((p->state == PROCESS_STATE_RUNNING ||
      p->state == PROCESS_STATE_NEEDS_POLL) &&
     p->thread != NULL) {
    process_current = p;

    ret = p->thread(&p->pt, ev, data);
    if(ret == PT_EXITED ||
       ret == PT_ENDED ||
       ev == PROCESS_EVENT_EXIT) {
      exit_process(p, p);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
process_exit(struct process *p)
{
  exit_process(p, PROCESS_CURRENT());
}
/*---------------------------------------------------------------------------*/
void
process_init(void)
{
  lastevent = PROCESS_EVENT_MAX;

  nevents = fevent = 0;

  process_current = process_list = NULL;
}
/*---------------------------------------------------------------------------*/
/*
 * Call each process' poll handler.
 */
/*---------------------------------------------------------------------------*/
static void
do_poll(void)
{
  struct process *p;

  poll_requested = 0;

#ifdef NPOLLS
  unsigned i;
  int s;
  /* Fastpoll */
  //printf("F %d\n", npolls);
  for(i = 0; i < npolls; i++) {
  do_more:
    if(i == NPOLLS) {
      goto slowpoll;
    }
    if(needpoll[i] != PROCESS_ZOMBIE
       && needpoll[i]->state == PROCESS_STATE_NEEDS_POLL) {
      needpoll[i]->state = PROCESS_STATE_RUNNING;
      call_process(needpoll[i], PROCESS_EVENT_POLL, NULL);
    }
  }
  s = splhigh();
  if(i == npolls) {
    npolls = 0;
    splx(s);
    return;
  }
  splx(s);
  goto do_more;

  /* Call poll handlers. */
 slowpoll:
  //printf("S %d\n", npolls);
  npolls = 0;
#endif
  /* Call poll handlers. */
  for(p = process_list; p != NULL; p = p->next) {

    if(p->state == PROCESS_STATE_NEEDS_POLL) {
      p->state = PROCESS_STATE_RUNNING;
      call_process(p, PROCESS_EVENT_POLL, NULL);
    }

#if 0
    /* If a poll has been requested for one of the processes, we start
       from the beginning again. */
    if(poll_requested) {
      poll_requested = 0;
      p = process_list;
    }
#endif
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Process the next event in the event queue and deliver it to
 * listening processes.
 */
/*---------------------------------------------------------------------------*/
static void
do_event(void)
{
  static process_event_t ev;
  static process_data_t data;
  static struct process *receiver;
  static struct process *p;

  /*
   * If there are any events in the queue, take the first one and walk
   * through the list of processes to see if the event should be
   * delivered to any of them. If so, we call the event handler
   * function for the process. We only process one event at a time and
   * call the poll handlers inbetween.
   */

  if(nevents > 0) {

    /* There are events that we should deliver. */
    ev = events[fevent].ev;

    data = events[fevent].data;
    receiver = events[fevent].p;

    /* Since we have seen the new event, we move pointer upwards
       and decrese the number of events. */
    fevent = (fevent + 1) % PROCESS_CONF_NUMEVENTS;
    --nevents;

    /* If this is a broadcast event, we deliver it to all events, in
       order of their priority. */
    if(receiver == PROCESS_BROADCAST) {
      for(p = process_list; p != NULL; p = p->next) {

	/* If we have been requested to poll a process, we do this in
	   between processing the broadcast event. */
	if(poll_requested) {
	  do_poll();
	}
	call_process(p, ev, data);
      }
    } else if(receiver == PROCESS_ZOMBIE) {
      /* This process has exited. */
    } else {
      /* This is not a broadcast event, so we deliver it to the
	 specified process. */
      /* If the event was an INIT event, we should also update the
	 state of the process. */
      if(ev == PROCESS_EVENT_INIT) {
	receiver->state = PROCESS_STATE_RUNNING;
      }

      /* Make sure that the process actually is running. */
      call_process(receiver, ev, data);
    }
  }
}
/*---------------------------------------------------------------------------*/
int
process_run(void)
{
  /* Process "poll" events. */
  if(poll_requested) {
    do_poll();
  }

  /* Process one event */
  do_event();

  return nevents + poll_requested;
}
/*---------------------------------------------------------------------------*/
int
process_nevents(void)
{
#ifdef NPOLLS
  return nevents + npolls;
#else
  return nevents + poll_requested;
#endif
}
/*---------------------------------------------------------------------------*/
int
process_post(struct process *p, process_event_t ev, process_data_t data)
{
  static unsigned char snum;

  if(nevents == PROCESS_CONF_NUMEVENTS) {
#if DEBUG
    if(p == PROCESS_BROADCAST) {
      printf("soft panic: event queue is full when broadcast event %d was posted from %s\n", ev, process_current->name);
    } else {
      printf("soft panic: event queue is full when event %d was posted to %s frpm %s\n", ev, p->name, process_current->name);
    }
#endif /* DEBUG */
    return PROCESS_ERR_FULL;
  }

  snum = (fevent + nevents) % PROCESS_CONF_NUMEVENTS;
  events[snum].ev = ev;
  events[snum].data = data;
  events[snum].p = p;
  ++nevents;

  return PROCESS_ERR_OK;
}
/*---------------------------------------------------------------------------*/
void
process_post_synch(struct process *p, process_event_t ev, process_data_t data)
{
  struct process *caller = process_current;

  call_process(p, ev, data);
  process_current = caller;
}
/*---------------------------------------------------------------------------*/
void
process_poll(struct process *p)
{
  if(p != NULL) {
    if(p->state == PROCESS_STATE_RUNNING) {
      p->state = PROCESS_STATE_NEEDS_POLL;
      poll_requested = 1;
#ifdef NPOLLS
      int s = splhigh();
      if(npolls < NPOLLS) {
	needpoll[npolls] = p;
      }
      if(npolls != ~0u) npolls++; /* Beware of overflow! */
      splx(s);
#endif
    }
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
