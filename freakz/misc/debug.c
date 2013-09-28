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
    \file debug.c
    \ingroup misc
    \brief Functions for debugging

    Decode and print various frame formats for debugging purposes.
    Bascially, a set of various debug utilities that I cobbled together.
*/
/**************************************************************************/
#include "freakz.h"
#include "zcl_on_off.h"
#include "zcl_id.h"
#include "zcl_level.h"

// suppress all lint messages for this file
//lint --e{*}

/**************************************************************************/
/*!
    Print out the contents of a frame buffer.
*/
/**************************************************************************/
void debug_dump_buf(const U8 *buf, U8 len)
{
#if (DEBUG_BUF == 1)
    U8 i;

    DBG_PRINT_RAW("\n");
    DBG_PRINT("DBG_DUMP_BUF: ");
    for (i=0; i<len; i++)
    {
        DBG_PRINT_RAW("%02x ", buf[i]);
    }
    DBG_PRINT_RAW("\n");
#endif // DEBUG_BUF
}

/**************************************************************************/
/*!
    Decode the MAC header and print out the fields.
*/
/**************************************************************************/
void debug_dump_mac_hdr(const mac_hdr_t *hdr)
{
#if (DEBUG_MAC == 1)
    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_MAC_HDR: FRAME TYPE     = %s.\n",   (hdr->mac_frm_ctrl.frame_type == MAC_BEACON)    ?  "BEACON"  :
                                                        (hdr->mac_frm_ctrl.frame_type == MAC_DATA)      ?  "DATA"    :
                                                        (hdr->mac_frm_ctrl.frame_type == MAC_ACK)       ?  "ACK"     :
                                                                                                           "COMMAND");
    DBG_PRINT("DUMP_MAC_HDR: PENDING_FRAME  = %s.\n",   (hdr->mac_frm_ctrl.frame_pending)        ?  "TRUE"    : "FALSE");
    DBG_PRINT("DUMP_MAC_HDR: DSN            = %02X.\n", hdr->dsn);

    if (hdr->mac_frm_ctrl.frame_type != MAC_ACK)
    {
        DBG_PRINT("DUMP_MAC_HDR: ACK REQ        = %s.\n",   (hdr->mac_frm_ctrl.ack_req)              ?  "TRUE"    : "FALSE");
        DBG_PRINT("DUMP_MAC_HDR: PAN ID COMPR   = %s.\n",   (hdr->mac_frm_ctrl.pan_id_compr)         ?  "TRUE"    : "FALSE");
        DBG_PRINT("DUMP_MAC_HDR: DEST ADDR MODE = %s.\n",   (hdr->dest_addr.mode == 0)               ?  "NONE"    :
                                                            (hdr->dest_addr.mode == 2)               ?  "SHORT"   :
                                                            (hdr->dest_addr.mode == 3)               ?  "LONG"    :
                                                                                                        "UNDEF"            );
        DBG_PRINT("DUMP_MAC_HDR: SRC ADDR MODE  = %s.\n",   (hdr->src_addr.mode == 0)                ?  "NONE"    :
                                                            (hdr->src_addr.mode == 2)                ?  "SHORT"   :
                                                            (hdr->src_addr.mode == 3)                ?  "LONG"    :
                                                                                                        "UNDEF"            );

        switch (hdr->dest_addr.mode)
        {
        case NO_PAN_ID_ADDR:
            DBG_PRINT("DUMP_MAC_HDR: DEST PAN ID    = %s.\n", "NONE");
            DBG_PRINT("DUMP_MAC_HDR: DEST ADDR      = %s.\n", "NONE");
            break;
        case SHORT_ADDR:
            DBG_PRINT("DUMP_MAC_HDR: DEST PAN ID    = %04X.\n", hdr->dest_pan_id);
            DBG_PRINT("DUMP_MAC_HDR: DEST ADDR      = %04X.\n", hdr->dest_addr.short_addr);
            break;
        case LONG_ADDR:
            DBG_PRINT("DUMP_MAC_HDR: DEST PAN ID    = %04X.\n", hdr->dest_pan_id);
            DBG_PRINT("DUMP_MAC_HDR: DEST ADDR      = %016llX.\n",  hdr->dest_addr.long_addr);
            break;
        default:
            break;
        }

        if (!hdr->mac_frm_ctrl.pan_id_compr)
        {
            DBG_PRINT("DUMP_MAC_HDR: SRC PAN ID     = %04X.\n", hdr->src_pan_id);
        }
        else
        {
            DBG_PRINT("DUMP_MAC_HDR: SRC PAN ID     = %s.\n", "NONE");
        }

        switch (hdr->src_addr.mode)
        {
        case NO_PAN_ID_ADDR:    DBG_PRINT("DUMP_MAC_HDR: SRC ADDR       = %s.\n", "NONE"); break;
        case SHORT_ADDR:        DBG_PRINT("DUMP_MAC_HDR: SRC ADDR       = %04X.\n", hdr->src_addr.short_addr); break;
        case LONG_ADDR:         DBG_PRINT("DUMP_MAC_HDR: SRC ADDR       = %016llX.\n",  hdr->src_addr.long_addr); break;
        default: break;
        }
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Decode the NWK header and print out the fields.
*/
/**************************************************************************/
void debug_dump_nwk_hdr(const nwk_hdr_t *hdr)
{
#if (DEBUG_NWK == 1)
    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_NWK_HDR: FRAME TYPE     = %s.\n",   (hdr->nwk_frm_ctrl.frame_type)          ?   "CMD" : "DATA");
    DBG_PRINT("DUMP_NWK_HDR: PROT VER       = %X.\n",   (hdr->nwk_frm_ctrl.protocol_ver));
    DBG_PRINT("DUMP_NWK_HDR: DISCOVER ROUTE = %s.\n",   (hdr->nwk_frm_ctrl.disc_route == 0)     ?   "NO"    :
                                                        (hdr->nwk_frm_ctrl.disc_route == 1)     ?   "YES"   :
                                                        (hdr->nwk_frm_ctrl.disc_route == 2)     ?   "FORCE" :
                                                                                                    "RESERVED");
    DBG_PRINT("DUMP_NWK_HDR: MULTICAST      = %s.\n",   (hdr->nwk_frm_ctrl.mcast_flag)           ?  "PRESENT" : "NONE");
    DBG_PRINT("DUMP_NWK_HDR: SECURITY       = %s.\n",   (hdr->nwk_frm_ctrl.security)             ?  "PRESENT" : "NONE");
    DBG_PRINT("DUMP_NWK_HDR: SRC RTE        = %s.\n",   (hdr->nwk_frm_ctrl.src_rte)              ?  "PRESENT" : "NONE");
    DBG_PRINT("DUMP_NWK_HDR: DEST IEEE ADDR = %s.\n",   (hdr->nwk_frm_ctrl.dest_ieee_addr_flag)  ?  "PRESENT" : "NONE");
    DBG_PRINT("DUMP_NWK_HDR: SRC IEEE ADDR  = %s.\n",   (hdr->nwk_frm_ctrl.src_ieee_addr_flag)   ?  "PRESENT" : "NONE");

    DBG_PRINT("DUMP_NWK_HDR: DEST NWK ADDR  = %04X.\n", hdr->dest_addr);
    DBG_PRINT("DUMP_NWK_HDR: SRC NWK ADDR   = %04X.\n", hdr->src_addr);
    DBG_PRINT("DUMP_NWK_HDR: RADIUS         = %02X.\n", hdr->radius);
    DBG_PRINT("DUMP_NWK_HDR: SEQ NUM        = %02X.\n", hdr->seq_num);
    DBG_PRINT("DUMP_NWK_HDR: HANDLE         = %02X.\n", hdr->handle);

    if (hdr->nwk_frm_ctrl.dest_ieee_addr_flag)
    {
        DBG_PRINT("DUMP_NWK_HDR: DEST IEEE ADDR = %016llX.\n", hdr->dest_ieee_addr);
    }

    if (hdr->nwk_frm_ctrl.src_ieee_addr_flag)
    {
        DBG_PRINT("DUMP_NWK_HDR: SRC IEEE ADDR  = %016llX.\n", hdr->src_ieee_addr);
    }

    if (hdr->nwk_frm_ctrl.mcast_flag)
    {
        DBG_PRINT("DUMP_NWK_HDR: MCAST CTRL     = %02X.\n", hdr->mcast_ctrl);
    }

    if (hdr->nwk_frm_ctrl.src_rte)
    {
        U8 i, relay_cnt, *tmp;

        tmp = hdr->src_rte_subfrm;
        relay_cnt = *tmp;
        DBG_PRINT("DUMP_NWK_HDR: SRC RTE CNT   = %02X.\n", *tmp++);
        DBG_PRINT("DUMP_NWK_HDR: SRC RTE INDX  = %02X.\n", *tmp++);

        relay_cnt = *hdr->src_rte_subfrm;
        for (i=0; i<relay_cnt; i++)
        {
            DBG_PRINT("DUMP_NWK_HDR: SRC RTE LIST %d = %04X.\n", i, *(U16 *)(tmp + (i<<1)));
        }
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Decode the APS header and print out the fields.
*/
/**************************************************************************/
void debug_dump_aps_hdr(const aps_hdr_t *hdr)
{
#if (DEBUG_APS == 1)
    bool    dest_ep_flag    = false,
            clust_id_flag   = false,
            src_ep_flag     = false,
            group_addr_flag = false,
            prof_id_flag    = false;

    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_APS_HDR: FRAME TYPE     = %s.\n",   (hdr->aps_frm_ctrl.frm_type == 0x0)      ?  "DATA"       :
                                                        (hdr->aps_frm_ctrl.frm_type == 0x1)      ?  "COMMAND"    :
                                                        (hdr->aps_frm_ctrl.frm_type == 0x2)      ?  "ACK"        :
                                                                                                    "RESERVED"   );
    DBG_PRINT("DUMP_APS_HDR: DELIV MODE     = %s.\n",   (hdr->aps_frm_ctrl.delivery_mode == 0x0) ?  "UNICAST"    :
                                                        (hdr->aps_frm_ctrl.delivery_mode == 0x1) ?  "INDIRECT"   :
                                                        (hdr->aps_frm_ctrl.delivery_mode == 0x2) ?  "BROADCAST"  :
                                                                                                    "GROUP"      );
    DBG_PRINT("DUMP_APS_HDR: ACK FORMAT     = %s.\n",   (hdr->aps_frm_ctrl.delivery_mode != 0x1) ?  "DIRECT"    :
                                                        (hdr->aps_frm_ctrl.ack_format)           ?  "DEST EP"    : "SRC EP");
    DBG_PRINT("DUMP_APS_HDR: SECURITY       = %s.\n",   (hdr->aps_frm_ctrl.security)             ?  "PRESENT"    : "NONE");
    DBG_PRINT("DUMP_APS_HDR: ACK REQ        = %s.\n",   (hdr->aps_frm_ctrl.ack_req)              ?  "TRUE"       : "FALSE");
    DBG_PRINT("DUMP_APS_HDR: HANDLE         = %02X.\n",  hdr->handle);

    if ((hdr->aps_frm_ctrl.frm_type == APS_DATA_FRM) ||
        (hdr->aps_frm_ctrl.frm_type == APS_ACK_FRM))
    {
        prof_id_flag = clust_id_flag = true;
    }

    switch (hdr->aps_frm_ctrl.delivery_mode)
    {
    case APS_UNICAST:
        // fall through
    case APS_BROADCAST:
        src_ep_flag = dest_ep_flag = true;
        break;
    case APS_INDIRECT:
        if (hdr->aps_frm_ctrl.ack_format)
        {
            src_ep_flag = true;
        }
        else
        {
            dest_ep_flag = true;
        }
        break;
    case APS_GROUP:
        src_ep_flag  = group_addr_flag = true;
        break;
    }

    if (dest_ep_flag)       DBG_PRINT("DUMP_APS_HDR: DEST EP        = %02X.\n", hdr->dest_ep);
    if (group_addr_flag)    DBG_PRINT("DUMP_APS_HDR: GROUP ADDR     = %04X.\n", hdr->grp_addr);
    if (clust_id_flag)      DBG_PRINT("DUMP_APS_HDR: CLUST ID       = %04X.\n", hdr->clust_id);
    if (prof_id_flag)       DBG_PRINT("DUMP_APS_HDR: PROFILE ID     = %04X.\n", hdr->prof_id);
    if (src_ep_flag)        DBG_PRINT("DUMP_APS_HDR: SRC EP         = %02X.\n", hdr->src_ep);

    DBG_PRINT("DUMP_APS_HDR: APS CTR        = %02X.\n", hdr->aps_ctr);
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Decode the NWK command frame and print out the fields.
*/
/**************************************************************************/
void debug_dump_nwk_cmd(nwk_cmd_t *cmd)
{
#if (DEBUG_NWK == 1)
    DBG_PRINT_RAW("\n");
    switch (cmd->cmd_frm_id)
    {
    case NWK_CMD_RTE_REQ:
        DBG_PRINT("DUMP_NWK_CMD: CMD FRAME ID   = ROUTE REQUEST.\n");
        DBG_PRINT("DUMP_NWK_CMD: CMD OPTIONS    = %02X.\n", cmd->rreq.cmd_opts);
        DBG_PRINT("DUMP_NWK_CMD: RREQ ID        = %02X.\n", cmd->rreq.rreq_id);
        DBG_PRINT("DUMP_NWK_CMD: DESTINATION    = %04X.\n", cmd->rreq.dest_addr);
        DBG_PRINT("DUMP_NWK_CMD: PATH COST      = %02X.\n", cmd->rreq.path_cost);
        break;
    case NWK_CMD_RTE_REP:
        DBG_PRINT("DUMP_NWK_CMD: CMD FRAME ID   = ROUTE REPLY.\n");
        DBG_PRINT("DUMP_NWK_CMD: CMD OPTIONS    = %02X.\n", cmd->rrep.cmd_opts);
        DBG_PRINT("DUMP_NWK_CMD: RREQ ID        = %02X.\n", cmd->rrep.rreq_id);
        DBG_PRINT("DUMP_NWK_CMD: ORIGINATOR     = %04X.\n", cmd->rrep.originator);
        DBG_PRINT("DUMP_NWK_CMD: RESPONDER      = %04X.\n", cmd->rrep.responder);
        DBG_PRINT("DUMP_NWK_CMD: PATH COST      = %02X.\n", cmd->rrep.path_cost);
        break;

    case NWK_CMD_LEAVE:
        DBG_PRINT("DUMP_NWK_CMD: CMD FRAME ID   = LEAVE.\n");
        DBG_PRINT("DUMP_NWK_CMD: CMD OPTIONS    = %02X.\n", cmd->leave.cmd_opts);
        DBG_PRINT("DUMP_NWK_CMD: REQUEST        = %s.\n", (cmd->leave.cmd_opts & (1<<NWK_LEAVE_REQUEST_OFFSET)) ? "TRUE" : "FALSE");
        DBG_PRINT("DUMP_NWK_CMD: REJOIN         = %s.\n", (cmd->leave.cmd_opts & (1<<NWK_LEAVE_REJOIN_OFFSET)) ? "TRUE" : "FALSE");;
        DBG_PRINT("DUMP_NWK_CMD: REM CHILDREN   = %s.\n", (cmd->leave.cmd_opts & (1<<NWK_LEAVE_REM_CHILDREN_OFFSET)) ? "TRUE" : "FALSE");;
        break;
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Decode the MAC command frame and print out the fields.
*/
/**************************************************************************/
void debug_dump_mac_cmd(mac_cmd_t *cmd)
{
#if (DEBUG_MAC == 1)
    DBG_PRINT_RAW("\n");
    switch (cmd->cmd_id)
    {
    case MAC_ASSOC_REQ:
        DBG_PRINT("DUMP_MAC_CMD: CMD ID         = ASSOC REQ.\n");
        DBG_PRINT("DUMP_MAC_CMD: CAPAB INFO     = %02X.\n", cmd->assoc_req.cap_info);
        DBG_PRINT("DUMP_MAC_CMD: ALLOC ADDR     = %s.\n", \
                  ((cmd->assoc_req.cap_info >> NWK_CAPAB_ALLOC_ADDR_OFFSET) & 0x1) ? "TRUE" : "FALSE");
        DBG_PRINT("DUMP_MAC_CMD: SECURITY       = %s.\n", \
                  ((cmd->assoc_req.cap_info >> NWK_CAPAB_SECURITY_OFFSET) & 0x1) ? "TRUE" : "FALSE");
        DBG_PRINT("DUMP_MAC_CMD: RX ON IDLE     = %s.\n", \
                  ((cmd->assoc_req.cap_info >> NWK_CAPAB_RX_ON_IDLE_OFFSET) & 0x1) ? "TRUE" : "FALSE");
        DBG_PRINT("DUMP_MAC_CMD: POWER SOURCE   = %s.\n", \
                  ((cmd->assoc_req.cap_info >> NWK_CAPAB_POWER_SRC_OFFSET) & 0x1) ? "MAINS" : "BATTERY");
        DBG_PRINT("DUMP_MAC_CMD: DEVICE TYPE    = %s.\n", \
                  ((cmd->assoc_req.cap_info >> NWK_CAPAB_DEV_TYPE_OFFSET) & 0x1) ? "ROUTER" : "END DEVICE");
        DBG_PRINT("DUMP_MAC_CMD: ALT PAN COORD  = %s.\n", \
                  (cmd->assoc_req.cap_info & 0x1) ? "TRUE" : "FALSE");
        break;
    case MAC_ASSOC_RESP:
        DBG_PRINT("DUMP_MAC_CMD: CMD ID         = ASSOC RESPONSE.\n");
        DBG_PRINT("DUMP_MAC_CMD: SHORT ADDR     = %04X.\n", cmd->assoc_resp.short_addr);
        DBG_PRINT("DUMP_MAC_CMD: ASSOC STATUS   = %s.\n", \
                  (cmd->assoc_resp.assoc_status == MAC_SUCCESS) ? "SUCCESS" : "NOT ALLOWED");
        break;
    case MAC_DATA_REQ:
        DBG_PRINT("DUMP_MAC_CMD: CMD ID         = DATA REQUEST.\n");
        break;
    case MAC_BEACON_REQ:
        DBG_PRINT("DUMP_MAC_CMD: CMD ID         = BEACON REQUEST.\n");
        break;
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Dump the NWK information base.
*/
/**************************************************************************/
void debug_dump_nib()
{
#if (DEBUG_NWK == 1)
    nwk_nib_t *nib = nwk_nib_get();
    DBG_PRINT("DUMP_NWK_NIB: SEQ NUM        = %02X.\n",     nib->seq_num);
    DBG_PRINT("DUMP_NWK_NIB: MAX CHILDREN   = %02X.\n",     nib->max_children);
    DBG_PRINT("DUMP_NWK_NIB: MAX DEPTH      = %02X.\n",     nib->max_depth);
    DBG_PRINT("DUMP_NWK_NIB: MAX ROUTERS    = %02X.\n",     nib->max_routers);
    DBG_PRINT("DUMP_NWK_NIB: CONST COST     = %s.\n",       nib->report_const_cost ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_NWK_NIB: SYM LINK       = %s.\n",       nib->sym_link ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_NWK_NIB: CAPAB INFO     = %02X.\n",     nib->capability_info);
    DBG_PRINT("DUMP_NWK_NIB: TRXN PERSIST   = %04X.\n",     nib->traxn_persist_time);
    DBG_PRINT("DUMP_NWK_NIB: SHORT ADDR     = %04X.\n",     nib->short_addr);
    DBG_PRINT("DUMP_NWK_NIB: STACK PROFILE  = %02X.\n",     nib->stack_profile);
    DBG_PRINT("DUMP_NWK_NIB: EXT PAN ID     = %016llX.\n",  nib->ext_pan_ID);
    DBG_PRINT("DUMP_NWK_NIB: DEPTH          = %02X.\n",     nib->depth);
    DBG_PRINT("DUMP_NWK_NIB: RTR CAPACITY   = %02X.\n",     nib->rtr_cap);
    DBG_PRINT("DUMP_NWK_NIB: ED CAPACITY    = %02X.\n",     nib->ed_cap);
    DBG_PRINT("DUMP_NWK_NIB: CSKIP          = %04X.\n",     nib->cskip);
    DBG_PRINT("DUMP_NWK_NIB: DEV TYPE       = %s.\n",      (nib->dev_type == 1) ? "ROUTER" : (nib->dev_type == 2) ? "END DEVICE" : "COORDINATOR");
    DBG_PRINT("DUMP_NWK_NIB: JOINED         = %s.\n",       nib->joined ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_NWK_NIB: NWK MGR        = %04X.\n",     nib->nwk_mngr);
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
        Print out the neighbor table.
*/
/**************************************************************************/
void debug_dump_nbor_tbl()
{
#if (DEBUG_NWK == 1)
    U8 i = 0;
    mem_ptr_t *mem_ptr;
    
    DBG_PRINT("\n");
    for (mem_ptr = nwk_neighbor_tbl_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        DBG_PRINT("NBOR ENTRY %02X: EXT_ADDR      = %016llX.\n", i, NBOR_ENTRY(mem_ptr)->ext_addr);
        DBG_PRINT("NBOR ENTRY %02X: EXT_PAN_ID    = %016llX.\n", i, NBOR_ENTRY(mem_ptr)->ext_pan_id);
        DBG_PRINT("NBOR ENTRY %02X: NWK_ADDR      = %04X.\n", i, NBOR_ENTRY(mem_ptr)->nwk_addr);
        DBG_PRINT("NBOR ENTRY %02X: PAN_ID        = %04X.\n", i, NBOR_ENTRY(mem_ptr)->pan_id);

        switch (NBOR_ENTRY(mem_ptr)->device_type)
        {
        case NWK_COORDINATOR:   DBG_PRINT("NBOR ENTRY %02X: DEV_TYPE      = %s.\n", i, "COORDINATOR");       break;
        case NWK_ROUTER:        DBG_PRINT("NBOR ENTRY %02X: DEV_TYPE      = %s.\n", i, "ROUTER");            break;
        case NWK_END_DEVICE:    DBG_PRINT("NBOR ENTRY %02X: DEV_TYPE      = %s.\n", i, "END DEVICE");        break; 
        default:                DBG_PRINT("NBOR ENTRY %02X: DEV_TYPE      = %s.\n", i, "INVALID DEVICE");    break; 
        }

        DBG_PRINT("NBOR ENTRY %02X: RX_ON_IDLE    = %s.\n", i, NBOR_ENTRY(mem_ptr)->rx_on_when_idle ? "TRUE" : "FALSE");

        switch (NBOR_ENTRY(mem_ptr)->relationship)
        {
        case NWK_PARENT:        DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "PARENT"); break;
        case NWK_CHILD:         DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "CHILD"); break;
        case NWK_SIBLING:       DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "SIBLING"); break;
        case NWK_NONE:          DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "NONE"); break;
        case NWK_PREV_CHILD:    DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "PREV CHILD"); break;
        default:                DBG_PRINT("NBOR ENTRY %02X: RELATION      = %s.\n", i, "INVALID RELATION"); break;
        }
        
        DBG_PRINT("NBOR ENTRY %02X: CHANNEL       = %02X.\n", i, NBOR_ENTRY(mem_ptr)->channel);
        DBG_PRINT("NBOR ENTRY %02X: DEPTH         = %02X.\n", i, NBOR_ENTRY(mem_ptr)->depth);
        DBG_PRINT("NBOR ENTRY %02X: PERMIT_JOIN   = %s.\n", i, NBOR_ENTRY(mem_ptr)->permit_join ? "TRUE" : "FALSE");
        DBG_PRINT("NBOR ENTRY %02X: POT PARENT    = %s.\n", i, NBOR_ENTRY(mem_ptr)->pot_parent ? "TRUE" : "FALSE");
        DBG_PRINT("NBOR ENTRY %02X: TRANSMIT_FAIL = %02X.\n", i, NBOR_ENTRY(mem_ptr)->transmit_fail);
        DBG_PRINT("NBOR ENTRY %02X: LQI           = %02X.\n", i, NBOR_ENTRY(mem_ptr)->lqi);
        DBG_PRINT("\n");
        i++;
    }
#endif
}

/**************************************************************************/
/*!
        Converts the network status value into a human readable string for easy 
        debugging. No more 802.15.4 spec lookups!
*/
/**************************************************************************/
#if (DEBUG_NWK == 1)
static char *debug_dump_nwk_status(U8 status)
{
    switch (status)
    {
    case NWK_SUCCESS:               return "NWK_SUCCESS";               break;
    case NWK_INVALID_PARAMETER:     return "NWK_INVALID_PARAMETER";     break;           
    case NWK_INVALID_REQUEST:       return "NWK_INVALID_REQUEST";       break; 
    case NWK_NOT_PERMITTED:         return "NWK_NOT_PERMITTED";         break;
    case NWK_STARTUP_FAILURE:       return "NWK_STARTUP_FAILURE";       break;
    case NWK_ALREADY_PRESENT:       return "NWK_ALREADY_PRESENT";       break;
    case NWK_SYNC_FAILURE:          return "NWK_SYNC_FAILURE";          break;
    case NWK_TABLE_FULL:            return "NWK_TABLE_FULL";            break;
    case NWK_UNKNOWN_DEVICE:        return "NWK_UNKNOWN_DEVICE";        break;
    case NWK_UNSUPPORTED_ATTRIBUTE: return "NWK_UNSUPPORTED_ATTRIBUTE"; break;
    case NWK_NO_NETWORKS:           return "NWK_NO_NETWORKS";           break;
    case NWK_MAX_FRM_CNTR:          return "NWK_MAX_FRM_CNTR";          break;
    case NWK_NO_KEY:                return "NWK_NO_KEY";                break;
    case NWK_BAD_CCM_OUTPUT:        return "NWK_BAD_CCM_OUTPUT";        break;
    case NWK_NO_ROUTING_CAPACITY:   return "NWK_NO_ROUTING_CAPACITY";   break;
    case NWK_ROUTE_DISC_FAILED:     return "NWK_ROUTE_DISC_FAILED";     break;
    case NWK_ROUTE_ERROR:           return "NWK_ROUTE_ERROR";           break;
    case NWK_BT_TBL_FULL:           return "NWK_BT_TBL_FULL";           break;
    case NWK_FRAME_NOT_BUFFERED:    return "NWK_FRAME_NOT_BUFFERED";    break;
    default:                        return "INVALID STATUS";            break;
    }
}
#endif

/**************************************************************************/
/*!
        Dump the routing table. 
*/
/**************************************************************************/
void debug_dump_rte_tbl()
{
#if (DEBUG_NWK == 1)
    U8 i = 0;
    mem_ptr_t *mem_ptr;

    for (mem_ptr = nwk_rte_tbl_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        DBG_PRINT("RTE ENTRY %02X: DEST ADDR      = %04X.\n", i, RTE_ENTRY(mem_ptr)->dest_addr);
        DBG_PRINT("RTE ENTRY %02X: NEXT HOP       = %04X.\n", i, RTE_ENTRY(mem_ptr)->next_hop);
        DBG_PRINT("RTE ENTRY %02X: STATUS         = %s.\n", i, debug_dump_nwk_status(RTE_ENTRY(mem_ptr)->status));
        i++;
    }
#endif
}

/**************************************************************************/
/*!
    Dump the binding table
*/
/**************************************************************************/
void debug_dump_bnd_tbl()
{
#if (DEBUG_APS == 1)
    U8 i = 0;
    mem_ptr_t *mem_ptr;

    for (mem_ptr = aps_bind_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        DBG_PRINT("BIND ENTRY %02X: SRC EP        = %02X.\n", i, BIND_ENTRY(mem_ptr)->src_ep);   
        DBG_PRINT("BIND ENTRY %02X: CLUST ID      = %04X.\n", i, BIND_ENTRY(mem_ptr)->clust);   
        DBG_PRINT("BIND ENTRY %02X: DEST ADDR MDE = %s.\n", i, (BIND_ENTRY(mem_ptr)->dest_addr.mode == SHORT_ADDR) ? "SHORT ADDR" : "LONG ADDR");
        if (BIND_ENTRY(mem_ptr)->dest_addr.mode == SHORT_ADDR)
        {
            DBG_PRINT("BIND ENTRY %02X: DEST ADDR     = %04X.\n", i, BIND_ENTRY(mem_ptr)->dest_addr.short_addr);
        }
        else
        {
            DBG_PRINT("BIND_ENTRY %02X: DEST_ADDR     = %016llX.\n", i, BIND_ENTRY(mem_ptr)->dest_addr.long_addr);
        }
           
        DBG_PRINT("BIND ENTRY %02X: DEST EP       = %02X.\n", i, BIND_ENTRY(mem_ptr)->dest_ep);   
        i++;
    }
#endif
}

/**************************************************************************/
/*!
    Dump the group table
*/
/**************************************************************************/
void debug_dump_grp_tbl()
{
#if (DEBUG_APS == 1)
    U8 i = 0;
    mem_ptr_t *mem_ptr;

    for (mem_ptr = aps_grp_get_head(); mem_ptr != NULL; mem_ptr = mem_ptr->next)
    {
        DBG_PRINT("GROUP ENTRY %02X: GRP ID       = %04X.\n", i, GROUP_ENTRY(mem_ptr)->id);
        DBG_PRINT("GROUP ENTRY %02X: EP           = %02X.\n", i, GROUP_ENTRY(mem_ptr)->ep);
        i++;
    }
#endif
}

/**************************************************************************/
/*!
    Dump the MAC information base
*/
/**************************************************************************/
void debug_dump_pib()
{
#if (DEBUG_MAC == 1)
    mac_pib_t *pib = mac_pib_get();
    DBG_PRINT("DUMP_MAC_PIB: ACK WAIT DUR   = %04X.\n",     pib->ack_wait_duration);
    DBG_PRINT("DUMP_MAC_PIB: ASSOC PERMIT   = %s.\n",       pib->assoc_permit ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_MAC_PIB: DSN            = %02X.\n",     pib->dsn);
    DBG_PRINT("DUMP_MAC_PIB: PAN ID         = %04X.\n",     pib->pan_id);
    DBG_PRINT("DUMP_MAC_PIB: RX ON IDLE     = %s.\n",       pib->rx_on_when_idle ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_MAC_PIB: RESP WAIT TIME = %02X.\n",     pib->resp_wait_time);
    DBG_PRINT("DUMP_MAC_PIB: SECURITY ENB   = %s.\n",       pib->security_enb ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_MAC_PIB: SHORT ADDR     = %04X.\n",     pib->short_addr);
    DBG_PRINT("DUMP_MAC_PIB: EXT ADDR       = %016llX.\n",  pib->ext_addr);
    DBG_PRINT("DUMP_MAC_PIB: COORDINATOR    = %s.\n",       pib->coord ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_MAC_PIB: CURR CHANNEL   = %02X.\n",     pib->curr_channel);

    if (pib->coord_addr.mode == SHORT_ADDR)
    {
        DBG_PRINT("DUMP_MAC_PIB: COORD ADDR     = %04X.\n", pib->coord_addr.short_addr);
    }
    else
    {
        DBG_PRINT("DUMP_MAC_PIB: COORD ADDR     = %16llX.\n", pib->coord_addr.long_addr);
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
    Dump all of the scan descriptors.
*/
/**************************************************************************/
void debug_dump_pan_descr(pan_descr_t *desc)
{
#if (DEBUG_MAC == 1)
    DBG_PRINT("DUMP_PAN_DESCR: COORD ADDR MODE = %s.\n", (desc->coord_addr.mode == SHORT_ADDR) ? "SHORT" : "LONG");
    if (desc->coord_addr.mode == SHORT_ADDR)
    {
        DBG_PRINT("DUMP_PAN_DESCR: COORD ADDR      = %04X.\n", desc->coord_addr.short_addr);
    }
    else
    {
        DBG_PRINT("DUMP_PAN_DESCR: COORD ADDR      = %016llX.\n", desc->coord_addr.long_addr);
    }
    DBG_PRINT("DUMP_PAN_DESCR: COORD PAN ID    = %04X.\n", desc->coord_pan_id);
    DBG_PRINT("DUMP_PAN_DESCR: CHANNEL         = %02d.\n", desc->channel);
    DBG_PRINT("DUMP_PAN_DESCR: SUPERFRAME SPEC = %04X.\n", desc->superfrm_spec);
    DBG_PRINT("DUMP_PAN_DESCR: ASSOC PERMIT    = %s.\n", (desc->superfrm_spec & MAC_ASSOC_PERMIT_MASK) ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_PAN_DESCR: COORDINATOR     = %s.\n", (desc->superfrm_spec & MAC_COORDINATOR_MASK) ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_PAN_DESCR: LINK QUALITY    = %02X.\n", desc->link_quality);
    DBG_PRINT("DUMP_PAN_DESCR: PROTOCOL ID     = %02X.\n", desc->prot_id);
    DBG_PRINT("DUMP_PAN_DESCR: STACK PROFILE   = %02X.\n", desc->stack_profile);
    DBG_PRINT("DUMP_PAN_DESCR: PROTOCOL VER    = %02X.\n", desc->prot_ver);
    DBG_PRINT("DUMP_PAN_DESCR: ROUTER CAPACITY = %s.\n", (desc->rtr_cap) ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_PAN_DESCR: END DEV CAPACITY= %s.\n", (desc->end_dev_cap) ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_PAN_DESCR: EXT PAN ID      = %016llX.\n", desc->ext_pan_id);
    DBG_PRINT("DUMP_PAN_DESCR: POTENTIAL PARENT= %s.\n", (desc->pot_parent) ? "TRUE" : "FALSE");
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
        Decode the AF status value into a human readable string. No more 802.15.4 lookups!
*/
/**************************************************************************/
#if (DEBUG_ZDO == 1) || (DEBUG_ZCL == 1)
char *debug_dump_af_status(U8 status)
{
    switch (status)
    {
    case AF_SUCCESS            : return "AF_SUCCESS";
    case AF_DEVICE_NOT_FOUND   : return "AF_DEVICE_NOT_FOUND";
    case AF_INVALID_EP         : return "AF_INVALID_EP";
    case AF_NOT_ACTIVE         : return "AF_NOT_ACTIVE";
    case AF_NOT_SUPPORTED      : return "AF_NOT_SUPPORTED";
    case AF_TIMEOUT            : return "AF_TIMEOUT";
    case AF_NO_MATCH           : return "AF_NO_MATCH";
    case AF_NO_ENTRY           : return "AF_NO_ENTRY";
    case AF_NO_DESCRIPTOR      : return "AF_NO_DESCRIPTOR";
    case AF_INSUFFICIENT_SPACE : return "AF_INSUFFICIENT_SPACE";
    case AF_NOT_PERMITTED      : return "AF_NOT_PERMITTED";
    case AF_TABLE_FULL         : return "AF_TABLE_FULL";
    case AF_NOT_AUTHORIZED     : return "AF_NOT_AUTHORIZED";
    case AF_NO_FREE_BUFS       : return "AF_NO_FREE_BUFS";
    case AF_ERROR              : return "AF_ERROR";
    }
    return NULL;
}
#endif
/**************************************************************************/
/*!
    Dump the specified cluster list for the endpoint.
*/
/**************************************************************************/
void debug_dump_clust_list(clust_list_t *clust_list)
{
#if (DEBUG_ZDO == 1)
    U8 i;

    DBG_PRINT_RAW("DUMP_CLUST_LIST: CLUST_CNT           = %02X.\n", clust_list->clust_cnt);

    for (i=0; i<clust_list->clust_cnt; i++)
    {
        DBG_PRINT_RAW("DUMP_CLUST_LIST: CLUST_LIST[%d]       = %04X.\n", i, clust_list->clust_list[i]);
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
        Decode and print out the ZDO request.
*/
/**************************************************************************/
void debug_dump_zdo_request(zdo_req_t *req)
{
#if (DEBUG_ZDO == 1)
    U8 i;
    U16 *clust;

    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_ZDO_REQ: SEQ            = %02X.\n", req->seq);
    DBG_PRINT("DUMP_ZDO_REQ: CLUST          = %04X.\n", req->clust);

    switch (req->clust)
    {
    case NWK_ADDR_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK ADDR REQ");
        DBG_PRINT("DUMP_ZDO_REQ: EXT ADDR       = %016llX.\n", req->type.nwk_addr.ext_addr);
        DBG_PRINT("DUMP_ZDO_REQ: TYPE           = %s.\n", req->type.nwk_addr.req_type ? "EXTENDED" : "SINGLE");
        DBG_PRINT("DUMP_ZDO_REQ: START INDEX    = %02X.\n", req->type.nwk_addr.start_index);
        break;

    case IEEE_ADDR_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "IEEE ADDR REQ");
        DBG_PRINT("DUMP_ZDO_REQ: NWK ADDR       = %04X.\n", req->type.ieee_addr.nwk_addr);
        DBG_PRINT("DUMP_ZDO_REQ: TYPE           = %s.\n", req->type.ieee_addr.req_type ? "EXTENDED" : "SINGLE");
        DBG_PRINT("DUMP_ZDO_REQ: START INDEX    = %02X.\n", req->type.ieee_addr.start_index);
        break;

    case END_DEV_BIND_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "END DEVICE BIND REQ");
        DBG_PRINT("DUMP_ZDO_REQ: TARGET         = %04X.\n", req->type.ed_bind.target);
        DBG_PRINT("DUMP_ZDO_REQ: SRC ADDR       = %016llX.\n", req->type.ed_bind.src_ext_addr);
        DBG_PRINT("DUMP_ZDO_REQ: SRC EP         = %02X.\n", req->type.ed_bind.src_ep);
        DBG_PRINT("DUMP_ZDO_REQ: PROFILE ID     = %04X.\n", req->type.ed_bind.prof_id);

        // dump the in clusters
        DBG_PRINT("DUMP_ZDO_REQ: NUM IN CLUST   = %02X.\n", req->type.ed_bind.num_in_clust);
        DBG_PRINT("DUMP_ZDO_REQ: IN CLUST - ");
        clust = (U16 *)req->type.ed_bind.in_clust;
        for (i=0; i<req->type.ed_bind.num_in_clust; i++)
        {
            DBG_PRINT_RAW("%04X ", *clust);
            clust++;
        }
        DBG_PRINT_RAW("\n");

        // dump the out clusters
        DBG_PRINT("DUMP_ZDO_REQ: NUM OUT CLUST  = %02X.\n", req->type.ed_bind.num_out_clust);
        DBG_PRINT("DUMP_ZDO_REQ: OUT CLUST - ");
        clust = (U16 *)req->type.ed_bind.out_clust;
        for (i=0; i<req->type.ed_bind.num_out_clust; i++)
        {
            DBG_PRINT_RAW("%04X ", *clust);
            clust++;
        }
        DBG_PRINT_RAW("\n");
        break;

    case BIND_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "BIND REQ");
        DBG_PRINT("DUMP_ZDO_REQ: SRC ADDR       = %016llX.\n", req->type.bind.src_addr);
        DBG_PRINT("DUMP_ZDO_REQ: SRC EP         = %02X.\n", req->type.bind.src_ep);
        DBG_PRINT("DUMP_ZDO_REQ: CLUST          = %04X.\n", req->type.bind.clust);
        DBG_PRINT("DUMP_ZDO_REQ: DEST ADDR MODE = %02X.\n", req->type.bind.dest_addr.mode);

        if (req->type.bind.dest_addr.mode == SHORT_ADDR)
        {
            DBG_PRINT("DUMP_ZDO_REQ: GROUP ADDR     = %04X.\n", req->type.bind.dest_addr.short_addr);
        }
        else if (req->type.bind.dest_addr.mode == LONG_ADDR)
        {
            DBG_PRINT("DUMP_ZDO_REQ: DEST ADDR      = %016llX.\n", req->type.bind.dest_addr.long_addr);
            DBG_PRINT("DUMP_ZDO_REQ: DEST EP        = %02X.\n", req->type.bind.dest_ep);
        }
        break;

    case UNBIND_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "UNBIND REQ");
        DBG_PRINT("DUMP_ZDO_REQ: SRC ADDR       = %016llX.\n", req->type.bind.src_addr);
        DBG_PRINT("DUMP_ZDO_REQ: SRC EP         = %02X.\n", req->type.bind.src_ep);
        DBG_PRINT("DUMP_ZDO_REQ: CLUST          = %04X.\n", req->type.bind.clust);
        DBG_PRINT("DUMP_ZDO_REQ: DEST ADDR MODE = %02X.\n", req->type.bind.dest_addr.mode);

        if (req->type.bind.dest_addr.mode == SHORT_ADDR)
        {
            DBG_PRINT("DUMP_ZDO_REQ: GROUP ADDR     = %04X.\n", req->type.bind.dest_addr.short_addr);
        }
        else if (req->type.bind.dest_addr.mode == LONG_ADDR)
        {
            DBG_PRINT("DUMP_ZDO_REQ: DEST ADDR      = %016llX.\n", req->type.bind.dest_addr.long_addr);
            DBG_PRINT("DUMP_ZDO_REQ: DEST EP        = %02X.\n", req->type.bind.dest_ep);
        }
        break;

    case NWK_LQI_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK LQI REQ");
        DBG_PRINT("DUMP_ZDO_REQ: START_IDX      = %02X.\n", req->type.nwk_lqi.start_index);
        break;

    case NWK_DISC_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK DISC REQ");
        DBG_PRINT("DUMP_ZDO_REQ: SCAN CHANNELS  = %02X.\n", req->type.nwk_disc.scan_channels);
        DBG_PRINT("DUMP_ZDO_REQ: SCAN DURATION  = %02X.\n", req->type.nwk_disc.scan_duration);
        DBG_PRINT("DUMP_ZDO_REQ: START INDEX    = %02X.\n", req->type.nwk_disc.start_index);
        break;

    case NWK_PERMIT_JOIN_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK PERMIT JOIN REQ");
        DBG_PRINT("DUMP_ZDO_REQ: DURATION       = %02X.\n", req->type.permit_join.duration);
        DBG_PRINT("DUMP_ZDO_REQ: TC SIGNIF      = %02X.\n", req->type.permit_join.tc_sig);
        break;

    case NWK_LEAVE_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK LEAVE REQ");
        DBG_PRINT("DUMP_ZDO_REQ: ADDR           = %016llX.\n", req->type.leave.addr);
        DBG_PRINT("DUMP_ZDO_REQ: REM CHILDREN   = %s.\n", req->type.leave.rem_children ? "TRUE" : "FALSE");
        DBG_PRINT("DUMP_ZDO_REQ: REJOIN         = %s.\n", req->type.leave.rejoin ? "TRUE" : "FALSE");
        break;

    case NWK_UPDATE_REQ_CLUST:
        DBG_PRINT("DUMP_ZDO_REQ: REQ            = %s.\n", "NWK UPDATE REQ");
        DBG_PRINT("DUMP_ZDO_REQ: SCAN CHANNELS  = %08X.\n", req->type.nwk_update.scan_channels);
        DBG_PRINT("DUMP_ZDO_REQ: SCAN DURATION  = %02X.\n", req->type.nwk_update.scan_duration);

        if (req->type.nwk_update.scan_duration <= 0x5)
        {
            DBG_PRINT("DUMP_ZDO_REQ: SCAN COUNT     = %02X.\n", req->type.nwk_update.scan_cnt);
        }
        else if (req->type.nwk_update.scan_duration == 0xFE)
        {
            DBG_PRINT("DUMP_ZDO_REQ: NWK UPDATE ID  = %02X.\n", req->type.nwk_update.nwk_update_id);
        }
        else if (req->type.nwk_update.scan_duration == 0xFF)
        {
            DBG_PRINT("DUMP_ZDO_REQ: NWK UPDATE ID  = %02X.\n", req->type.nwk_update.nwk_update_id);
            DBG_PRINT("DUMP_ZDO_REQ: NWK MGR ADDR   = %04X.\n", req->type.nwk_update.nwk_mngr_addr);
        }
    }
#endif
/**************************************************************************/}
/*!
    Dump all the neighbor entries from the ZDO LQI response.
*/
/**************************************************************************/
#if (DEBUG_ZDO == 1)
static void debug_dump_nbor_entry(zdo_resp_t *resp, U8 *data)
{
    U8 i, tmp;

    for (i=0; i<resp->type.nwk_lqi.nbor_list_cnt; i++)
    {
        DBG_PRINT("LIST_ENTRY %02X: EXT PAN ID    = %016llX.\n", i, *(U64 *)data);
        data += sizeof(U64);
        DBG_PRINT("LIST_ENTRY %02X: EXT ADDR      = %016llX.\n", i, *(U64 *)data);
        data += sizeof(U64);
        DBG_PRINT("LIST_ENTRY %02X: NWK ADDR      = %04X.\n", i, *(U16 *)data);
        data += sizeof(U16);
        
        tmp = *data++;
        switch (tmp & 0x3)
        {
        case 0: DBG_PRINT("LIST_ENTRY %02X: DEV TYPE      = %s.\n", i, "COORDINATOR"); break; 
        case 1: DBG_PRINT("LIST_ENTRY %02X: DEV TYPE      = %s.\n", i, "ROUTER"); break; 
        case 2: DBG_PRINT("LIST_ENTRY %02X: DEV TYPE      = %s.\n", i, "END DEVICE"); break; 
        }

        DBG_PRINT("LIST_ENTRY %02X: NWK ADDR      = %02X.\n", i, *(U16 *)data);
        DBG_PRINT("LIST_ENTRY %02X: RX ON IDLE    = %02X.\n", i, (tmp >> 2) & 0x3);

        switch (tmp>>4 & 0x7)
        {
        case 0: DBG_PRINT("LIST_ENTRY %02X: RELATION      = %s.\n", i, "PARENT"); break;
        case 1: DBG_PRINT("LIST_ENTRY %02X: RELATION      = %s.\n", i, "CHILD"); break;
        case 2: DBG_PRINT("LIST_ENTRY %02X: RELATION      = %s.\n", i, "SIBLING"); break;
        case 3: DBG_PRINT("LIST_ENTRY %02X: RELATION      = %s.\n", i, "NONE"); break;
        }

        DBG_PRINT("LIST_ENTRY %02X: PERMIT JOIN   = %02X.\n", i, *data & 0x1); 
        data++;                                   
        DBG_PRINT("LIST_ENTRY %02X: DEPTH         = %02X.\n", i, *data);
        data++;
        DBG_PRINT("LIST_ENTRY %02X: LQI           = %02X.\n", i, *data);
        data++;
    }
}
#endif

/**************************************************************************/
/*!
    Dump all the remote nwk discovery entries in the ZDO response.
*/
/**************************************************************************/
#if (DEBUG_ZDO == 1)
static void debug_dump_disc_entry(zdo_resp_t *resp, U8 *data)
{
    U8 i;

    for (i=0; i<resp->type.nwk_disc.nwk_list_cnt; i++)
    {
        DBG_PRINT_RAW("\n");
        DBG_PRINT("DISC_ENTRY %02X: EXT PAN ID    = %016llX.\n", i, *(U64 *)data);
        data += sizeof(U64);

        DBG_PRINT("LIST_ENTRY %02X: CHANNEL       = %02X.\n", i, *data);
        data++;

        DBG_PRINT("LIST_ENTRY %02X: STACK PROFILE = %02X.\n", i, *data & 0xf);
        DBG_PRINT("LIST_ENTRY %02X: ZIGBEE VER    = %02X.\n", i, (*data >> 4) & 0xf);
        data++;

        DBG_PRINT("LIST_ENTRY %02X: BEACON ORDER  = %02X.\n", i, *data & 0xf);
        DBG_PRINT("LIST_ENTRY %02X: SUPERFRM ORD  = %02X.\n", i, (*data >> 4) & 0xf);
        data++;

        DBG_PRINT("LIST_ENTRY %02X: PERMIT JOIN   = %02X.\n", i, *data & 0x1);
        data++;
    }
}
#endif

/**************************************************************************/
/*!
    Dump the contents of the ZDO response.
*/
/**************************************************************************/
void debug_dump_zdo_resp(zdo_resp_t *resp, U8 *data)
{
#if (DEBUG_ZDO == 1)
    U8 i;

    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_ZDO_RESP: SEQ           = %02X.\n", resp->seq);
    DBG_PRINT("DUMP_ZDO_RESP: CLUST         = %04X.\n", resp->clust);

    switch (resp->clust)
    {
    case NWK_ADDR_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK ADDR RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.addr.status));
        DBG_PRINT("DUMP_ZDO_RESP: IEEE ADDR     = %016llX.\n", resp->type.addr.ext_addr_rmt);
        DBG_PRINT("DUMP_ZDO_RESP: NWK ADDR      = %04X.\n", resp->type.addr.nwk_addr_rmt);
        break;

    case IEEE_ADDR_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "IEEE ADDR RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.addr.status));
        DBG_PRINT("DUMP_ZDO_RESP: IEEE ADDR     = %016llX.\n", resp->type.addr.ext_addr_rmt);
        DBG_PRINT("DUMP_ZDO_RESP: NWK ADDR      = %04X.\n", resp->type.addr.nwk_addr_rmt);
        break;

    case END_DEV_BIND_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "END DEV BIND RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.ed_bind.status));
        break;

    case BIND_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "BIND RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.bind.status));
        break;

    case UNBIND_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "UNBIND RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.bind.status));
        break;

    case NWK_LQI_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK LQI RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.nwk_lqi.status));
        DBG_PRINT("DUMP_ZDO_RESP: TOT ENTRIES   = %02X.\n", resp->type.nwk_lqi.nbor_tbl_cnt);
        DBG_PRINT("DUMP_ZDO_RESP: START INDEX   = %02X.\n", resp->type.nwk_lqi.start_index);
        DBG_PRINT("DUMP_ZDO_RESP: LIST COUNT    = %02X.\n", resp->type.nwk_lqi.nbor_list_cnt);
        debug_dump_nbor_entry(resp, data);
        break;

    case NWK_DISC_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK DISC RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.nwk_disc.status));
        DBG_PRINT("DUMP_ZDO_RESP: NWK COUNT     = %02X.\n", resp->type.nwk_disc.nwk_cnt);
        DBG_PRINT("DUMP_ZDO_RESP: START IDX     = %02X.\n", resp->type.nwk_disc.start_index);
        DBG_PRINT("DUMP_ZDO_RESP: ENTRY COUNT   = %02X.\n", resp->type.nwk_disc.nwk_list_cnt);
        debug_dump_disc_entry(resp, data);
        break;

    case NWK_PERMIT_JOIN_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK PERMIT JOIN RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.permit_join.status));
        break;

    case NWK_LEAVE_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK LEAVE RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.leave.status));
        break;
        
    case NWK_UPDATE_RESP_CLUST:
        DBG_PRINT("DUMP_ZDO_RESP: RESP          = %s.\n", "NWK UPDATE RESPONSE");
        DBG_PRINT("DUMP_ZDO_RESP: STATUS        = %s.\n", debug_dump_af_status(resp->type.nwk_update.status));
        DBG_PRINT("DUMP_ZDO_RESP: SCAN CHANNELS = %08X.\n", resp->type.nwk_update.scan_channels);
        DBG_PRINT("DUMP_ZDO_RESP: XMITS         = %04X.\n", resp->type.nwk_update.total_xmits);
        DBG_PRINT("DUMP_ZDO_RESP: XMIT FAILS    = %04X.\n", resp->type.nwk_update.xmit_failures);
        DBG_PRINT("DUMP_ZDO_RESP: LIST COUNT    = %02X.\n", resp->type.nwk_update.scan_channel_list_cnt);
        
        for (i=0; i<resp->type.nwk_update.scan_channel_list_cnt; i++)
        {
            DBG_PRINT_RAW("%02X ", *data);
            data++;
        }
        DBG_PRINT_RAW("\n");
        break;
    }
