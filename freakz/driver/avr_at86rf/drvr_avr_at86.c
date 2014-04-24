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
 *  POSSIBILITY OF SUCH DAMAGE.**/

/*!
    [Akiba] Gratuitously modified and massaged into FreakZ by me :) [/Akiba]

    \defgroup atdrvr Driver - AT86RF23x
    \file drvr_avr_at86.c
    \ingroup atdrvr
    \brief AT86RF230 driver file
*/
#include <stdio.h>
#include "freakz.h"
#include "hal.h"
#include "drvr_avr_at86.h"
#include "at86rf230_registermap.h"

#if (TEST_RAVENUSB == 1)
#define BLUE_LED    1<<7 // Port D
#define RED_LED     1<<5 // Port D
#define GREEN_LED   1<<7 // Port E
#define ORANGE_LED  1<<6 // Port E
#endif

/* Main driver process for at86 driver */
PROCESS(drvr_process, "AT86RF Driver Process");
extern process_event_t event_drvr_conf;
/* Driver control block instantiation */
static at86_dcb_t dcb;

/* This function checks if the radio transceiver is sleeping.
 *
 * true    The radio transceiver is in SLEEP or one of the *_NOCLK
 *             states.
 * false   The radio transceiver is not sleeping.
 */
bool at86_is_sleeping(void)
{
	/*
	 * The radio transceiver will be at SLEEP or one of the
	 * *_NOCLK states only if the SLP_TR pin is high.
	 */
	if (hal_get_slptr())
		return true;
	else
		return false;
}

/*
 * This function will reset all the registers and the state machine of
 * the radio transceiver.
 */
void drvr_at86_reset()
{
	hal_set_rst_low();
	hal_set_slptr_low();
	delay_us(TIME_RESET);
	hal_set_rst_high();
}

/*
 * This function will reset the state machine (to TRX_OFF) from any of
 * its states, except for the SLEEP state.
 */
void drvr_reset_fsm()
{

	/* The data sheet is not clear on what happens when slptr is raised in RX on
	 * states, it "remains in the new state and returns to the preceding state
	 * when slptr is lowered". Possibly that is why there is an undocumented
	 * TIME_NOCLK_TO_WAKE delay here?
	 */
	if (at86_is_sleeping()) {
		hal_set_slptr_low();
		delay_us(TIME_NOCLK_TO_WAKE);
	}
	hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
	delay_us(TIME_CMD_FORCE_TRX_OFF);
}

/*
 * Get the driver control block. When called, it will return a pointer
 * the driver control block which holds information that is needed by other
 * areas of the driver. Plus its nice to have all the flags and status in
 * one place.
 */
at86_dcb_t *drvr_dcb_get()
{
	return &dcb;
}

void drvr_init_leds()
{
#if (TEST_RAVENUSB == 1)
	/* init the LEDs for the Raven USB board */
	DDRD |= BLUE_LED | RED_LED;
	DDRE |= GREEN_LED | ORANGE_LED;

	PORTD &= ~BLUE_LED;                 // Blue LED is active high
	PORTD |= RED_LED;                   // Red LED is active low
	PORTE |= GREEN_LED | ORANGE_LED;    // both green and orange leds are active low
#endif
}

void drvr_set_leds(bool on)
{
#if (TEST_RAVENUSB == 1)
	if (on) {
		PORTD |= BLUE_LED;
		PORTD &= ~RED_LED;
		PORTE &= ~(GREEN_LED | ORANGE_LED);
	} else {
		PORTD &= ~BLUE_LED;
		PORTD |= RED_LED;	// Red LED is active low
		PORTE |= GREEN_LED | ORANGE_LED; // both green and orange leds are active low
	}
#endif
}

void drvr_toggle_leds()
{
#if (TEST_RAVENUSB == 1)
	PORTD ^= BLUE_LED | RED_LED;
	PORTE ^= GREEN_LED | ORANGE_LED;
#endif
}

/*
 * This sets the handle of the frame in the driver control block. It will
 * be used later when the confirmation gets sent up the stack to identify
 * the frame that the confirmation pertains to.
 */
void drvr_set_handle(uint8_t handle)
{
	dcb.handle = handle;
}

/*
 * Set the transmission status. This will also be used by the confirmation
 * which will be sent up the stack from the MAC layer. The status will be
 * included in the confirmation and will signal the higher layers whether the
 * transmission was successful or not.
 */
