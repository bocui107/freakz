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

    Title: test_check_frm.c

    Description:
    This files performs an auto-check between two frames.
*******************************************************************/
#include "zigbee.h"

static aps_hdr_t    aps_hdr_exp;
static nwk_hdr_t    nwk_hdr_exp;
static mac_hdr_t    mac_hdr_exp;
static nwk_cmd_t    nwk_cmd_exp;
static mac_cmd_t    mac_cmd_exp;
static pan_descr_t  pan_desc_exp;
static U8 payload_exp[aMaxPHYPacketSize];

/**************************************************************************
Function Name: test_check_load_aps_hdr

Description:

**************************************************************************/
void test_check_load_aps_hdr(aps_hdr_t *hdr)
{
    memcpy(&aps_hdr_exp, hdr, sizeof(aps_hdr_t));
}

/**************************************************************************
Function Name: test_check_load_nwk_hdr

Description:

**************************************************************************/
void test_check_load_nwk_hdr(nwk_hdr_t *hdr)
{
    memcpy(&nwk_hdr_exp, hdr, sizeof(nwk_hdr_t));
}

/**************************************************************************
Function Name: test_check_load_mac_hdr

Description:

**************************************************************************/
void test_check_load_mac_hdr(mac_hdr_t *hdr)
{
    memcpy(&mac_hdr_exp, hdr, sizeof(mac_hdr_t));
}

/**************************************************************************
Function Name: test_check_load_nwk_cmd

Description:

**************************************************************************/
void test_check_load_nwk_cmd(nwk_cmd_t *cmd)
{
    memcpy(&nwk_cmd_exp, cmd, sizeof(nwk_cmd_t));
}

/**************************************************************************
Function Name: test_check_load_mac_cmd

Description:

**************************************************************************/
void test_check_load_mac_cmd(mac_cmd_t *cmd)
{
    memcpy(&mac_cmd_exp, cmd, sizeof(mac_cmd_t));
}

/**************************************************************************
Function Name: test_check_load_pan_descr

Description:
Load the pan descriptor into the expected pan descriptor structure.
**************************************************************************/
void test_check_load_pan_descr(pan_descr_t *desc)
{
    memcpy(&pan_desc_exp, desc, sizeof(pan_descr_t));
}

/**************************************************************************
Function Name: test_check_load_payload

Description:

**************************************************************************/
void test_check_load_payload(U8 *data, U8 size)
{
    memcpy(payload_exp, data, size);
}

/**************************************************************************
Function Name: test_check_aps

Description:
Check that the aps headers match.
**************************************************************************/
void test_check_aps(aps_hdr_t *hdr_in)
{
    bool    clust_id_flag   = false,
            prof_id_flag    = false,
            group_addr_flag = false,
            dest_ep_flag    = false,
            src_ep_flag     = false;

    // set up the check fields since they are dependent on the frame control fields
    if ((hdr_in->aps_frm_ctrl.frm_type == APS_DATA_FRM) ||
        (hdr_in->aps_frm_ctrl.frm_type == APS_ACK_FRM))
    {
        clust_id_flag = true;
        prof_id_flag = true;
    }

    switch (hdr_in->aps_frm_ctrl.delivery_mode)
    {
    case APS_UNICAST:
        // fall through
    case APS_BROADCAST:
        dest_ep_flag = true;
        src_ep_flag = true;
        break;
    case APS_INDIRECT:
        if (hdr_in->aps_frm_ctrl.ack_format)
        {
            src_ep_flag = true;
        }
        else
        {
            dest_ep_flag = true;
        }
        break;
    case APS_GROUP:
        group_addr_flag = true;
        src_ep_flag = true;
        break;
    }

    assert(hdr_in->aps_fcf == aps_hdr_exp.aps_fcf);
    if (dest_ep_flag)       assert(hdr_in->dest_ep  == aps_hdr_exp.dest_ep);
    if (group_addr_flag)    assert(hdr_in->grp_addr == aps_hdr_exp.grp_addr);
    if (clust_id_flag)      assert(hdr_in->clust_id == aps_hdr_exp.clust_id);
    if (prof_id_flag)       assert(hdr_in->prof_id  == aps_hdr_exp.prof_id);
    if (src_ep_flag)        assert(hdr_in->src_ep   == aps_hdr_exp.src_ep);
}