#endif
}

/**************************************************************************/
/*!
    Decode and dump the simple descriptor.
*/
/**************************************************************************/
void debug_dump_simple_desc(U8 *desc)
{
#if (DEBUG_ZDO == 1)
    simple_desc_t *simple_desc = (simple_desc_t *)desc;
    clust_list_t *in_list, *out_list;
    U8 in_size, i;

    in_list = (clust_list_t *)simple_desc->clust_info;
    in_size = 1 + in_list->clust_cnt * sizeof(U16);
    out_list = (clust_list_t *)simple_desc->clust_info + in_size;

    DBG_PRINT("DUMP_SIMPLE_DESC: EP ID              = %02X.\n", simple_desc->ep);
    DBG_PRINT("DUMP_SIMPLE_DESC: APP PROF ID        = %04X.\n", simple_desc->prof_id);
    DBG_PRINT("DUMP_SIMPLE_DESC: APP DEV ID         = %04X.\n", simple_desc->dev_id);
    DBG_PRINT("DUMP_SIMPLE_DESC: APP DEV VER        = %02X.\n", simple_desc->dev_ver);

    DBG_PRINT("DUMP_SIMPLE_DESC: CLUST IN CNT       = %02X.\n", in_list->clust_cnt);
    for (i=0; i<in_list->clust_cnt; i++)
    {
        DBG_PRINT("DUMP_SIMPLE_DESC: CLUST IN LIST[%d]   = %02X.\n", i, in_list->clust_list[i]);
    }

    for (i=0; i<out_list->clust_cnt; i++)
    {
        DBG_PRINT("DUMP_SIMPLE_DESC: CLUST OUT LIST[%d]  = %02X.\n", i, out_list->clust_list[i]);
    }
#endif // DEBUG_FRMS
}