void drvr_set_status(uint8_t status)
{
	dcb.status = status;
	dcb.status_avail = true;
	process_poll(&drvr_process);
}

/*
 * Signal that data was received. This gets used in the driver process polling
 * mechanism. If data is received, it will notify the MAC and begin to process
 * the incoming frame.
 */
void drvr_set_data_rx_flag(bool flag)
{
	dcb.data_rx = flag;
	if (flag)
		process_poll(&drvr_process);
}

/*
 * This function will enable or disable automatic CRC during frame
 * transmission.
 *
 * auto_crc_on If this parameter equals true auto CRC will be used for
 * all frames to be transmitted. If this is set, then the hardware will
 * automatically generate the two byte frame checksum tacked on to the
 * end of the frame. If the parameter equals false, the automatic
 * CRC will be disabled.
 */
void drvr_set_auto_crc(bool auto_crc_on) {
	if (auto_crc_on)
		hal_subregister_write(SR_TX_AUTO_CRC_ON, 1);
	else
		hal_subregister_write(SR_TX_AUTO_CRC_ON, 0);
}

/*
 * Initialize the random seed. Random values are used a lot in the stack
 * and this function will seed the pseudorandom number generator so that
 * it won't always start at the same value. The seed will be taken from
 * the TCNT0 timer which is free running.
 */
void drvr_set_rand_seed()
{
	srand(TCNT0);
}

/*
 * Get a random number. This function returns a random 16-bit number
 * to the function that calls it.
 */
uint16_t drvr_get_rand()
{
	return (uint16_t)rand();
}

/*
 * Get the Tx/Rx state. It will read the hardware registers and return
 * the current state of the hardware's TX/RX FSM.
 */

/* This function return the Radio Transceivers current state.
 *
 * P_ON               When the external supply voltage (VDD) is
 *                    first supplied to the transceiver IC, the
 *                    system is in the P_ON (Poweron) mode.
 * BUSY_RX            The radio transceiver is busy receiving a
 *                    frame.
 * BUSY_TX            The radio transceiver is busy transmitting a
 *                    frame.
 * RX_ON              The RX_ON mode enables the analog and digital
 *                    receiver blocks and the PLL frequency
 *                    synthesizer.
 * TRX_OFF            In this mode, the SPI module and crystal
 *                    oscillator are active.
 * PLL_ON             Entering the PLL_ON mode from TRX_OFF will
 *                    first enable the analog voltage regulator. The
 *                    transceiver is ready to transmit a frame.
 * BUSY_RX_AACK       The radio was in RX_AACK_ON mode and received
 *                    the Start of Frame Delimiter (SFD). State
 *                    transition to BUSY_RX_AACK is done if the SFD
 *                    is valid.
 * BUSY_TX_ARET       The radio transceiver is busy handling the
 *                    auto retry mechanism.
 * RX_AACK_ON         The auto acknowledge mode of the radio is
 *                    enabled and it is waiting for an incomming
 *                    frame.
 * TX_ARET_ON         The auto retry mechanism is enabled and the
 *                    radio transceiver is waiting for the user to
 *                    send the TX_START command.
 * RX_ON_NOCLK        The radio transceiver is listening for
 *                    incomming frames, but the CLKM is disabled so
 *                    that the controller could be sleeping.
 *                    However, this is only true if the controller
 *                    is run from the clock output of the radio.
 * RX_AACK_ON_NOCLK   Same as the RX_ON_NOCLK state, but with the
 *                    auto acknowledge module turned on.
 * BUSY_RX_AACK_NOCLK Same as BUSY_RX_AACK, but the controller
 *                    could be sleeping since the CLKM pin is
 *                    disabled.
 * STATE_TRANSITION   The radio transceiver's state machine is in
 *                    transition between two states.
 */
uint8_t drvr_get_trx_state()
{
	return hal_subregister_read(SR_TRX_STATUS);
}

/*
 * Returns the channel that the hardware is currently set to
 * Current channel, 11 to 26.
 */
uint8_t drvr_get_channel()
{
	return hal_subregister_read(SR_CHANNEL);
}

