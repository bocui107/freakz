/*******************************************************************
 *   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *      Colin O'Flynn coflynn@newae.com
 *      Eric Gnoske egnoske@gmail.com
 *      Blake Leverett bleverett@gmail.com
 *      Mike Vidales mavida404@gmail.com
 *      Kevin Brown kbrown3@uccs.edu
 *      Nate Bohlmann nate@elfwerks.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
*******************************************************************/
/*!
    [Akiba] Gratuitously modified and massaged into FreakZ by me :) [/Akiba]

    \file drvr_avr_at86.h
    \ingroup atdrvr
    \brief AT86RF230 driver header file
*/
/*******************************************************************/
#ifndef DRVR_AVR_AT86RF_H
#define DRVR_AVR_AT86RF_H

#include <avr/io.h>
#include <util/delay_basic.h>
#include "contiki.h"
#include "types.h"
#include "buf.h"

/* 1 usec delay */
#define delay_us(us)	(_delay_loop_2((F_CPU / 4000000UL) * (us)))

#define IRQ_MASK_PLL_LOCK	0	/* IRQ reg bit - PLL Lock */
#define IRQ_MASK_PLL_UNLOCK	1	/* IRQ reg bit - PLL Unlock */
#define IRQ_MASK_RX_START	2	/* IRQ reg bit - RX Start */
#define IRQ_MASK_TRX_END	3	/* IRQ reg bit - TRX End */

/* AT86RF23x register address definitions */
typedef enum
{
	AT86_TRX_STATUS     = 0x01,
	AT86_TRX_STATE      = 0x02,
	AT86_TRX_CTRL_0     = 0x03,
	AT86_TRX_CTRL_1     = 0x04,
	AT86_PHY_TX_PWR     = 0x05,
	AT86_PHY_RSSI       = 0x06,
	AT86_PHY_ED_LEVEL   = 0x07,
	AT86_PHY_CC_CCA     = 0x08,
	AT86_CCA_THRES      = 0x09,
	AT86_RX_CTRL        = 0x0a,
	AT86_SFD_VAL        = 0x0b,
	AT86_TRX_CTRL2      = 0x0c,
	AT86_ANT_DIV        = 0x0d,
	AT86_IRQ_MASK       = 0x0e,
	AT86_IRQ_STATUS     = 0x0f,
	AT86_VREG_CTRL      = 0x10,
	AT86_BATMON         = 0x11,
	AT86_XOSC_CTRL      = 0x12,
	AT86_RX_SYN         = 0x15,
	AT86_XAH_CTRL_1     = 0x17,
	AT86_FTN_CTRL       = 0x18,
	AT86_PLL_CF         = 0x1a,
	AT86_PLL_DCU        = 0x1b,
	AT86_PART_NUM       = 0x1c,
	AT86_VERSION_NUM    = 0x1d,
	AT86_MAN_ID_0       = 0x1e,
	AT86_MAN_ID_1       = 0x1f,
	AT86_SHORT_ADDR_0   = 0x20,
	AT86_SHORT_ADDR_1   = 0x21,
	AT86_PAN_ID_0       = 0x22,
	AT86_PAN_ID_1       = 0x23,
	AT86_IEEE_ADDR_0    = 0x24,
	AT86_IEEE_ADDR_1    = 0x25,
	AT86_IEEE_ADDR_2    = 0x26,
	AT86_IEEE_ADDR_3    = 0x27,
	AT86_IEEE_ADDR_4    = 0x28,
	AT86_IEEE_ADDR_5    = 0x29,
	AT86_IEEE_ADDR_6    = 0x2a,
	AT86_IEEE_ADDR_7    = 0x2b,
	AT86_XAH_CTRL_0     = 0x2c,
	AT86_CSMA_SEED_0    = 0x2d,
	AT86_CSMA_SEED_1    = 0x2e,
	AT86_CSMA_BE        = 0x2f
} at86_reg_t;

/*
 * This enumeration defines the necessary timing information for the
 * AT86RF230 radio transceiver. All times are in microseconds.
 *
 * These constants are extracted from the datasheet.
 */