/**************************************************************************/
/*!
        Decode the attribute type value into human readable form for printing to 
        screen.
*/
/**************************************************************************/
#if (DEBUG_ZCL == 1)
static void debug_dump_attrib_type(U8 type)
{
    switch (type)
    {
    case ZCL_TYPE_U8:           DBG_PRINT(", TYPE: %s", "UNSIGNED CHAR");   break;
    case ZCL_TYPE_U16:          DBG_PRINT(", TYPE: %s", "UNSIGNED INT");    break;
    case ZCL_TYPE_U32:          DBG_PRINT(", TYPE: %s", "UNSIGNED WORD");   break;
    case ZCL_TYPE_BOOL:         DBG_PRINT(", TYPE: %s", "BOOL");            break;
    case ZCL_TYPE_CHAR_STRING:  DBG_PRINT(", TYPE: %s", "CHAR STRING");     break; 
    default:                                                                    break; 
    }
}
#endif

/**************************************************************************/
/*!
        Dump the attribute data. Both the attribute type and value will get printed out.
        Currently, only supports certain attribute types.
*/
/**************************************************************************/
#if (DEBUG_ZCL == 1)
static U8 debug_dump_attrib_data(U8 type, U8 *data)
{
    U8 size, str_buf[128], *data_ptr = data;
    switch (type)
    {
    case ZCL_TYPE_U8:
        DBG_PRINT_RAW("TYPE: %s, ", "UNSIGNED CHAR");
        DBG_PRINT_RAW("DATA: %02X", *data_ptr);
        data_ptr++;
        break;

    case ZCL_TYPE_U16:
        DBG_PRINT_RAW("TYPE: %s, ", "UNSIGNED INT");
        DBG_PRINT_RAW("DATA: %04X", *(U16 *)data_ptr);
        data_ptr += sizeof(U16);
        break;

    case ZCL_TYPE_U32:
        DBG_PRINT_RAW("TYPE: %s, ", "UNSIGNED WORD");
        DBG_PRINT_RAW("DATA: %08X", *(U32 *)data_ptr);
        data_ptr += sizeof(U32);
        break;

    case ZCL_TYPE_BOOL:
        DBG_PRINT_RAW("TYPE: %s, ", "BOOL");
        DBG_PRINT_RAW("DATA: %s", *data_ptr ? "TRUE" : "FALSE");
        data_ptr++;
        break;

    case ZCL_TYPE_CHAR_STRING:
        DBG_PRINT_RAW("TYPE: %s, ", "CHAR STRING");

        // byte 0 is the length of the string
        size = *(U8 *)data_ptr++;

        // start the copy from byte 1 
        memcpy(str_buf, data_ptr, size); 
        data_ptr += size;
        str_buf[size] = 0;
        
        DBG_PRINT_RAW("DATA: %s", str_buf);
        break; 
    default:
        break; 
    }
    return data_ptr - data;
}
#endif // DEBUG_FRMS