/* This function will change the operating channel.
 *
 * channel New channel to operate on. Must be between 11 and 26.
 *
 * RADIO_SUCCESS New channel set.
 * RADIO_WRONG_STATE Transceiver is in a state where the channel cannot be changed (SLEEP).
 * RADIO_INVALID_ARGUMENT Channel argument is out of bounds.
 * RADIO_TIMED_OUT The PLL did not lock within the specified time.
 */
/*
 * Sets the channel in the hardware. Changing a channel requires a certain
 * amount of delay as the hardware and PLL sync up to the new channel. This
 * function also returns a status in case the channel change was unsuccessful.
 * In a normal case, this would probably only happen if the channel that was
 * set is outside the 16 channels supported by 802.15.4.
 */
uint8_t drvr_set_channel(uint8_t channel)
{
	uint8_t state;

	if ((channel < RF230_MIN_CHANNEL) || (channel > RF230_MAX_CHANNEL))
		return RADIO_INVALID_ARGUMENT;

	if (at86_is_sleeping())
		return RADIO_WRONG_STATE;

	if (drvr_get_channel() == channel)
		return RADIO_SUCCESS;

	hal_subregister_write(SR_CHANNEL, channel);

	/* add a delay to allow the PLL to lock if in active mode */
	state = drvr_get_trx_state();
	if ((state == RX_ON) || (state == PLL_ON))
		delay_us(TIME_PLL_LOCK);

	if (drvr_get_channel() == channel)
		return RADIO_SUCCESS;

	return RADIO_TIMED_OUT;
}

/*
 * Set the coordinator bit in the hardware. This signals to the hardware
 * that the device is the network coordinator. Basically, the only benefit of
 * having this set in hardware is that the hardware auto filter will only
 * accept frames that have source addressing (no destination addressing) if
 * the device is a PAN coordinator. It might be a relic of the Zigbee 2006
 * spec where the coordinator would contain the binding table. The Zigbee 2007
 * Residential and Pro specs should have source and dest addressing on all frames.
 */
void drvr_set_coord(bool coord)
{
	hal_subregister_write(SR_I_AM_COORD, coord);
}

/* Get the coordinator bit from the hardware */
bool drvr_get_coord()
{
	return hal_subregister_read(SR_I_AM_COORD);
}

/*
 * Get the TX power setting from hardware
 *
 * 0 to 15 Current output power in "TX power settings" as defined in
 * the radio transceiver's datasheet
 */
uint8_t drvr_get_tx_pwr()
{
	return hal_subregister_read(SR_TX_PWR);
}

/*
 * Set the TX power setting in hardware.
 * These are the settings and the corresponding TX power levels:
 * 0x0     3.0
 * 0x1     2.8
 * 0x2     2.3
 * 0x3     1.8
 * 0x4     1.3
 * 0x5     0.7
 * 0x6     0.0
 * 0x7     -1
 * 0x8     -2
 * 0x9     -3
 * 0xA     -4
 * 0xB     -5
 * 0xC     -7
 * 0xD     -9
 * 0xE     -12
 * 0xF     -17
 */
/* This function will change the output power level.
 *
 * power_level New output power level in the "TX power settings"
 *             as defined in the radio transceiver's datasheet.
 *
 * RADIO_SUCCESS New output power set successfully.
 * RADIO_INVALID_ARGUMENT The supplied function argument is out of bounds.
 * RADIO_WRONG_STATE It is not possible to change the TX power when the
 *                 device is sleeping.
 */
uint8_t drvr_set_tx_pwr(uint8_t power_level)
{
	/*Check function parameter and state.*/
	if (power_level > TX_PWR_17_2DBM)
		return RADIO_INVALID_ARGUMENT;

	if (at86_is_sleeping())
		return RADIO_WRONG_STATE;

	hal_subregister_write(SR_TX_PWR, power_level & 0xf);

	return RADIO_SUCCESS;
}

/*
 * Set the PAN ID in hardware. When the auto frame filtering is used, the hardware
 * will discard any frames that don't match the PAN ID set in the device.
 * the value is from 0 to 0xFFFF
 */
void drvr_set_pan_id(uint16_t new_pan_id)
{
	uint8_t pan_byte;

	pan_byte = new_pan_id & 0xFF; /*  Extract new_pan_id_7_0. */
	hal_register_write(RG_PAN_ID_0, pan_byte);

	pan_byte = (new_pan_id >> 8*1) & 0xFF;  /*  Extract new_pan_id_15_8. */
	hal_register_write(RG_PAN_ID_1, pan_byte);
}

