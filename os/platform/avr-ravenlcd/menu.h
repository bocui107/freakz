/*
 *  Copyright (c) 2008  Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      This file operates the menu flow chart described in the readme
 *      notes. This will create the proper commands needed to control the 1284p.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef MENU_H
#define MENU_H

#include <avr/pgmspace.h>
#include <stdbool.h>

/** \brief Pointer to function, used to dispatch a menu-driven function.  See tmenu_item */
typedef void (*tmenufunc)(uint8_t *);

/** \brief This structure defines the joystick operation within the menu_items[]. */
typedef struct {
    const char * PROGMEM text;  /**< Menu text to display. */
    uint8_t left;               /**< Contains next menu position for left. */
    uint8_t right;              /**< Contains next menu position for right. */
    uint8_t up;                 /**< Contains next menu position for up. */
    uint8_t down;               /**< Contains next menu position for down. */
    uint8_t *state;             /**< State variable used as argument for menu enter function. */
    tmenufunc enter_func;       /**< Pointer to function to call when enter button is pressed. */
} tmenu_item;

extern uint8_t ping_response;
extern bool ping_mode;
extern bool timeout_flag;
extern bool temp_flag;
extern bool auto_temp;
extern const PROGMEM tmenu_item menu_items[];

typedef enum
{
    SEND_TEMP               = 0x80,
    SEND_PING               = 0x81,
    SEND_START              = 0x82,
    SEND_JOIN               = 0x83,
    SEND_TX_DATA            = 0x84,
    SEND_SET_ADDR           = 0x85,

    REPORT_NWK_FORM_CONF    = 0xC0,
    REPORT_NWK_JOIN_CONF    = 0xC1,
    REPORT_TEXT_MSG         = 0xC2,
    REPORT_RX_DATA          = 0xC3
} uart_cmd_t;

typedef enum
{
    BUTTON_UP       = 0xA0,
    BUTTON_DOWN     = 0xA1,
    BUTTON_RIGHT    = 0xA2,
    BUTTON_LEFT     = 0xA3
} button_t;

#define EEPROM_DEBUG_ADDR   0

void menu_run_sleep(uint8_t *val);
void dectoascii(uint8_t val, char *str);
uint8_t *signed_dectoascii(int16_t n, uint8_t *str);
void eeprom_init(void);
void menu_debug_mode(uint8_t *val);
void menu_read_temp(uint8_t *val);
void menu_display_temp(void);
void menu_clear_temp(void);
void menu_prepare_temp(uint8_t *val);
void menu_stop_temp(void);
void menu_send_temp(void);
void menu_led_toggle(uint8_t *val);
void menu_send_start_coord(uint8_t *val);
void menu_send_start_router(uint8_t *val);
void menu_send_tx_data(uint8_t *val);
void menu_send_tx_data2(uint8_t *val);
void menu_set_ext_addr();

#endif /* MENU_H */