/**************************************************************************/
/*!

*/
/**************************************************************************/
#if (DEBUG_ZCL == 1)
static char *debug_dump_zcl_cmd(U16 clust, U8 cmd_id)
{
    switch (clust)
    {
    case ZCL_IDENTIFY_CLUST_ID:
        if (cmd_id == ZCL_ID_CMD_ID) return "IDENTIFY";

    case ZCL_ON_OFF_CLUST_ID:
        switch (cmd_id)
        {
        case ZCL_ON_OFF_CMD_OFF:    return "OFF";
        case ZCL_ON_OFF_CMD_ON:     return "ON";
        case ZCL_ON_OFF_CMD_TOGGLE: return "TOGGLE";
        }
    }
    return NULL;
}
#endif

/**************************************************************************/
/*!
        Decode the ZCL status value into human readable form. No more 802.15.4 lookups!
*/
/**************************************************************************/
char *debug_dump_zcl_status(U8 status)
{
#if (DEBUG_ZCL == 1)
    switch (status)
    {
    case ZCL_STATUS_SUCCESS:                return "ZCL SUCCESS";                
    case ZCL_STATUS_FAIL:                   return "ZCL FAIL";                    
    case ZCL_STATUS_MALFORMED_CMD:          return "ZCL MALFORMED CMD";           
    case ZCL_STATUS_UNSUP_CLUST_CMD:        return "ZCL UNSUP CLUST CMD";         
    case ZCL_STATUS_UNSUP_GEN_CMD:          return "ZCL UNSUP GEN CMD";           
    case ZCL_STATUS_UNSUP_MANUF_CLUST_CMD:  return "ZCL UNSUP MANUF CLUST CMD";   
    case ZCL_STATUS_UNSUP_MANUF_GEN_CMD:    return "ZCL UNSUP MANUF GEN CMD";     
    case ZCL_STATUS_INVALID_FIELD:          return "ZCL INVALID FIELD";           
    case ZCL_STATUS_UNSUP_ATTRIB:           return "ZCL UNSUP ATTRIB";            
    case ZCL_STATUS_INVALID_VALUE:          return "ZCL INVALID VALUE";           
    case ZCL_STATUS_READ_ONLY:              return "ZCL READ ONLY";               
    case ZCL_STATUS_INSUFF_SPACE:           return "ZCL INSUFF SPACE";            
    case ZCL_STATUS_DUPE_EXISTS:            return "ZCL DUPE EXISTS";             
    case ZCL_STATUS_NOT_FOUND:              return "ZCL NOT FOUND";               
    case ZCL_STATUS_UNREPORTABLE_ATTRIB:    return "ZCL UNREPORTABLE ATTRIB";     
    case ZCL_STATUS_INVALID_TYPE:           return "ZCL INVALID TYPE";            
    case ZCL_STATUS_HW_FAIL:                return "ZCL HW FAIL";                 
    case ZCL_STATUS_SW_FAIL:                return "ZCL SW FAIL";                 
    case ZCL_STATUS_CALIB_ERR:              return "ZCL CALIB ERR";               
    default:                                return "ZCL UNKNOWN STATUS";          
    }
#endif // DEBUG_FRMS
    return NULL;
}

