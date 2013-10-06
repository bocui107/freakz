/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
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
 *  $Id: hal.c,v 1.2 2008/10/14 18:37:28 c_oflynn Exp $
*/

/**
 *   \ingroup atdrvr
 *   \brief AVR/AT86RF23x hardware abstraction layer
*/

/**
 *  \file hal.c
 *  This file contains low-level radio driver code.
 */
#include <stdlib.h>

#include "hal.h"
#include "at86rf230_registermap.h"
#include "drvr_avr_at86.h"

/*
 * TODO: Remove this and put some type of hook for this
 * function to preserve layering
 */
#include "mac.h"

/*
 * Macros defined for the radio transceiver's access modes.
 *
 * These functions are implemented as macros since they are used very often.
 */
#define HAL_DUMMY_READ		(0x00) /* Dummy value for the SPI. */

#define HAL_TRX_CMD_RW		(0xC0) /* Register Write (short mode). */
#define HAL_TRX_CMD_RR		(0x80) /* Register Read (short mode). */
#define HAL_TRX_CMD_FW		(0x60) /* Frame Transmit Mode (long mode). */
#define HAL_TRX_CMD_FR		(0x20) /* Frame Receive Mode (long mode). */
#define HAL_TRX_CMD_SW		(0x40) /* SRAM Write. */
#define HAL_TRX_CMD_SR		(0x00) /* SRAM Read. */
#define HAL_TRX_CMD_RADDRM	(0x7F) /* Register Address Mask. */

static unsigned char test_flag;

/*
 * This is a file internal variable that contains the 16 MSB of the
 * system time.
 *
 * The system time (32-bit) is the current time in microseconds. For the
 * AVR microcontroller implementation this is solved by using a 16-bit
 * timer (Timer1) with a clock frequency of 1MHz. The hal_system_time is
 * incremented when the 16-bit timer overflows, representing the 16 MSB.
 * The timer value it self (TCNT1) is then the 16 LSB.
 *
 * hal_get_system_time
 */

/*
 * This function initializes the Hardware Abstraction Layer.
 */
void hal_init(void)
{
    test_flag = 0;

    /*IO Specific Initialization */
    DDR_SLP_TR |= (1 << SLP_TR); /* Enable SLP_TR as output */
    DDR_RST |= (1 << RST); /* Enable RST as output */

    /* Set SS, CLK and MOSI as output */
    HAL_DDR_SPI  |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK) | (1 << HAL_DD_MOSI);
    HAL_PORT_SPI |= (1 << HAL_DD_SS) | (1 << HAL_DD_SCK); /* Set SS and CLK high */
    /* Run SPI at max speed */
    SPCR = (1 << SPE) | (1 << MSTR); /* Enable SPI module and master operation */
    SPSR = (1 << SPI2X); /* Enable doubled SPI speed in master mode */

    /* TIMER1 Specific Initialization */
    TCCR1B = HAL_TCCR1B_CONFIG; /* Set clock prescaler */
    TIFR1 |= (1 << ICF1); /* Clear Input Capture Flag */
    HAL_ENABLE_OVERFLOW_INTERRUPT(); /* Enable Timer1 overflow interrupt */

    hal_enable_trx_interrupt(); /* Enable interrupts from the radio transceiver */
}

/* Write a byte to the SPI. Return the read value */
U8 hal_spi_write(U8 data)
{
	SPDR = data;
	while (!(SPSR & (1<<SPIF)));
	return SPDR;
}

/*
 * This function reads data from one of the radio transceiver's registers.
 *
 * address: Register address to read from. See datasheet for register
 * map.
 *
 * Look at the at86rf230_registermap.h file for register address definitions.
 *
 * The actual value of the read register.
 */
U8 hal_register_read(U8 address)
{
	U8 reg_val = 0;

	/* Add the register read command to the register address. */
	address &= HAL_TRX_CMD_RADDRM;
	address |= HAL_TRX_CMD_RR;

	AVR_ENTER_CRITICAL_REGION();

	/* Start the SPI transaction by pulling the Slave Select low. */
	HAL_SS_LOW();

	/*Send Register address and read register content.*/
	reg_val = hal_spi_write(address);
	reg_val = hal_spi_write(reg_val);

	/* End the transaction by pulling the Slave Select High. */
	HAL_SS_HIGH();

	AVR_LEAVE_CRITICAL_REGION();

	return reg_val;
}

/*
 * This function writes a new value to one of the radio transceiver's
 * registers.
 *
 * Look at the at86rf230_registermap.h file for register address definitions.
 *
 * address: Address of register to write.
 * value: Value to write.
 */
