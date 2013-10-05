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
    \file zdo_bind_mgr.c
    \ingroup zdo
    \brief ZDO binding manager

        This file implements the binding manager for the ZDO binding cluster. The
        binding manager is used to bind two devices together via a commissioning device
        or to handle an end device which is a binding operation that is done without
        a commissioning device.
*/
#include "freakz.h"

/* Timeout value for the end device bind process */
#define ED_BIND_TIMEOUT (CLOCK_SECOND * 30)

/* Main process for end device binding */
PROCESS(zdo_ed_bind_process, "End Device Bind Process");

enum
{
	END_DEV_BIND_IDLE = 0,  /* End device bind idle state enum */
	END_DEV_BIND_START,     /* End device bind start state enum */
	END_DEV_BIND_CHECK_DIR, /* End device bind check direction state enum */
	END_DEV_BIND_SENDING    /* End device bind send state enum */
};

static U8 ed_bind_state;           /* End device bind state variable */
static struct ctimer ed_bind_tmr;  /* Callback timer used for the end device bind timeout */
static mem_ptr_t *ed_bind_mem_ptr; /* Memory handle used to keep the end device bind info */

extern process_event_t event_ed_bind_req;   /* End device bind request event */
extern process_event_t event_ed_bind_match; /* End device bind match event */
extern process_event_t event_unbind_resp;   /* End device unbind response event */

static void zdo_bind_ed_timeout(void *ptr);
static U8 zdo_bind_ed_match_clust(U8 num_in_clust,
				  const U8 *in_clust,
				  U8 num_out_clust,
				  const U8 *out_clust,
				  U8 *match_list);
static void zdo_bind_ed_send_resp(U8 status, U16 addr, U8 seq);
static void zdo_bind_ed_find_match();
static void zdo_bind_ed_cleanup();

/*
 * Init the ZDO bind manager. The fsm will be reset and the events will be
 * enumerated and registered. Also, the end device bind process will be started.
 */
void zdo_bind_mgr_init()
{
	ed_bind_state   = 0;
	ed_bind_mem_ptr = NULL;

	/* alloc events for the end dev bind process */
	event_ed_bind_req   = process_alloc_event();
	event_ed_bind_match = process_alloc_event();
	event_unbind_resp   = process_alloc_event();

	/* start the ed bind process */
	process_start(&zdo_ed_bind_process, NULL);
}

/*
 * Timeout function called by the end device bind timeout callback timer
 * in the event that the end device bind request from the second device
 * does not arrive in time.
 */
static void zdo_bind_ed_timeout(void *ptr)
{
	U8 status, seq;
	U16 dest_addr;

	DBG_PRINT("ZDO_BIND: END DEVICE TIMEOUT.\n");

	/* stop the ctimer just in case it's still on */
	ctimer_stop(&ed_bind_tmr);

	if (ed_bind_mem_ptr)
	{
		/* gather the info from the memory pointer */
		dest_addr   = ED_BIND(ed_bind_mem_ptr)->dev0.src_addr;
		seq         = ED_BIND(ed_bind_mem_ptr)->dev0.req.seq;
		status      = AF_TIMEOUT;
		zdo_bind_ed_send_resp(status, dest_addr, seq);
	}

	/* clean up the memory and reset the state machine */
	zdo_bind_ed_cleanup();
}

/*
 * Generate a match list for the clusters. Return the number of matches. All
 * clusters in the match list will be bound together.
 */
static U8 zdo_bind_ed_match_clust(U8 num_in_clust, const U8 *in_clust, U8 num_out_clust, const U8 *out_clust, U8 *match_list)
{
	U8 i, j, matches = 0;
	U16 *in, *out, *match;

	in = (U16 *)in_clust;
	out = (U16 *)out_clust;
	match = (U16 *)match_list;

	for (i=0; i<num_in_clust; i++)
	{
		for (j=0; j<num_out_clust; j++)
		{
			if (in[i] == out[j])
				match[matches++] = in[i];
		}
	}
	return matches;
}

/* Send response to the specified end device */
static void zdo_bind_ed_send_resp(U8 status, U16 addr, U8 seq)
{
	U8 handle, size, *resp, resp_data[2];

	resp = resp_data;
	*resp++ = seq;
	*resp++ = status;
	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, addr, END_DEV_BIND_RESP_CLUST, 0, handle);
}