/**************************************************************************/
/*!
        Decode and dump the ZCL frame.
*/
/**************************************************************************/
void debug_dump_zcl_frm(U8 *data, zcl_hdr_t *hdr, U16 clust)
{
#if (DEBUG_ZCL == 1)
    U8 i, *data_ptr, status, dir, type, len;
    U16 attrib_id;

    DBG_PRINT_RAW("\n");
    DBG_PRINT("DUMP_ZCL_FRM: FRM TYPE       = %s.\n", hdr->frm_ctrl.frm_type        ? "SPECIFIC" : "GENERAL");
    DBG_PRINT("DUMP_ZCL_FRM: MANUF SPECIFIC = %s.\n", hdr->frm_ctrl.manuf_spec      ? "TRUE" : "FALSE");
    DBG_PRINT("DUMP_ZCL_FRM: DIRECT         = %s.\n", hdr->frm_ctrl.dir             ? "TO CLIENT" : "TO SERVER");
    DBG_PRINT("DUMP_ZCL_FRM: DIS DEF RESP   = %s.\n", hdr->frm_ctrl.dis_def_resp    ? "TRUE" : "FALSE");

    if (hdr->frm_ctrl.manuf_spec)
    {
        DBG_PRINT("DUMP_ZCL_FRM: MANUF CODE     = %4X.\n",hdr->manuf_code);
    }

    DBG_PRINT("DUMP_ZCL_FRM: SEQ NUM        = %02X.\n", hdr->seq_num);

    if (hdr->frm_ctrl.frm_type == ZCL_FRM_TYPE_GENERAL)
    {
        switch (hdr->cmd)
        {
        case ZCL_CMD_READ_ATTRIB:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "READ ATTRIBUTES");
            DBG_PRINT("DUMP_ZCL_FRM: ATTRIBS: ");

            data_ptr = hdr->payload;
            for (i=0; i<(hdr->payload_len)/2; i++)
            {
                DBG_PRINT("%04X ", *(U16 *)data_ptr);
                data_ptr += sizeof(U16);
            }
            DBG_PRINT("\n");
            break;

        case ZCL_CMD_READ_ATTRIB_RESP:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "READ ATTRIBUTE RESPONSE");

            data_ptr = hdr->payload;
            while ((data_ptr - hdr->payload) < hdr->payload_len)
            {
                DBG_PRINT("ATTRIB ID: %04X", *(U16 *)data_ptr);
                data_ptr += sizeof(U16);

                status = *data_ptr++;
                DBG_PRINT(", STATUS: %02X", status);

                if (status == ZCL_STATUS_SUCCESS)
                {
                    // since the data is variable, there is a special function to handle dumping it.
                    // just pass in the type and the data ptr. then increment the data ptr when 
                    // we're done.
                    type = *data_ptr++;
                    len = debug_dump_attrib_data(type, data_ptr);
                    data_ptr += len;
                }
                DBG_PRINT("\n");
            }
            break;

        case ZCL_CMD_WRITE_ATTRIB:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "WRITE ATTRIBUTES");

            data_ptr = hdr->payload;
            while ((data_ptr - hdr->payload) <  hdr->payload_len)
            {
                DBG_PRINT("ATTRIB ID: %04X, ", *(U16 *)data_ptr);
                data_ptr += sizeof(U16);

                // since the data is variable, there is a special function to handle dumping it.
                // just pass in the type and the data ptr. then increment the data ptr when 
                // we're done.
                type = *data_ptr++;
                len = debug_dump_attrib_data(type, data_ptr);
                data_ptr += len;
                DBG_PRINT("\n");
            }
            break;

        case ZCL_CMD_WRITE_ATTRIB_RESP:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "WRITE ATTRIBUTE RESP");

            data_ptr = hdr->payload;
            while ((data_ptr - hdr->payload) < hdr->payload_len)
            {
                status = *data_ptr++;
                attrib_id = *(U16 *)data_ptr;
                data_ptr += sizeof(U16);
                DBG_PRINT("Attrib ID: %04X, ", attrib_id);
                DBG_PRINT("DUMP_ZCL_FRM: STATUS         = %s.\n", debug_dump_zcl_status(status));
            }
            break;

        case ZCL_CMD_DISC_ATTRIB:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "DISC ATTRIBUTES");

            data_ptr = hdr->payload;
            DBG_PRINT("DUMP_ZCL_FRM: START ID       = %04X.\n", *(U16 *)data_ptr);
            data_ptr += sizeof(U16);
            DBG_PRINT("DUMP_ZCL_FRM: NUM OF ATTRIBS = %02X.\n", *data_ptr);
            data_ptr++;
            break;

        case ZCL_CMD_DISC_ATTRIB_RESP:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "DISC ATTRIBUTES RESP");

            data_ptr = hdr->payload;
            DBG_PRINT("DUMP_ZCL_FRM: DISC COMPLETE  = %s.\n", *data_ptr ? "TRUE" : "FALSE");
            data_ptr++;

            while ((data_ptr - hdr->payload) < hdr->payload_len)
            {
                attrib_id = *(U16 *)data_ptr;
                data_ptr += sizeof(U16);
                type = *data_ptr++;

                DBG_PRINT("Attrib ID: %04X, ", attrib_id);
                debug_dump_attrib_type(type);
                DBG_PRINT("\n");
            }
            break;

        case ZCL_CMD_CONFIG_REPORT_RESP:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "CONFIG ATTRIBUTES RESP");
            data_ptr = hdr->payload;

            status      = *data_ptr++;
            dir         = *data_ptr++;
            attrib_id   = *(U16 *)data_ptr;
            data_ptr += sizeof(U16);
            DBG_PRINT("DUMP_ZCL_FRM: ATTRIB ID = %04X.\n ", attrib_id);
            DBG_PRINT("DIR                          = %s.\n ", *data_ptr ? "RECEIVE REPORT" : "SEND REPORT");
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", debug_dump_zcl_status(status));
            break;

        case ZCL_CMD_REPORT_ATTRIB:
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "REPORT ATTRIBUTES");
            data_ptr = hdr->payload;

            attrib_id = *(U16 *)data_ptr;
            data_ptr += sizeof(U16);
            type = *data_ptr++;

            DBG_PRINT("DUMP_ZCL_FRM: ATTRIB ID = %04X, ", attrib_id);
            len = debug_dump_attrib_data(type, data_ptr);
            data_ptr += len;
            DBG_PRINT_RAW("\n");
            break;

        case ZCL_CMD_DEFAULT_RESP:
            data_ptr = hdr->payload;
            DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "DEFAULT RESPONSE");
            DBG_PRINT("DUMP_ZCL_FRM: CMD ID         = %s, ", debug_dump_zcl_cmd(clust, *data_ptr));
            data_ptr++;
            DBG_PRINT("DUMP_ZCL_FRM: STATUS         = %s, ", debug_dump_zcl_status(*data_ptr));
            break;
        }
    }
    else
    {
        switch (clust)
        {
        case ZCL_IDENTIFY_CLUST_ID:
            if (hdr->frm_ctrl.dir == ZCL_FRM_DIR_TO_SRVR)
            {
                switch (hdr->cmd)
                {
                case ZCL_ID_CMD_ID:
                    data_ptr = hdr->payload;
                    DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "IDENTIFY");
                    DBG_PRINT("DUMP_ZCL_FRM: TIMEOUT        = %04X.\n", *(U16 *)data_ptr);
                    break;
    
                case ZCL_ID_CMD_ID_QUERY:
                    DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "IDENTIFY QUERY");
                    break;
                }
            }
            else
            {
                if (hdr->cmd == ZCL_ID_CMD_ID_QUERY_RESP)
                {
                    data_ptr = hdr->payload;
                    DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "IDENTIFY QUERY RESP");
                    DBG_PRINT("DUMP_ZCL_FRM: TIME REMAINING = %04X.\n", *(U16 *)data_ptr);
                }
            }
            break;
            
        case ZCL_ON_OFF_CLUST_ID:
            switch (hdr->cmd)
            {
            case ZCL_ON_OFF_CMD_OFF:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "OFF");
                break;
            case ZCL_ON_OFF_CMD_ON:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "ON");
                break;
            case ZCL_ON_OFF_CMD_TOGGLE:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "TOGGLE");
                break;
            }
            break;

        case ZCL_LEVEL_CLUST_ID:
            data_ptr = hdr->payload;
            switch (hdr->cmd)
            {
            case ZCL_LEVEL_CMD_MOVE_TO_LEVEL:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "MOVE TO LEVEL");
                DBG_PRINT("DUMP_ZCL_FRM: LEVEL          = %02X.\n", *data_ptr);
                data_ptr++;
                DBG_PRINT("DUMP_ZCL_FRM: TRANS TIME     = %04X.\n", *(U16 *)data_ptr);
                break;

            case ZCL_LEVEL_CMD_MOVE:

                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "MOVE");
                DBG_PRINT("DUMP_ZCL_FRM: DIR            = %s.\n", *data_ptr ? "DOWN" : "UP");
                data_ptr++;
                DBG_PRINT("DUMP_ZCL_FRM: RATE           = %02X.\n", *data_ptr);
                break;

            case ZCL_LEVEL_CMD_STEP:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "MOVE TO LEVEL");
                DBG_PRINT("DUMP_ZCL_FRM: STEP           = %02X.\n", *data_ptr);
                data_ptr++;
                DBG_PRINT("DUMP_ZCL_FRM: STEP SIZE      = %02X.\n", *data_ptr);
                data_ptr++;
                DBG_PRINT("DUMP_ZCL_FRM: TRANS TIME     = %04X.\n", *(U16 *)data_ptr);
                break;

            case ZCL_LEVEL_CMD_STOP:
                DBG_PRINT("DUMP_ZCL_FRM: CMD            = %s.\n", "STOP");
                break;
            }
            break;
        }
    }
#endif
}