void hal_register_write(U8 address, U8 value)
{
	U8 dummy_read;

	/* Add the Register Write command to the address. */
	address = HAL_TRX_CMD_RW | (HAL_TRX_CMD_RADDRM & address);

	AVR_ENTER_CRITICAL_REGION();

	/* Start the SPI transaction by pulling the Slave Select low. */
	HAL_SS_LOW();

	/*Send Register address and write register content.*/
	dummy_read = hal_spi_write(address);
	dummy_read = hal_spi_write(value);

	/* End the transaction by pulling the Slave Slect High. */
	HAL_SS_HIGH();

	AVR_LEAVE_CRITICAL_REGION();
}

/*
 * This function reads the value of a specific subregister.
 *
 * Look at the at86rf230_registermap.h file for register and subregister
 * definitions.
 *
 * address:  Main register's address.
 * mask:  Bit mask of the subregister.
 * position:   Bit position of the subregister
 */
U8 hal_subregister_read(U8 address, U8 mask, U8 position)
{
	/* Read current register value and mask out subregister. */
	U8 register_value = hal_register_read(address);
	register_value &= mask;
	register_value >>= position; /* Align subregister value. */

	return register_value;
}

/*
 * This function writes a new value to one of the radio transceiver's
 * subregisters.
 *
 * Look at the at86rf230_registermap.h file for register and subregister
 * definitions.
 *
 * address:  Main register's address.
 * mask:  Bit mask of the subregister.
 * position:  Bit position of the subregister
 * value:  Value to write into the subregister.
 */
void hal_subregister_write(U8 address, U8 mask, U8 position, U8 value)
{
	/* Read current register value and mask area outside the subregister. */
	U8 register_value = hal_register_read(address);
	register_value &= ~mask;

	/* Start preparing the new subregister value. shift in place and mask. */
	value <<= position;
	value &= mask;

	value |= register_value; /* Set the new subregister value. */

	/* Write the modified register value. */
	hal_register_write(address, value);
}

/*
 * This function will upload a frame from the radio transceiver's frame
 * buffer.
 *
 * If the frame currently available in the radio transceiver's frame buffer
 * is out of the defined bounds. Then the frame length, lqi value and crc
 * be set to zero. This is done to indicate an error.
 */
void hal_frame_read(buffer_t *buf)
{
	U8 len, dummy, *rx_data=0;

	/* check that we have either valid frame pointer or callback pointer */
	if (!buf)
		return;

	AVR_ENTER_CRITICAL_REGION();

	HAL_SS_LOW();

	/* Send frame read command and read the length */
	dummy = hal_spi_write(HAL_TRX_CMD_FR);
	len = hal_spi_write(dummy);

	/*
	 * copy data into the buffer starting from the back.
	 * it will be easier to forward in case we're not the
	 * final destination. Also, we won't store the last
	 * two bytes (RSSI).
	 */
	buf->dptr = &buf->buf[aMaxPHYPacketSize - (len - 2)];

	/* Check for correct frame length */

	/*
	 * TODO: CHECK FOR THE CRC VALID BIT TO QUALIFY THE FRAME
	 * check the length of the frame to make sure the incoming frame
	 * doesn't overflow the buffer
	 */
	if ((len >= HAL_MIN_FRAME_LENGTH) && (len <= HAL_MAX_FRAME_LENGTH))
	{
		rx_data     = buf->dptr;
		buf->len    = len - 2;

		/* Upload frame buffer to data pointer. Calculate CRC */
		/* we already read the first byte out of the buffer */
		SPDR = len - 1;
		while ((SPSR & (1 << SPIF)) == 0) {;}

		/*
		 * only extract the data. leave the last 2 bytes to be
		 * manually extracted
		 */
		len -= 2;

		/* extract data from fifo */
		do {
			dummy = SPDR;
			SPDR = 0;       /*  dummy write */
			*rx_data++ = dummy;
			while ((SPSR & (1 << SPIF)) == 0) {;}
		} while (--len > 0);

		/*
		 * read off the last two bytes which are the RSSI.
		 * we're just going to discard them.
		 */
		dummy = SPDR;
		SPDR = 0;       /*  dummy write */
		while ((SPSR & (1 << SPIF)) == 0) {
			;
		}
		dummy = SPDR;
		SPDR = 0;       /*  dummy write */
		while ((SPSR & (1 << SPIF)) == 0) {
			;
		}

		/*Read LQI value for this frame.*/
		buf->lqi = SPDR;
		HAL_SS_HIGH();
	} else {
		HAL_SS_HIGH();

		buf->len = 0;
		buf->lqi = 0;
	}

	AVR_LEAVE_CRITICAL_REGION();
}

/*
 * This function will download a frame to the radio transceiver's frame
 * buffer.
 *
 * data: Pointer to data that is to be written to frame buffer.
 * len: Length of data. The maximum length is 127 bytes.
 */