/* Get the PAN ID of the device. Returns the 16-bit PAN ID */
uint16_t drvr_get_pan_id()
{
	uint8_t pan_id_15_8, pan_id_7_0;
	uint16_t pan_id = 0;

	pan_id_15_8 = hal_register_read(RG_PAN_ID_1); /*  Read pan_id_15_8. */
	pan_id_7_0 = hal_register_read(RG_PAN_ID_0); /*  Read pan_id_7_0. */

	pan_id = ((uint16_t)(pan_id_15_8 << 8)) | pan_id_7_0;

	return pan_id;
}

/*
 * Set the frame pending bit in hardware. When a data request command frame
 * is sent, the frame pending bit in the ACK indicates if there is data pending for the
 * device. If software detects an indirect transfer pending for the device
 * sending the request, then this bit will need to get set.
 */
void drvr_set_frm_pend(bool pend)
{
    hal_subregister_write(SR_AACK_SET_PD, pend);
}

/** \brief  This function will return the current short address used by the
 *          address filter.
 *
 *  \retval Any value from 0x0000 to 0xFFFF
 */
uint16_t drvr_get_short_addr(void)
{

	uint8_t short_address_15_8, short_address_7_0;
	uint16_t short_address;

	short_address_15_8 = hal_register_read(RG_SHORT_ADDR_1); /*  Read short_address_15_8. */
	short_address_7_0  = hal_register_read(RG_SHORT_ADDR_1); /*  Read short_address_7_0. */

	short_address = ((uint16_t)(short_address_15_8 << 8)) | short_address_7_0;

	return short_address;
}

/** \brief  This function will set the short address used by the address filter.
 *
 *  \param  new_short_address Short address to be used by the address filter.
 */
void drvr_set_short_addr(uint16_t new_short_address)
{

    uint8_t short_address_byte = new_short_address & 0xFF; /*  Extract short_address_7_0. */
    hal_register_write(RG_SHORT_ADDR_0, short_address_byte);

    short_address_byte = (new_short_address >> 8*1) & 0xFF; /*  Extract short_address_15_8. */
    hal_register_write(RG_SHORT_ADDR_1, short_address_byte);
}

/* Set the extended address */
void drvr_set_ext_addr(U64 addr)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		hal_register_write(RG_IEEE_ADDR_0 + i, addr >> (8 * i));
	}
}

/* Get the extended address */
U64 drvr_get_ext_addr()
{
	uint8_t i;
	U64 addr = 0;

	for (i = 0; i < 8; i++) {
		addr |= hal_register_read(RG_IEEE_ADDR_0 + i) << (8 * i);
	}
	return addr;
}

/*
 * Configure the CSMA. This consists of setting the number of frame retries (in case
 * of tx failure)...actually, it's not part of CSMA but thought it should go in here,
 * the max CSMA retries (ie: number of CSMA backoffs allowed), the min backoff exponent,
 * and the CSMA seeds. When two devices try to transmit at the same time,
 */
uint8_t drvr_config_csma(uint8_t seed0, uint8_t seed1, uint8_t min_be, uint8_t frame_retries, uint8_t csma_retries)
{
	/* make sure the device ain't sleeping */
	if (at86_is_sleeping())
		return RADIO_WRONG_STATE;

	/*
	 * note: in revA, max frame retries should be set to 0
	 * due to errata. i'm assuming i only have rev B parts
	 * with no known errata...i hope...
	 */
	hal_subregister_write(SR_MAX_FRAME_RETRIES, frame_retries);
	hal_subregister_write(SR_MAX_CSMA_RETRIES, csma_retries);
	hal_subregister_write(SR_MIN_BE, min_be);
	hal_register_write(RG_CSMA_SEED_0, seed0);
	hal_subregister_write(SR_CSMA_SEED_1, (seed1 & 0x7));
	return RADIO_SUCCESS;
}

/*
 * Set the clear channel assessment threshold and mode here. CCA is used
 * for CSMA to determine whether another device is transmitting. The Atmel
 * chips allow different methods to calculate a clear channel such as using
 * the carrier sense or also including energy detection. Please see datasheet
 * for more details.
 */