/*
 * Generate a bind/unbind request for the ed bind request state machine.
 * dev = true if you want to use dev1 or false to use dev0. bind = true if you want
 * to send a bind req or false to send an unbind req.
 */
static void zdo_bind_ed_gen_req(U8 dev)
{
	zdo_req_t req;
	U8 curr_index;
	U16 dest_addr, clust;

	/*
	 * choose the cluster based on whether we are doing
	 * a bind or an unbind
	 */
	clust = (ED_BIND(ed_bind_mem_ptr)->bind) ? BIND_REQ_CLUST : UNBIND_REQ_CLUST;
	curr_index = 2 * ED_BIND(ed_bind_mem_ptr)->curr_index; // each cluster is 2 bytes wide

	req.seq                             = zdo_seq_get();
	req.clust                           = clust;

	if (dev) {
		dest_addr = ED_BIND(ed_bind_mem_ptr)->dev1.src_addr;
		req.type.unbind.src_addr = ED_BIND(ed_bind_mem_ptr)->dev1.src_ext_addr;
		req.type.unbind.src_ep = ED_BIND(ed_bind_mem_ptr)->dev1.src_ep;
		req.type.unbind.clust = ED_BIND(ed_bind_mem_ptr)->dev1.match_list[curr_index];
		req.type.unbind.dest_addr.mode = LONG_ADDR;
		req.type.unbind.dest_addr.long_addr = ED_BIND(ed_bind_mem_ptr)->dev0.src_ext_addr;
		req.type.unbind.dest_ep = ED_BIND(ed_bind_mem_ptr)->dev0.src_ep;
	} else {
		dest_addr = ED_BIND(ed_bind_mem_ptr)->dev0.src_addr;
		req.type.unbind.src_addr = ED_BIND(ed_bind_mem_ptr)->dev0.src_ext_addr;
		req.type.unbind.src_ep = ED_BIND(ed_bind_mem_ptr)->dev0.src_ep;
		req.type.unbind.clust = ED_BIND(ed_bind_mem_ptr)->dev0.match_list[curr_index];
		req.type.unbind.dest_addr.mode = LONG_ADDR;
		req.type.unbind.dest_addr.long_addr  = ED_BIND(ed_bind_mem_ptr)->dev1.src_ext_addr;
		req.type.unbind.dest_ep = ED_BIND(ed_bind_mem_ptr)->dev1.src_ep;
	}

	if (zdo_gen_req(dest_addr, &req) != AF_SUCCESS)
		zdo_bind_ed_cleanup();
}

/*
 * Check the two requests to find matching clusters. Send a response to both
 * devices regarding their end device bind request. If all is okay, then proceed
 * to the next state. Otherwise, terminate the operation and clean up.
 */
