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
/*!
    \file ctrl.c
    \ingroup usb

    This file handles all of the control requests issued by the host.
    It's where most of the USB protocol layer functionality is implemented.
*/
#include "freakusb.h"

/*
 * Handle the descriptor requests coming in from the host. This function will
 * respond to device, configuration, and string descriptor requests by
 * returning the relevant descriptor stored in flash.
 */
void ctrl_get_desc(req_t *req)
{
	U8 i = 0, desc_len = 0, desc_type, desc_idx;
	U8 *desc = NULL;

	desc_type = (req->val >> 8);
	desc_idx = req->val & 0x00ff;

	switch (desc_type) {
	case DEV_DESCR:
		desc        = desc_dev_get();
		desc_len    = desc_dev_get_len(); /* size is first byte of dev desc */
		break;
	case CFG_DESCR:
		desc        = desc_cfg_get();
		desc_len    = desc_cfg_get_len(); /* total len is third byte of cfg desc */
		break;
	case DEV_QUAL_DESCR:
		desc        = desc_dev_qual_get();
		desc_len    = desc_dev_qual_get_len();
		break;
	case STR_DESCR:
		desc        = desc_str_get(desc_idx);
		desc_len    = desc_str_get_len(desc_idx);
		break;
	}

	/*
	 * check to see if the get desc specified length is greater than the
	 * desc len. if it is, then send the whole descriptor plus a possible
	 * zero packet. otherwise, just send the amount of data that's being
	 * requested with no zero packet.
	 */
	if (req->len < desc_len)
		desc_len = req->len;

	/*
	 * since ctrl endpoints are the only endpoints that transfer data
	 * in both directions, we need to have special handling of the
	 * buffers to accomodate this. now that we've decoded the request,
	 * discard the request data by clearing the fifo. then we will
	 * reuse the buffer to transmit the descriptor info.
	 */
	usb_buf_clear_fifo(EP_CTRL);

	/*
	 * this is a bit inefficient since we're copying the data out of
	 * flash and into a buffer before we transfer it. however this
	 * way, we can re-use the ep_write function since it requires
	 * data to be in the buffer. otherwise, we need a separate
	 * function that can write to the ep from flash memory.
	 */
	do {
		usb_buf_write(EP_CTRL, hw_flash_get_byte(desc++));
		i++;

		/*
		 * if we hit a max buf size, then send out the data
		 * before we continue. we need to empty out the buffer
		 * before we can add more data into it.
		 */
		if ((i % MAX_BUF_SZ) == 0)
			ep_write(EP_CTRL);
	} while (i < desc_len);

	/*
	 * now write the data from the buffer to the endpoint
	 * tx fifo. yes...a double copy.
	 */
	ep_write(EP_CTRL);
}

/* Return the device configuration number to the host. */
void ctrl_get_config()
{
	usb_pcb_t *pcb = usb_pcb_get();

	usb_buf_write(EP_CTRL, pcb->cfg_num);
	ep_write(EP_CTRL);
}

/* Return the status of the device or endpoint to the host. */
void ctrl_get_status(req_t *req)
{
	U8 i, rem_wake_enb = 0;
	U8 status[2];
	usb_pcb_t *pcb = usb_pcb_get();

	memset(status, 0, 2 * sizeof(status));
	switch (req->type) {
	case GET_DEVICE_STATUS:
		rem_wake_enb = (pcb->flags & (1<<REMOTE_WAKEUP_ENB)) >> REMOTE_WAKEUP_ENB;
		status[0] = POWER_SRC | (rem_wake_enb << 1);
		status[1] = 0;

		for (i = 0; i < 2; i++)
			usb_buf_write(EP_CTRL, status[i]);

		ep_write(EP_CTRL);
		break;
	case GET_EP_STATUS:
		status[0] = (pcb->ep_stall & (1 << req->idx)) >> req->idx;
		status[1] = 0;

		for (i = 0; i < 2; i++)
			usb_buf_write(EP_CTRL, status[i]);

		ep_write(EP_CTRL);
		break;
	default:
		ep_set_stall(EP_CTRL);
		break;
	}
}

