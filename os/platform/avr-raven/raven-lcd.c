/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
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
 * $Id: raven-lcd.c,v 1.1 2008/10/14 09:45:17 julienabeille Exp $
*/

/**
 *  \brief This module contains code to interface a Contiki-based
 *  project on the AVR Raven platform's ATMega1284P chip to the LCD
 *  driver chip (ATMega3290P) on the Raven.
 *
 *  \author Blake Leverett <bleverett@gmail.com>
 *
*/

/**  \addtogroup raven
 * @{
 */

/**
 *  \defgroup ravenserial Serial interface between Raven processors
 * @{
 */
/**
 *  \file
 *  This file contains code to connect the two AVR Raven processors via a serial connection.
 *
 */



#include "contiki.h"
#include "raven-lcd.h"

#if (TEST_RAVEN == 1)
#   include "test_avr_raven.h"
#else
#   if (TEST_ZCL == 1)
#       include "test_zcl.h"
#   endif
#endif


#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define cmd_len 8
static struct{
    u8_t frame[cmd_len];
    u8_t ndx;
    u8_t len;
    u8_t cmd;
    u8_t done;
} cmd;

void rs232_send(uint8_t port, unsigned char c);
static process_event_t event_ser_rx;

/*---------------------------------------------------------------------------*/
/* Send a serial command frame to the ATMega3290 Processsor on Raven via serial port */
void send_frame(uint8_t cmd, uint8_t len, uint8_t *payload)
{
    uint8_t i;

    rs232_send(0, SOF_CHAR);    /* Start of Frame */
    rs232_send(0, len);
    rs232_send(0, cmd);
    for (i=0;i<len;i++)
        rs232_send(0,*payload++);
    rs232_send(0, EOF_CHAR);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void raven_lcd_init()
{
    event_ser_rx = process_alloc_event();
}

#if (TEST_RAVEN == 1)

/*---------------------------------------------------------------------------*/
static void raven_lcd_cmd_handler()
{
    if (cmd.done)
    {
        /* Execute the waiting command */
        switch (cmd.cmd)
        {
        case CMD_TEMP:
            break;
        case CMD_START:
            test_raven_zdo_start(cmd.frame[0]);
            break;
        case CMD_TX_DATA:
            test_raven_tx_data(cmd.len, cmd.frame);
            break;
        case CMD_SET_ADDR:
            test_raven_set_addr();
            break;
        default:
            break;
        }
        /* Reset command done flag. */
        cmd.done = false;
    }
}
#else
static void raven_lcd_cmd_handler()
{
}
#endif
/*---------------------------------------------------------------------------*/
/* Process an input character from serial port.
 *  ** This is called from an ISR!!
*/
int raven_lcd_serial_input(unsigned char ch)
{
    /* Parse frame,  */
    switch (cmd.ndx){
    case 0:
        /* first byte, must be 0x01 */
        cmd.done = false;
        if (ch != 0x01){
            return 0;
        }
        break;
    case 1:
        /* Second byte, length of payload */
        cmd.len = ch;
        break;
    case 2:
        /* Third byte, command byte */
        cmd.cmd = ch;
        break;
    default:
        /* Payload and ETX */
        if (cmd.ndx >= cmd.len+3){
            /* all done, check ETX */
            if (ch == 0x04){
                cmd.done = true;
                process_post(&raven_lcd_process, event_ser_rx, NULL);
            } else {
                /* Failed ETX */
                cmd.ndx = 0;
                return 0;
            }
        } else {
            /* Just grab and store payload */
            cmd.frame[cmd.ndx - 3] = ch;
        }
        break;
    }

    cmd.ndx++;

    return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS(raven_lcd_process, "Raven LCD interface process");
PROCESS_THREAD(raven_lcd_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
      PROCESS_WAIT_EVENT();
      raven_lcd_cmd_handler();
	  cmd.ndx = 0;
  }
  PROCESS_END();
}
/** @} */