/**************************************************************************
Function Name: test_check_nwk

Description:
Check that the NWK headers match.
**************************************************************************/
void test_check_nwk(nwk_hdr_t *hdr_in)
{
    assert(hdr_in->nwk_fcf      == nwk_hdr_exp.nwk_fcf);
    assert(hdr_in->dest_addr    == nwk_hdr_exp.dest_addr);
    assert(hdr_in->src_addr     == nwk_hdr_exp.src_addr);
    assert(hdr_in->radius       == nwk_hdr_exp.radius);

    if (hdr_in->nwk_frm_ctrl.dest_ieee_addr_flag)   assert(hdr_in->dest_ieee_addr == nwk_hdr_exp.dest_ieee_addr);
    if (hdr_in->nwk_frm_ctrl.src_ieee_addr_flag)    assert(hdr_in->src_ieee_addr == nwk_hdr_exp.src_ieee_addr);
    if (hdr_in->nwk_frm_ctrl.mcast_flag)            assert(hdr_in->mcast_ctrl == nwk_hdr_exp.mcast_ctrl);
    if (hdr_in->nwk_frm_ctrl.src_rte)
    {
       // no source route checking yet.
    }
}

/**************************************************************************
Function Name: test_check_mac

Description:
Check that the MAC headers match.
**************************************************************************/
void test_check_mac(mac_hdr_t *hdr_in)
{
    assert(hdr_in->mac_fcf == hdr_in->mac_fcf);
    assert(hdr_in->dest_pan_id == mac_hdr_exp.dest_pan_id);
    switch (hdr_in->dest_addr.mode)
    {
    case SHORT_ADDR:        assert(hdr_in->dest_addr.short_addr == mac_hdr_exp.dest_addr.short_addr); break;
    case LONG_ADDR:         assert(hdr_in->dest_addr.long_addr == mac_hdr_exp.dest_addr.long_addr); break;
    }

    if (!hdr_in->mac_frm_ctrl.pan_id_compr) assert(hdr_in->src_pan_id == mac_hdr_exp.src_pan_id);

    switch (hdr_in->src_addr.mode)
    {
    case SHORT_ADDR:    assert(hdr_in->src_addr.short_addr == mac_hdr_exp.src_addr.short_addr); break;
    case LONG_ADDR:     assert(hdr_in->src_addr.long_addr == mac_hdr_exp.src_addr.long_addr); break;
    }
}

/**************************************************************************
Function Name: test_check_nwk_cmd

Description:

**************************************************************************/
void test_check_nwk_cmd( nwk_cmd_t *cmd_in)
{
    switch (cmd_in->cmd_frm_id)
    {
    case NWK_CMD_RTE_REQ:
        assert(cmd_in->rreq.cmd_opts    == nwk_cmd_exp.rreq.cmd_opts);
        assert(cmd_in->rreq.dest_addr   == nwk_cmd_exp.rreq.dest_addr);
        assert(cmd_in->rreq.path_cost   == nwk_cmd_exp.rreq.path_cost);
        break;
    case NWK_CMD_RTE_REP:
        assert(cmd_in->rrep.cmd_opts    == nwk_cmd_exp.rrep.cmd_opts);
        assert(cmd_in->rrep.originator  == nwk_cmd_exp.rrep.originator);
        assert(cmd_in->rrep.responder   == nwk_cmd_exp.rrep.responder);
        assert(cmd_in->rrep.path_cost   == nwk_cmd_exp.rrep.path_cost);
        break;
    }
}