void hal_frame_write(U8 *data, U8 len)
{
	U8 dummy;
	len &= HAL_TRX_CMD_RADDRM; /* Truncate length to maximum frame length. */

	AVR_ENTER_CRITICAL_REGION();

	/* Initiate the SPI transaction */
	HAL_SS_LOW();

	/* SEND FRAME WRITE COMMAND AND FRAME LENGTH */
	dummy = hal_spi_write(HAL_TRX_CMD_FW);
	dummy = hal_spi_write(*data++);
	len--;

	/* Download to the Frame Buffer. */
	do {
		SPDR = *data++;
		--len;

		while ((SPSR & (1 << SPIF)) == 0) {;}

		dummy = SPDR;
	} while (len > 0);

	HAL_SS_HIGH(); /* Terminate SPI transaction. */

	AVR_LEAVE_CRITICAL_REGION();
}

/*
 * Read SRAM
 *
 * This function reads from the SRAM of the radio transceiver.
 *
 * address: Address in the TRX's SRAM where the read burst should start
 * length: Length of the read burst
 * data: Pointer to buffer where data is stored.
 */
void hal_sram_read(U8 address, U8 length, U8 *data)
{
	U8 dummy;

	AVR_ENTER_CRITICAL_REGION();

	/* Initiate the SPI transaction. */
	HAL_SS_LOW();

	/* Send SRAM read command */
	dummy = hal_spi_write(HAL_TRX_CMD_SR);

	/* Send address where to start reading */
	dummy = hal_spi_write(address);

	/* Upload the chosen memory area */
	do {
		SPDR = HAL_DUMMY_READ;
		while ((SPSR & (1 << SPIF)) == 0) {;}
		*data++ = SPDR;
	} while (--length > 0);

	HAL_SS_HIGH();

	AVR_LEAVE_CRITICAL_REGION();
}

/*
 * \brief Write SRAM
 *
 * This function writes into the SRAM of the radio transceiver.
 *
 * \param address Address in the TRX's SRAM where the write burst should start
 * \param length  Length of the write burst
 * \param data    Pointer to an array of bytes that should be written
 */
void hal_sram_write(U8 address, U8 length, U8 *data)
{
	U8 dummy;

	AVR_ENTER_CRITICAL_REGION();

	HAL_SS_LOW();

	/* Send SRAM write command */
	dummy = hal_spi_write(HAL_TRX_CMD_SW);

	/* Send address where to start writing to */
	dummy = hal_spi_write(address);

	/* Upload the chosen memory area */
	do {
		SPDR = *data++;
		while ((SPSR & (1 << SPIF)) == 0) {;}
		dummy = SPDR;
	} while (--length > 0);

	HAL_SS_HIGH();

	AVR_LEAVE_CRITICAL_REGION();
}

/*
 * \brief ISR for the radio IRQ line, triggered by the input capture.
 *  This is the interrupt service routine for timer1.ICIE1 input capture.
 *  It is triggered of a rising edge on the radio transceivers IRQ line.
 */
ISR(RADIO_VECT)
{
	U8 dummy, intp_src = 0;
	buffer_t *buf;

	/* Read Interrupt source */
	HAL_SS_LOW();

	/* Send Register address and read register content */
	dummy = hal_spi_write(RG_IRQ_STATUS | HAL_TRX_CMD_RR);
	intp_src = hal_spi_write(dummy);

	HAL_SS_HIGH();

	/* Handle the incomming interrupt. Prioritized */
	if ((intp_src & HAL_RX_START_MASK))
	{
	} else if (intp_src & HAL_TRX_END_MASK) {
		U8 state = hal_subregister_read(SR_TRX_STATUS);

		if ((state == RX_ON) ||
		    (state == RX_AACK_ON) ||
		    (state == BUSY_RX_AACK))
		{
			BUF_ALLOC(buf, RX);
			hal_frame_read(buf);
			mac_queue_buf_insert(buf);
			drvr_set_data_rx_flag(true);
		} else {
			U8 status;

			while (drvr_set_trx_state(RX_AACK_ON) != RADIO_SUCCESS);

			status = hal_subregister_read(SR_TRAC_STATUS);

			switch (status)
			{
			case TRAC_SUCCESS:
			case TRAC_SUCCESS_DATA_PENDING:
				drvr_set_status(RADIO_SUCCESS);
				break;
			case TRAC_NO_ACK:
				drvr_set_status(RADIO_NO_ACK);
				break;
			case TRAC_CHANNEL_ACCESS_FAILURE:
				drvr_set_status(RADIO_CHANNEL_ACCESS_FAILURE);
				break;
			default:
				break;
			}
		}
	} else if (intp_src & HAL_TRX_UR_MASK) {
	} else if (intp_src & HAL_PLL_UNLOCK_MASK) {
	} else if (intp_src & HAL_PLL_LOCK_MASK) {
	} else if (intp_src & HAL_BAT_LOW_MASK)	{
	} else {
	}
}

/*
 * \brief Timer Overflow ISR
 * This is the interrupt service routine for timer1 overflow.
 */
ISR(TIMER1_OVF_vect)
{
}