uint8_t drvr_set_cca(uint8_t mode, uint8_t ed_thresh)
{
	if ((mode != CCA_ED) &&
	    (mode != CCA_CARRIER_SENSE) &&
	    (mode != CCA_CARRIER_SENSE_WITH_ED))
		return RADIO_INVALID_ARGUMENT;

	if (at86_is_sleeping())
		return RADIO_WRONG_STATE;

	/*Change cca mode and ed threshold.*/
	hal_subregister_write(SR_CCA_MODE, mode);
	hal_subregister_write(SR_CCA_ED_THRES, ed_thresh);

	return RADIO_SUCCESS;
}

/* Get the clear channel assessment mode */
uint8_t drvr_get_cca()
{
	return hal_subregister_read(SR_CCA_MODE);
}

/*
 * Get the energy detection value for the current channel. This normally
 * used in network formation to find a channel with low activity.
 */
uint8_t drvr_get_ed()
{
	/*
	 * ED register needs to be written with a dummy
	 * value to initiate a manual ED reading. The ED
	 * level will appear after approx 8 symbol times
	 * or 128 usec.
	 */
	hal_register_write(RG_PHY_ED_LEVEL, 0);
	delay_us(140);
	return hal_register_read(RG_PHY_ED_LEVEL);
}

/*
 * Set the TX/RX state machine state. Some manual manipulation is required
 * for certain operations. Check the datasheet for more details on the state
 * machine and manipulations.
 */
uint8_t drvr_set_trx_state(uint8_t state)
{
	uint8_t current_state, delay;

	/* if we're sleeping then don't allow transition */
	if (at86_is_sleeping())
		return RADIO_WRONG_STATE;

	/*
	 * if we're in a transition state, wait for the
	 * state to become stable
	 */
	current_state = drvr_get_trx_state();
	if ((current_state == BUSY_TX_ARET) ||
	    (current_state == BUSY_RX_AACK) ||
	    (current_state == BUSY_RX) ||
	    (current_state == BUSY_TX))
	{
		while (drvr_get_trx_state() == current_state);
	}

	/*
	 * At this point it is clear that the requested new_state is:
	 * TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON.
	 * we need to handle some special cases before we transition
	 * to the new state
	 */

	/*
	 * The radio transceiver can be in one of the following states:
	 * TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON
	 */
	if(state == TRX_OFF){
		drvr_reset_fsm();
	} else {
		/*
		 * It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly
		 * to TX_AACK_ON or RX_AACK_ON respectively. Need to go via PLL_ON.
		 * (Old datasheets allowed other transitions, but this code complies
		 * with the current specification for RF230, RF231 and 128RFA1.)
		 */
		if (((state == TX_ARET_ON) && (current_state == RX_AACK_ON)) ||
		    ((state == RX_AACK_ON) && (current_state == TX_ARET_ON))) {
			/*
			 * First do intermediate state transition to PLL_ON.
			 * The final state transition is handled after the
			 * if-else if.
			 */
			hal_subregister_write(SR_TRX_CMD, PLL_ON);
			delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
		}
	}

	/* Now we're okay to transition to any new state. */
	hal_subregister_write(SR_TRX_CMD, state);

	/* When the PLL is active most states can be reached in 1us. However, from */
	/* TRX_OFF the PLL needs time to activate. */
	delay = (current_state == TRX_OFF) ? TIME_TRX_OFF_TO_PLL_ACTIVE :
					  TIME_STATE_TRANSITION_PLL_ACTIVE;
	delay_us(delay);

	if (drvr_get_trx_state() == state)
		return RADIO_SUCCESS;

	return RADIO_TIMED_OUT;
}

/*
 * This is the main function in the driver which transmits data and hence
 * a very important function. The fsm needs to be set to the tx state to allow
 * transmission to take place. The buffer is then loaded, and data is transmitted
 * when the sleep pin is strobed. You can also trigger transmission from the
 * registers, but strobing the sleep pin seemed to work in the original Contiki
 * driver so I kept it.
 *
 * TODO: need to modify this and see if its cool to write the frame data
 * to the buffer before initiate the transmission...that's just kind of weird...
 */