static void zdo_bind_ed_find_match()
{
	U8 status, num_matches1, num_matches2;
	U8 match_list1[ZDO_MAX_CLUST], match_list2[ZDO_MAX_CLUST];

	status = AF_SUCCESS;
	if (!ed_bind_mem_ptr) {
		/* something weird happen so just clean up and exit sequence */
		zdo_bind_ed_cleanup();
		return;
	}

	/*
	 * both profile id's must match. if they don't,
	 * then register an error status.
	 */
	if ((ED_BIND(ed_bind_mem_ptr)->dev0.req.type.ed_bind.prof_id !=
	     ED_BIND(ed_bind_mem_ptr)->dev1.req.type.ed_bind.prof_id))
	{
		status = AF_NO_MATCH;
	}

	/*
	 * generate the match lists by pairing up the in clusters
	 * with the out clusters to see which ones are in common
	 *
	 * NOTE: we normally should not send addresses from managed
	 * memory into a function. it's very dangerous because the
	 * address can change if any memory is freed. however as long
	 * as we're careful not to free any memory inside the function,
	 * then it's okay.
	 */
	num_matches1 = zdo_bind_ed_match_clust(ED_BIND(ed_bind_mem_ptr)->dev0.req.type.ed_bind.num_in_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev0.in_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev1.req.type.ed_bind.num_out_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev1.out_clust,
					       match_list1);

	num_matches2 = zdo_bind_ed_match_clust(ED_BIND(ed_bind_mem_ptr)->dev1.req.type.ed_bind.num_in_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev1.in_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev0.req.type.ed_bind.num_out_clust,
					       ED_BIND(ed_bind_mem_ptr)->dev0.out_clust,
					       match_list2);

	/* if no matches are found, then return a status of no match */
	if (!(num_matches1 || num_matches2))
		status = AF_NO_MATCH;

	/* send responses to both devices */
	zdo_bind_ed_send_resp(status,
			      ED_BIND(ed_bind_mem_ptr)->dev0.src_addr,
			      ED_BIND(ed_bind_mem_ptr)->dev0.req.seq);
	zdo_bind_ed_send_resp(status,
			      ED_BIND(ed_bind_mem_ptr)->dev1.src_addr,
			      ED_BIND(ed_bind_mem_ptr)->dev1.req.seq);

	/*
	 * next step depends on status. if there was no match,
	 * then just cleanup and stop the sequence.
	 */
	if (status == AF_NO_MATCH)
	{
		/* free the pointer and clean up */
		zdo_bind_ed_cleanup();
	} else {
		/* save off the match lists */
		memcpy(ED_BIND(ed_bind_mem_ptr)->dev0.match_list, match_list1, num_matches1 * sizeof(U16));
		memcpy(ED_BIND(ed_bind_mem_ptr)->dev1.match_list, match_list2, num_matches2 * sizeof(U16));
		ED_BIND(ed_bind_mem_ptr)->dev0.num_matches = num_matches1;
		ED_BIND(ed_bind_mem_ptr)->dev1.num_matches = num_matches2;

		/* register an event to the process to move to the next step */
		process_post(&zdo_ed_bind_process, event_ed_bind_match, NULL);
	}
}

/*
 * Clean up the end device bind operation. Clean up consists of freeing
 * the alloc'd memory block and setting the state machine to idle.
 */
static void zdo_bind_ed_cleanup()
{
	mem_heap_free(ed_bind_mem_ptr);
	ed_bind_mem_ptr = NULL;
	ed_bind_state = END_DEV_BIND_IDLE;
}

/*
 * Handle an end device bind request. When an end device bind request comes in,
 * we need to do some checks, start off a timer, save the info, and wait for
 * the next end device bind request to come in. ED bind request must come in
 * pairs because two devices will be bound together.
 */