typedef enum{
	TIME_TO_ENTER_P_ON     = 510, /* Transition time from VCC is applied to P_ON. */
	TIME_P_ON_TO_TRX_OFF   = 510, /* Transition time from P_ON to TRX_OFF. */
	TIME_SLEEP_TO_TRX_OFF  = 880, /* Transition time from SLEEP to TRX_OFF. */
	TIME_RESET             = 6,   /* Time to hold the RST pin low during reset */
	TIME_ED_MEASUREMENT    = 140, /* Time it takes to do a ED measurement. */
	TIME_CCA               = 140, /* Time it takes to do a CCA. */
	TIME_PLL_LOCK          = 150, /* Maximum time it should take for the PLL to lock. */
	TIME_FTN_TUNING        = 25,  /* Maximum time it should take to do the filter tuning. */
	TIME_NOCLK_TO_WAKE     = 6,   /* Transition time from *_NOCLK to being awake. */
	TIME_CMD_FORCE_TRX_OFF = 1,   /* Time it takes to execute the FORCE_TRX_OFF command. */
	TIME_TRX_OFF_TO_PLL_ACTIVE  = 180, /* Transition time from TRX_OFF to: RX_ON, PLL_ON, TX_ARET_ON and RX_AACK_ON. */
	TIME_STATE_TRANSITION_PLL_ACTIVE = 1, /* Transition time from PLL active state to another. */
} at86_radio_timing_t;

/*
 * Radio status enumerations. These are the status values that can be
 * returned by the radio.
 */
typedef enum{
	RADIO_SUCCESS = 0x40,          /* The requested service was performed successfully. */
	RADIO_UNSUPPORTED_DEVICE,      /* The connected device is not an Atmel AT86RF230. */
	RADIO_INVALID_ARGUMENT,        /* One or more of the supplied function arguments are invalid. */
	RADIO_TIMED_OUT,               /* The requested service timed out. */
	RADIO_WRONG_STATE,             /* The end-user tried to do an invalid state transition. */
	RADIO_BUSY_STATE,              /* The radio transceiver is busy receiving or transmitting. */
	RADIO_STATE_TRANSITION_FAILED, /* The requested state transition could not be completed. */
	RADIO_CCA_IDLE,                /* Channel is clear, available to transmit a new frame. */
	RADIO_CCA_BUSY,                /* Channel busy. */
	RADIO_TRX_BUSY,                /* Transceiver is busy receiving or transmitting data. */
	RADIO_BAT_LOW,                 /* Measured battery voltage is lower than voltage threshold. */
	RADIO_BAT_OK,                  /* Measured battery voltage is above the voltage threshold. */
	RADIO_CRC_FAILED,              /* The CRC failed for the actual frame. */
	RADIO_CHANNEL_ACCESS_FAILURE,  /* The channel access failed during the auto mode. */
	RADIO_NO_ACK,                  /* No acknowledge frame was received. */
}at86_radio_status_t;

/* Clear channel assessment enumerations */
typedef enum{
	CCA_ED                    = 0, /* Use energy detection above threshold mode. */
	CCA_CARRIER_SENSE         = 1, /* Use carrier sense mode. */
	CCA_CARRIER_SENSE_WITH_ED = 2  /* Use a combination of both energy detection and carrier sense. */
}at86_radio_cca_mode_t;

/*
 * Driver Control Block
 * This structure contains flags and information that is used to control
 * the behavior of the driver and allow it to respond to rx and tx events.
 */
typedef struct
{
	U8 handle;         /* Frame handle used to identify the frame and assoc it with a status. */
	U8 status;         /* Transmit status after a transmission attempt */
	bool status_avail; /* Status available flag. Used to signal the driver that a new status is avail */
	bool data_rx;      /* Data rx flag. Used to signal the driver that data has been received */
} at86_dcb_t;

PROCESS_NAME(drvr_process); /* Main AT86 driver process */

void drvr_init();
void drvr_at86_reset();
void drvr_reset_fsm();
at86_dcb_t *drvr_dcb_get();
void drvr_set_frm_pend(bool pend);
void drvr_set_auto_crc(bool auto_crc_on);
void drvr_set_handle(U8 handle);
void drvr_set_status(U8 status);
void drvr_set_data_rx_flag(bool flag);
void drvr_set_rand_seed();
U16 drvr_get_rand();
U8 drvr_get_trx_state();
U8 drvr_get_channel();
U8 drvr_set_channel(U8 channel);
void drvr_set_coord(bool coord);
bool drvr_get_coord();
void drvr_set_pan_id(U16 pan_id);
U16 drvr_get_pan_id();
void drvr_set_short_addr(U16 addr);
U16 drvr_get_short_addr();
void drvr_set_ext_addr(U64 addr);
U64 drvr_get_ext_addr();
U8 drvr_config_csma(U8 seed0, U8 seed1, U8 min_be, U8 frame_retries, U8 csma_retries);
U8 drvr_set_cca(U8 mode, U8 ed_thresh);
U8 drvr_get_cca();
U8 drvr_set_trx_state(U8 state);
U8 drvr_get_ed();
U8 drvr_tx(const buffer_t *buf);
void drvr_init_leds();
void drvr_set_leds(bool on);
void drvr_toggle_leds();
#endif