uint8_t drvr_tx(const buffer_t *buf)
{
	uint8_t state = drvr_get_trx_state();

	if ((state == BUSY_TX) || (state == BUSY_TX_ARET))
		return RADIO_WRONG_STATE;

	/*
	 * TODO: check why we need to transition to the off
	 * state before we go to tx_aret_on
	 */
	drvr_set_trx_state(TRX_OFF);
	drvr_set_trx_state(TX_ARET_ON);

	/*
	 * TODO: try and start the frame transmission by writing
	 * TX_START command instead of toggling sleep pin...i just
	 * feel like it's kind of weird...
	 */

	/* write frame to buffer */
	hal_frame_write(buf->dptr, buf->len);

	/*
	 * Do frame transmission. Toggle the SLP_TR pin to initiate
	 * the frame transmission.
	 */
	hal_set_slptr_high();
	hal_set_slptr_low();

	/*
	 * TODO: if we're in extended operating mode, check for ACK or
	 * NO_ACK return codes...
	 */
	return RADIO_SUCCESS;
}

/*
 * Initialize the driver. This function gets called when the Contiki OS is
 * initialized.
 */
void drvr_init()
{
	uint8_t part_num, ver_num, irq;
	uint16_t man_id = 0;

	memset(&dcb, 0, sizeof(at86_dcb_t));

	delay_us(TIME_TO_ENTER_P_ON);
	hal_init();

	/* reset all regs in at86rf */
	drvr_at86_reset();

	part_num = hal_register_read(AT86_PART_NUM);
	ver_num = hal_register_read(AT86_VERSION_NUM);
	man_id |= hal_register_read(AT86_MAN_ID_1) << 8;
	man_id |= hal_register_read(AT86_MAN_ID_0);

	hal_register_write(AT86_IRQ_MASK, 0);
	irq = hal_register_read(AT86_IRQ_STATUS);

	/* force transceiver off while we configure the intps */
	hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
	delay_us(TIME_P_ON_TO_TRX_OFF);

	/* wait for transceiver to transition to the off state */
	while (drvr_get_trx_state() != TRX_OFF);

	hal_register_write(AT86_IRQ_MASK, (1 << IRQ_MASK_TRX_END) | (1 << IRQ_MASK_RX_START));

	/* configure the CSMA parameters */
	drvr_config_csma(drvr_get_rand() & 0xf,
			 drvr_get_rand() & 0xf,
			 aMinBE,
			 aMacMaxFrameRetries,
			 aMaxCsmaBackoffs);

	/* set the default channel */
	drvr_set_channel(11);

	/* set autocrc mode */
	drvr_set_auto_crc(true);

	/* start the contiki driver process and register the event number */
	process_start(&drvr_process, NULL);
	event_drvr_conf = process_alloc_event();

	/* put trx in rx auto ack mode */
	drvr_set_trx_state(RX_AACK_ON);
	while (drvr_get_trx_state() != RX_AACK_ON);
}

/*
 * The Contiki driver process will poll this function constantly. The poll
 * function does two things:
 * 1) Check for received data
 * 2) Check for the status after a transmission
 *
 * The received data flag will trigger the MAC layer to pick up the frame
 * from the queue and start to process it.
 *
 * If a transmit status is available, then it will generate a confirm that
 * gets propagated up the stack.
 */
static void pollhandler()
{
	uint8_t status;

	if (dcb.data_rx) {
		process_post(&mac_process, event_mac_rx, NULL);
		dcb.data_rx = false;
	} else if (dcb.status_avail) {
		switch (dcb.status)
		{
		case RADIO_SUCCESS:
			status = MAC_SUCCESS;
			break;
		case RADIO_NO_ACK:
			status = MAC_NO_ACK;
			break;
		case RADIO_CHANNEL_ACCESS_FAILURE:
			status = MAC_CHANNEL_ACCESS_FAILURE;
			break;
		default:
			status = MAC_UNSPECIFIED_FAILURE;
			break;
		}
		process_post(&mac_process, event_drvr_conf, &dcb);
		dcb.status_avail = false;
	}

	process_poll(&drvr_process);
}

/*
 * The main process for the driver. Actually this process doesn't do anything
 * other than call the polling function.
 *
 * According to the Contiki documentation, an ISR cannot signal a process event
 * directly, so a polling function needs to be used.
 */
PROCESS_THREAD(drvr_process, ev, data)
{
	PROCESS_POLLHANDLER(pollhandler());

	PROCESS_BEGIN();

	process_poll(&drvr_process);

	while (1) {
		PROCESS_YIELD();
	}
	PROCESS_END();
}