void zdo_bind_ed_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 status, size, *resp, resp_data[2];
	U8 in_clust[ZDO_MAX_CLUST], out_clust[ZDO_MAX_CLUST];
	zdo_req_t req;

	/*
	 * set up the in and out cluster arrays
	 * before we use the req structure
	 */
	req.type.ed_bind.in_clust = in_clust;
	req.type.ed_bind.out_clust = out_clust;

	zdo_parse_req(src_addr, data, clust, &req);

	/*
	 * make sure that the endpoint is within bounds.
	 * if so, then kick off the state machine.
	 */
	if ((req.type.ed_bind.src_ep == 0) ||
	    (req.type.ed_bind.src_ep > AF_EP_RANGE_MAX))
	{
		status = AF_INVALID_EP;
		resp = resp_data;
		*resp++ = req.seq;
		*resp++ = status;

		size = resp - resp_data;
		zdo_tx(resp_data, size, src_addr, END_DEV_BIND_RESP_CLUST, 0, req.seq);
	} else {
		// ep is okay. let's start the state machine. first save off the data.
		if (ed_bind_state == END_DEV_BIND_IDLE)
		{
			/*
			 * start the end device bind sequence.
			 * first alloc the memory
			 */
			if ((ed_bind_mem_ptr = mem_heap_alloc(sizeof(ed_bind_t))) == NULL)
			{
				/* TODO: Add memory exception here */
				zdo_bind_ed_cleanup();
				return;
			}
			memset(ED_BIND(ed_bind_mem_ptr), 0, sizeof(ed_bind_t));

			/* save off all the info for the device */
			memcpy(&ED_BIND(ed_bind_mem_ptr)->dev0.req, &req, sizeof(zdo_req_t));

			/* save off the in and out cluster lists */
			memcpy(ED_BIND(ed_bind_mem_ptr)->dev0.in_clust,
				in_clust,
				req.type.ed_bind.num_in_clust * sizeof(U16));
			memcpy(ED_BIND(ed_bind_mem_ptr)->dev0.out_clust,
				out_clust,
				req.type.ed_bind.num_out_clust * sizeof(U16));

			// save off the rest of the info that we're gonna need
			ED_BIND(ed_bind_mem_ptr)->dev0.src_addr      = src_addr;
			ED_BIND(ed_bind_mem_ptr)->dev0.src_ext_addr  = req.type.ed_bind.src_ext_addr;
			ED_BIND(ed_bind_mem_ptr)->dev0.src_ep        = req.type.ed_bind.src_ep;

			/* register the event to the process thread */
			process_post(&zdo_ed_bind_process, event_ed_bind_req, NULL);
		} else if (ed_bind_state == END_DEV_BIND_START) {
			/* save off the device info */
			memcpy(&ED_BIND(ed_bind_mem_ptr)->dev1.req, &req, sizeof(zdo_req_t));

			/* save off the in and out cluster lists */
			memcpy(ED_BIND(ed_bind_mem_ptr)->dev1.in_clust,
				in_clust,
				req.type.ed_bind.num_in_clust * sizeof(U16));
			memcpy(ED_BIND(ed_bind_mem_ptr)->dev1.out_clust,
				out_clust,
				req.type.ed_bind.num_out_clust * sizeof(U16));

			/* save off the rest of the info that we're gonna need */
			ED_BIND(ed_bind_mem_ptr)->dev1.src_addr      = src_addr;
			ED_BIND(ed_bind_mem_ptr)->dev1.src_ext_addr  = req.type.ed_bind.src_ext_addr;
			ED_BIND(ed_bind_mem_ptr)->dev1.src_ep        = req.type.ed_bind.src_ep;

			process_post(&zdo_ed_bind_process, event_ed_bind_req, NULL);
		}
	}
}

/*  Handle the bind/unbind requests requests coming from the higher layers */
void zdo_bind_unbind_req_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	U8 handle, status, tmp, size, *resp, resp_data[MAX_APS_PAYLOAD];
	zdo_req_t req;
	mac_pib_t *pib = mac_pib_get();
	U16 clust_id;

	zdo_parse_req(src_addr, data, clust, &req);

	/* check to see if its a bind or unbind request */
	clust_id = (clust == BIND_REQ_CLUST) ? BIND_RESP_CLUST : UNBIND_RESP_CLUST;

	status = AF_SUCCESS;
	if (req.type.bind.src_addr == pib->ext_addr)
	{
		if (clust == BIND_REQ_CLUST)
		{
			tmp = aps_bind_req(req.type.bind.src_ep,
					   &req.type.bind.dest_addr,
					   req.type.bind.dest_ep,
					   req.type.bind.clust);
			status = (tmp == APS_SUCCESS) ? AF_SUCCESS : AF_TABLE_FULL;
		} else {
			tmp = aps_unbind_req(req.type.bind.src_ep,
					     &req.type.bind.dest_addr,
					     req.type.bind.dest_ep,
					     req.type.bind.clust);
			status = (tmp == APS_SUCCESS) ? AF_SUCCESS : AF_NO_ENTRY;
		}
	}
	resp = resp_data;
	*resp++ = req.seq;
	*resp++ = status;

	size = resp - resp_data;
	handle = af_handle_get();
	zdo_tx(resp_data, size, src_addr, clust_id, 0, handle);
}

/*
 * Handle the end device bind response. This just parses the end device
 * bind response to see what the status is.
 */
void zdo_bind_ed_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	zdo_resp_t resp;

	zdo_parse_resp(data, clust, &resp);
}

/*
 * Handle the unbind response. If an end device bind operation is going on, then
 * it will trigger state machine transitions. Otherwise, just parse the response
 * and do nothing for now.
 */
