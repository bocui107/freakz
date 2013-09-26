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
    \file debug.h
    \ingroup misc
    \brief Debug header file

    This is the header file for the debug utilities.
*/
/*******************************************************************/
#ifndef FREAKZ_DEBUG_H
#define FREAKZ_DEBUG_H

#include <stdio.h>
#include "aps.h"
#include "nwk.h"
#include "mac.h"
#include "zdo.h"
#include "zcl.h"

#if (TEST_SIM)
extern FILE *fp;
#endif

#if (FREAKZ_DEBUG)
    #if (TEST_SIM)
        #define DBG_PRINT(...)                                  \
            do                                                  \
            {                                                   \
                mac_pib_t *pib = mac_pib_get();                 \
                printf("<Addr: %04X>", pib->short_addr);        \
                printf(__VA_ARGS__);                            \
                fflush(stdout);                                 \
                fprintf(fp, "<Addr: %04X> ", pib->short_addr);  \
                fprintf(fp, __VA_ARGS__);                       \
                fflush(fp);                                     \
            } while(0);

        #define DBG_PRINT_RAW(...)                      \
            do                                          \
            {                                           \
                printf(__VA_ARGS__);                    \
                fflush(stdout);                         \
                fprintf(fp, __VA_ARGS__);               \
                fflush(fp);                             \
            } while(0);

        #define DBG_PRINT_SIMONLY(...)                  \
            do                                          \
            {                                           \
                printf(__VA_ARGS__);                    \
                fflush(stdout);                         \
                fprintf(fp, __VA_ARGS__);               \
                fflush(fp);                             \
            } while (0);
    #else
        #define DBG_PRINT(...) printf(__VA_ARGS__);
        #define DBG_PRINT_RAW(...) printf(__VA_ARGS__);
        #define DBG_PRINT_SIMONLY(...)
    #endif //TEST_SIM
#else
    #define DBG_PRINT(...)
    #define DBG_PRINT_RAW(...)
    #define DBG_PRINT_SIMONLY(...)
#endif //FREAKZ_DEBUG

void debug_dump_buf(const U8 *buf, U8 len);
void debug_dump_mac_hdr(const mac_hdr_t *hdr);
void debug_dump_nwk_hdr(const nwk_hdr_t *hdr);
void debug_dump_aps_hdr(const aps_hdr_t *hdr);
void debug_dump_nwk_cmd(nwk_cmd_t *cmd);
void debug_dump_pan_descr(pan_descr_t *desc);
void debug_dump_mac_cmd(mac_cmd_t *cmd);
void debug_dump_nib();
void debug_dump_pib();
void debug_dump_nbor_tbl();
void debug_dump_rte_tbl();
void debug_dump_bnd_tbl();
void debug_dump_grp_tbl();
void debug_dump_simple_desc(U8 *desc);
void debug_dump_clust_list(clust_list_t *clust_list);
void debug_dump_zdo_request(zdo_req_t *req);
void debug_dump_zdo_resp(zdo_resp_t *resp, U8 *data);
void debug_dump_zcl_frm(U8 *data, zcl_hdr_t *hdr, U16 clust);
char *debug_dump_af_status(U8 status);

#endif // FREAKZ_DEBUG_H