/**************************************************************************
Function Name: test_check_mac_cmd

Description:

**************************************************************************/
void test_check_mac_cmd(mac_cmd_t *cmd_in)
{
    assert(cmd_in->cmd_id == mac_cmd_exp.cmd_id);
    switch (cmd_in->cmd_id)
    {
    case MAC_ASSOC_REQ:
        assert(cmd_in->cmd_params.assoc_req.cap_info == mac_cmd_exp.cmd_params.assoc_req.cap_info);
        break;
    case MAC_ASSOC_RESP:
        assert(cmd_in->cmd_params.assoc_resp.assoc_status == mac_cmd_exp.cmd_params.assoc_resp.assoc_status);
        assert(cmd_in->cmd_params.assoc_resp.short_addr == mac_cmd_exp.cmd_params.assoc_resp.short_addr);
        break;
    default:
        break;
    }
}

/**************************************************************************
Function Name: test_check_pan_descr

Description:

**************************************************************************/
void test_check_pan_descr(pan_descr_t *desc_in)
{
    assert(desc_in->coord_pan_id            == pan_desc_exp.coord_pan_id);
    assert(desc_in->channel                 == pan_desc_exp.channel);
    assert(desc_in->superfrm_spec           == pan_desc_exp.superfrm_spec);
    assert(desc_in->prot_id                 == pan_desc_exp.prot_id);
    assert(desc_in->stack_profile           == pan_desc_exp.stack_profile);
    assert(desc_in->rtr_cap                 == pan_desc_exp.rtr_cap);
    assert(desc_in->end_dev_cap             == pan_desc_exp.end_dev_cap);
    assert(desc_in->ext_pan_id              == pan_desc_exp.ext_pan_id);
    assert(desc_in->coord_addr.mode         == pan_desc_exp.coord_addr.mode);
    //assert(desc_in->link_quality            == pan_desc_exp.link_quality);
    //assert(desc_in->pot_parent              == pan_desc_exp.pot_parent);

    if (desc_in->coord_addr.mode == SHORT_ADDR)
    {
        assert(desc_in->coord_addr.short_addr   == pan_desc_exp.coord_addr.short_addr);
    }
    else
    {
        assert(desc_in->coord_addr.long_addr == pan_desc_exp.coord_addr.long_addr);
    }
}

/**************************************************************************
Function Name: test_check_payload

Description:
Check that the payloads match.
**************************************************************************/
static void test_check_payload(U8 *payload, U8 size)
{
    assert( memcmp(payload, payload_exp, size) == 0);
}

/**************************************************************************
Function Name: test_check_load_frm

Description:
Load the expected frame headers and payload that will be used to check against
the actual frame.
**************************************************************************/
void test_check_load_frm(aps_hdr_t *aps_hdr, nwk_hdr_t *nwk_hdr, mac_hdr_t *mac_hdr, U8 *payload, U8 payload_size)
{
    test_check_load_aps_hdr(aps_hdr);
    test_check_load_nwk_hdr(nwk_hdr);
    test_check_load_mac_hdr(mac_hdr);
    test_check_load_payload(payload, payload_size);
}

/**************************************************************************
Function Name: test_check_frm

Description:
Check that the frm info matches.
**************************************************************************/
void test_check_frm(buffer_t *buf)
{
    mac_hdr_t mac_hdr_in;
    nwk_hdr_t nwk_hdr_in;
    aps_hdr_t aps_hdr_in;
    U8 payload_in[aMaxPHYPacketSize];
    U8 size;

    mac_parse_hdr(buf, &mac_hdr_in);
    nwk_parse_hdr(buf, &nwk_hdr_in);
    aps_parse_hdr(buf, &aps_hdr_in);

    size = (buf->buf + aMaxPHYPacketSize) - buf->dptr;
    memcpy(payload_in, buf->dptr, size);

    test_check_mac(&mac_hdr_in);
    test_check_nwk(&nwk_hdr_in);
    test_check_aps(&aps_hdr_in);
    test_check_payload(payload_in, size);
}

/**************************************************************************
Function Name: void_test_check_frm_hdrs

Description:
Check that the frame headers match.
**************************************************************************/
void test_check_frm_hdrs(aps_hdr_t *aps_hdr_in, nwk_hdr_t *nwk_hdr_in, mac_hdr_t *mac_hdr_in)
{
    test_check_mac(mac_hdr_in);
    test_check_nwk(nwk_hdr_in);
    test_check_aps(aps_hdr_in);
}