void zdo_bind_unbind_resp_handler(U8 *data, U8 len, U16 src_addr, U8 src_ep, U16 clust)
{
	zdo_resp_t resp;
	bool bind;

	zdo_parse_resp(data, clust, &resp);

	switch (ed_bind_state)
	{
	case END_DEV_BIND_CHECK_DIR:
		if (clust == UNBIND_RESP_CLUST)
		{
			/*
			 * whether the operation will be a bind or unbind
			 * depends on the test unbind request that we sent.
			 * if the binding doesn't exist, then we will do a
			 * bind. if it does exist, then we just unbound
			 * that cluster. increment the index and proceed
			 * with the rest of the unbind.
			 */
			bind = resp.type.unbind.status == AF_NO_ENTRY;
			ED_BIND(ed_bind_mem_ptr)->bind = bind;

			/*
			 * we need to increment the curr_index because we
			 * just successfully unbound the cluster. if we don't
			 * increment, then we'll unbind it again later when we
			 * start the actual unbind procedure.
			 */
			if (!bind) {
				ED_BIND(ed_bind_mem_ptr)->curr_index++;
			}

			process_post(&zdo_ed_bind_process, event_unbind_resp, NULL);
		}
		break;
	case END_DEV_BIND_SENDING:
		if (resp.type.unbind.status == AF_SUCCESS)
		{
			/*
			 * process next matched clust check to see if we're
			 * done with the match list for this device. if so,
			 * then switch over to the next one.
			 */
			ED_BIND(ed_bind_mem_ptr)->curr_index++;
			if (ED_BIND(ed_bind_mem_ptr)->curr_dev == 0)
			{
				if (ED_BIND(ed_bind_mem_ptr)->curr_index ==
				    ED_BIND(ed_bind_mem_ptr)->dev0.num_matches)
				{
					ED_BIND(ed_bind_mem_ptr)->curr_dev   = 1;
					ED_BIND(ed_bind_mem_ptr)->curr_index = 0;
				}
			} else {
				if (ED_BIND(ed_bind_mem_ptr)->curr_index ==
				    ED_BIND(ed_bind_mem_ptr)->dev1.num_matches)
				{
					/* we're finished. */
					zdo_bind_ed_cleanup();
					return;
				}
			}
			zdo_bind_ed_gen_req(ED_BIND(ed_bind_mem_ptr)->curr_dev);
		} else {
			/*
			 * weird status...need to figure out how to handle this.
			 * for now, just stop the operation
			 */
			zdo_bind_ed_cleanup();
		}
		break;
	}
}

/*
 * This is the end device bind process. It is used to process events as they
 * occur for the end device bind. The whole end device bind dance is a bit complicated
 * which is why I thought using a process might make things easier. As the usage
 * becomes more clear, I'm going to try and simplify this function. Otherwise,
 * just take it out since I suspect that most people will eventually commission
 * their networks with commissioning devices.
 */
PROCESS_THREAD(zdo_ed_bind_process, ev, data)
{
	PROCESS_BEGIN();

	while (1)
	{
		PROCESS_WAIT_EVENT();
		if (ev == event_ed_bind_req)
		{
			if (ed_bind_state == END_DEV_BIND_IDLE)
			{
				/*
				 * start the binding timer and wait for
				 * the next bind req
				 */
				ctimer_set(&ed_bind_tmr, ED_BIND_TIMEOUT, zdo_bind_ed_timeout, NULL);
				ed_bind_state = END_DEV_BIND_START;
			} else if (ed_bind_state == END_DEV_BIND_START) {
				/* stop the ctimer */
				ctimer_stop(&ed_bind_tmr);
				zdo_bind_ed_find_match();
			}
		} else if (ev == event_ed_bind_match) {
			/*
			 * match has been found. next, we need to check whether
			 * we do a bind or unbind. this will depend on the
			 * response we get when we send an unbind request to
			 * one of the matched clusters.
			 */
			ed_bind_state = END_DEV_BIND_CHECK_DIR;
			zdo_bind_ed_gen_req(0);
		} else if (ev == event_unbind_resp) {
			/*
			 * the test unbind req received a response and we now
			 * know if we need to do a bind or unbind operation.
			 * so now, let's get to the sending.
			 */
			ed_bind_state = END_DEV_BIND_SENDING;

			/*
			 * start off the binding process by sending a
			 * bind/unbind request to the first device
			 */
			zdo_bind_ed_gen_req(ED_BIND(ed_bind_mem_ptr)->curr_dev);
		}
	}
	PROCESS_END();
}