/*
 * Set the configuration. This function will call the class init callback,
 * which will setup the endpoints according to the device class.
 */
void ctrl_set_config(req_t *req)
{
	usb_pcb_t *pcb;

	ep_send_zlp(EP_CTRL);

	pcb = usb_pcb_get();
	pcb->cfg_num = req->val;

	/* we only have one config for now */
	pcb->class_init();

	/* signal that the device is enumerated */
	pcb->flags |= (1<<ENUMERATED);
}

/*
 * Set the specified feature. Currently there are only three features defined
 * in the USB spec: endpoint halt, remote wakeup, and test mode. We currently
 * don't support test mode. Also, remote wakeup is not supported. We only
 * set a flag in the protocol control block for now, but don't really do
 * anything with it. This may be implemented in the future. Endpoint halt will
 * stall the specified endpoint.
 */
void ctrl_set_feat(req_t *req)
{
	usb_pcb_t *pcb = usb_pcb_get();

	switch (req->val) {
	case ENDPOINT_HALT:
		ep_set_stall(req->idx);
		break;
	case REMOTE_WAKEUP:
		pcb->flags |= (1<<REMOTE_WAKEUP_ENB);
		break;
	default:
		ep_set_stall(EP_CTRL);
		break;
	}
}

/*
 * Only three features can be cleared by this request. Since we don't support
 * test mode, then we only handle the endpoint halt and the remote wakeup clear
 * requests. Clearing the endpoint halt just means that we clear the stall condition.
 * Clearing the remote wakeup enable will just clear the flag. We currently
 * don't do anything with the flag.
 */
void ctrl_clear_feat(req_t *req)
{
	usb_pcb_t *pcb = usb_pcb_get();

	switch (req->val) {
	case ENDPOINT_HALT:
		ep_clear_stall(req->idx);

		/* when you clear the halt feature, you also reset the data toggle */
		ep_reset_toggle(req->idx);
		break;
	case REMOTE_WAKEUP:
		pcb->flags &= ~(1<<REMOTE_WAKEUP_ENB);
		break;
	default:
		ep_set_stall(EP_CTRL);
		break;
	}
}

/*
 * Handle the control requests from the host. This is the meat of the USB stack
 * where the requests are divided into standard requests (handled by the USB layer)
 * or class specific (handled by the class driver). If its an unsupported request,
 * then we'll stall the endpoint.
 */
void ctrl_handler()
{
	usb_pcb_t *pcb = usb_pcb_get();
	U8 i, req[CTRL_IN_REQ_SZ];
	req_t *reqp;

	/* read out the request from the buffers */
	for (i=0; i<CTRL_IN_REQ_SZ; i++)
		req[i] = usb_buf_read(EP_CTRL);

	/* do a pointer overlay on the requst */
	reqp = (req_t *)req;

	/* decode the standard request */
	if (!(reqp->type & TYPE_CLASS)) {
		/* this is a standard request */
		switch (reqp->req) {
		case GET_DESCRIPTOR:
			if (reqp->type & DEVICE_TO_HOST)
				ctrl_get_desc(reqp);
			break;
		case GET_CONFIGURATION:
			if (reqp->type & DEVICE_TO_HOST)
				ctrl_get_config();
			break;
		case GET_STATUS:
			if (reqp->type & DEVICE_TO_HOST)
				ctrl_get_status(reqp);
			break;
		case SET_ADDRESS:
			ep_set_addr(reqp->val);
			break;
		case SET_CONFIGURATION:
			ctrl_set_config(reqp);
			break;
		case SET_FEATURE:
			ctrl_set_feat(reqp);
			break;
		case CLEAR_FEATURE:
			ctrl_clear_feat(reqp);
			break;
		default:
			ep_set_stall(EP_CTRL);
			break;
		}
	} else {
	/*
	 * if the class req handler has been registered,
	 * then send the packet to the req handler
	 */
	if (pcb->class_req_handler)
		pcb->class_req_handler(reqp);
	else
		ep_set_stall(EP_CTRL);
	}
}
