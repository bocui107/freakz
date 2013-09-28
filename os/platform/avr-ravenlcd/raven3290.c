/*
 *     \mainpage ATmega3290p LCD Driver Software for Raven
*/
/**
 *     \image html raven.png
 *     \ingroup platform
 *     \defgroup lcdraven RZRAVEN LCD 3290p
 *
 *     \section intro_lcd LCD Introduction
 *
 *  This Raven LCD Driver application software was designed for a user interface
 *  to the Contiki 6LoWPAN collaboration on board the ATmega3290p. The
 *  LCD functionality uses the binary command set described in the release notes.
 *  These binary commands can also be found in a list of main.h.
 *
 *     \section compile_lcd Compiling Raven LCD Driver
 *
 *  Raven LCD Driver can be compiled on the following platforms:
 *
 *   -# <b>WinAvr + AVR Studio (AVR-GCC).</b>  The @b ravenlcd_3290.aps file is used by AVR
 *      Studio.  If you have WinAVR and AVR Studio installed, ravenlcd can be
 *      compiled and developed using free tools.  The Makefile.avr-ravenlcd is not
 *      used with AVR Studio.
 *   -# <b>AVR-GCC on Linux.</b>  The <b>Makefile.avr-ravenlcd</b> file is to be used for Linux.
 *      Be sure you have a recent toolchain installed, including patches
 *      required for GCC/avr-libc to recognize new devices.  The avr-libc
 *      webpage includes a concise guide on how to patch and build the AVR
 *      toolchain.
 *
 *     \section fuses_lcd Board fuse settings
 *
 *  The Raven LCD (3290p device) requires the proper fuse settings to function
 *  properly. These settings have been summarized below:
 *   -# Raven LCD (3290p device)
 *      -# Extended: <b>0xFF</b> (No Brown Out)
 *      -# High: <b>0x99</b> (JTAG and ISP enabled, No OCDEN or EEPROM saving required)
 *      -# Low: <b>0xE2</b> (Use Int RC OSC - Start-up Time:6CK + 65ms)
 *
 *     \section notes_lcd Operation Release Notes
 *
 *  After programming the Raven LCD 3290p with the proper image, you will be introduced to
 *  the menu in the picture below:
 *
 *     \image html contiki_menu_3290.jpg
 *
 *  Operating the menu requires that the matching command set has been programmed into
 *  the ATmega1284 application. This will allow the menu to communicate properly and control the
 *  Contiki 6LoWPAN applcation.
 *
 *  During normal operation, you will need to make note of these <b>IMPORTANT</b> operating instructions:
 *   -# <b>Temp Sensor</b> - The temperature sensor shares the same GPIO as the JTAG interface for the 3290p.
 *   This requires the JTAG feature to be <b>disabled</b> in order to have proper temperature readings.
 *   -# <b>Debug Menu</b> - The DEBUG menu setting is used to configure this JTAG feature.
 *      -# If the JTAG feature is enabled during a temperature reading attempt,
 *      the menu will signal a <b>caution</b> symbol to the user letting them know the JTAG
 *      feature needs to be disabled.
 *     \image html caution.gif
 *   -# <b>Temp Data</b> - Once the temperature reading is proper, the user can send this reading
 *   to the webserver for Sensor Reading Data (<b>Once</b> or <b>Auto</b>). The webserver will
 *   only update the html data when <b>refreshed</b>.
 *
 *   More information about the operation of the Contiki 6LoWPAN system can be found
 *   at the \ref tutorialraven.
 *
 *   More information about the 802.15.4 MAC designed for the Contiki 6LoWPAN system
 *   can be found at the \ref macdoc.
 *
 *     \section binary_lcd Binary Command Description
 *
 *  Using the binary commmand list described in main.h, the 3290p will contruct a binary
 *  command serial frame to control the 1284p. An example command frame is contructed below:
 *   -# <b>0x01,0x01,0x81,0x01,0x04</b> - Send Ping Request number 1 to 1284p
 *       -# <b>0x01</b> - Start of binary command frame
 *       -# <b>0x01</b> - Length of binary command payload
 *       -# <b>0x81</b> - Binary command SEND_PING
 *       -# <b>0x01</b> - Payload value (eg. ping Request number 1)
 *       -# <b>0x04</b> - End of binary command frame
 *
 *  The following commands are used to control the 1284p.
 *   -# <b>SEND_TEMP - (0x80)</b>
 *   -# <b>SEND_PING - (0x81)</b>
 *
 *  The following commands are used to update the 3290p.
 *   -# <b>REPORT_PING - (0xC0)</b>
 *   -# <b>REPORT_PING_BEEP - (0xC1)</b>
 *   -# <b>REPORT_TEXT_MSG - (0xC2)</b>
*/
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
 *      This is the main file for the Raven LCD application.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#include "lcd.h"
#include "key.h"
#include "main.h"
#include "uart.h"
#include "timer.h"
#include "menu.h"
#include "temp.h"

#include <avr/io.h>
#include <avr/fuse.h>
FUSES =
	{
		.low = 0xe2,
		.high = 0x99,
		.extended = 0xff,
	};


/** \ingroup lcdraven
    \defgroup lcd LCD Functions and data
 *  \{
*/

#if defined( DOXYGEN )
const char menu_text0[];
const char menu_text1[];
const char menu_text2[];
const char menu_text3[];
const char menu_text4[];
//const char menu_text5[];
//const char menu_text6[];
//const char menu_text7[];
//const char menu_text8[];
//const char menu_text9[];
//const char menu_text10[];
//const char menu_text11[];
//const char menu_text12[];
//const char menu_text13[];
//const char menu_text14[];
//const char menu_text15[];
//const tmenu_item menu_items[16];
#else  /* !DOXYGEN */
/** \brief This is the menu text in Flash. See menu_items[] for menu operation. */
const char menu_text0[] PROGMEM =  "FREAKZ";
const char menu_text1[] PROGMEM =  "START";
const char menu_text2[] PROGMEM =  "COORD";
const char menu_text3[] PROGMEM =  "ROUTER";
const char menu_text4[] PROGMEM =  "TXDATA1";
const char menu_text5[] PROGMEM =  "SEND";
const char menu_text6[] PROGMEM =  "TXDATA2";
const char menu_text7[] PROGMEM =  "SEND2";
const char menu_text8[] PROGMEM =  "EXTADDR";
//const char menu_text9[] PROGMEM =  "ONCE";
//const char menu_text10[] PROGMEM = "AUTO";
//const char menu_text11[] PROGMEM = "DEBUG";
//const char menu_text12[] PROGMEM = "DBG ON";
//const char menu_text13[] PROGMEM = "DBG OFF";
//const char menu_text14[] PROGMEM = "SENT";
//const char menu_text15[] PROGMEM = "SENDING";

/*---------------------------------------------------------------------------*/

/**
 *   \brief Menus for user interface
 *
 *   This constructs the Raven 3290p menu for the Contiki and 6lowpan collaboration. This
 *   follows the struct description of tmenu_item.
 *
*    { text,        left,   right,  up,     down,   state,          tmenufunc}*
 *
*/
const PROGMEM tmenu_item menu_items[16]  = {
    {menu_text0,    0,      0,      4,      1,      0,              0                       },
    {menu_text1,    0,      2,      0,      4,      0,              0                       },
    {menu_text2,    1,      2,      3,      3,      0,              menu_send_start_coord   },
    {menu_text3,    1,      3,      2,      2,      0,              menu_send_start_router  },
    {menu_text4,    4,      5,      1,      6,      0,              0                       },
    {menu_text5,    4,      4,      5,      5,      0,              menu_send_tx_data       },
    {menu_text6,    6,      7,      4,      8,      0,              0                       },
    {menu_text7,    6,      6,      7,      7,      0,              menu_send_tx_data2      },
    {menu_text8,    8,      8,      6,      0,      0,              menu_set_ext_addr       },
//    {menu_text9,    0,      0,      0,     10,      0,              0                  },
//    {menu_text10,   0,      0,      0,     11,      0,              0                  },
//    {menu_text11,   0,      0,      0,     12,      0,              0                  },
//    {menu_text12,   0,      0,      0,     13,      0,              0                  },
//    {menu_text13,   0,      0,      0,     14,      0,              0                  },
//    {menu_text14,   0,      0,      0,     15,      0,              0                  },
//    {menu_text15,   0,      0,      0,     0,       0,              0                  },

//    {menu_text6,    5,      5,      7,      7,      (uint8_t*)1,    menu_read_temp     },
//    {menu_text7,    5,      5,      6,      6,      (uint8_t*)0,    menu_read_temp     },
//    {menu_text8,    4,      9,      5,      5,      0,              0                  },
//    {menu_text9,    8,      14,     10,     10,     (uint8_t*)0,    menu_prepare_temp  },
//    {menu_text10,   8,      15,     9,      9,      (uint8_t*)1,    menu_prepare_temp  },
//    {menu_text11,   0,      12,     4,      2,      0,              0                  },
//    {menu_text12,   11,     11,     13,     13,     (uint8_t*)1,    menu_debug_mode    },
//    {menu_text13,   11,     11,     12,     12,     (uint8_t*)0,    menu_debug_mode    },
//    {menu_text14,   9,      14,     14,     14,     0,              0                  },
//    {menu_text15,   10,     15,     15,     15,     0,              0                  },
};
#endif /* !DOXYGEN */

key_state_t button=KEY_STATE_NO_KEY;
tmenu_item menu;
uint8_t count;
uint8_t timeout_count;

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will read the menu_items[] from the index requested.
 *
 *   \param ndx Position index of menu_items[] lookup.
*/
void
read_menu(uint8_t ndx)
{
    /* Reads menu structure from Flash area */
    uint8_t i;
    uint8_t *src = (uint8_t*)&menu_items[ndx];
    uint8_t *dest = (uint8_t*)&menu;

    for (i=0;i<sizeof(tmenu_item);i++){
        *dest++ = pgm_read_byte(src+i);
    }
}

/*---------------------------------------------------------------------------*/
/**
 *   \brief This will check for the temp menu screen to determine if we need to
 *   clear the 4 digit LCD segments or stop an auto temp send. Also check for
 *   stopping a ping request.
*/
void
check_menu(void)
{
    //if(menu.text == menu_text4){
    //    menu_clear_temp();
    //}
    //
    //if(menu.text == menu_text10){
    //    menu_stop_temp();
    //}
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This is main...
*/
int
main(void)
{
    lcd_init();

    key_init();

    uart_init();

    eeprom_init();

    temp_init();

    timer_init();

    sei();

    lcd_symbol_set(LCD_SYMBOL_RAVEN);
    lcd_symbol_set(LCD_SYMBOL_ZIGBEE);

    /* Start with main menu */
    read_menu(0);

    /* and draw it */
    lcd_puts_P(menu.text);

    timer_start();

    for (;;){
        /* Make sure interrupts are always on */
        sei();

        /* The one second timer has fired. */
        if(timer_flag)
        {
            timer_flag = false;

            ///* Update LCD with temp data. */
            //if(temp_flag)
            //{
            //    menu_display_temp();
            //}
            //
            ///* Auto send temp data to 1284p. */
            //if(auto_temp){
            //    menu_send_temp();
            //}
        }

        /* Check for button press and deal with it */
        if (is_button())
        {
            /* Dispatch the button pressed */
            switch (get_button())
            {
                case KEY_UP:
                    read_menu(menu.up);
                    lcd_puts_P(menu.text);
                    break;
                case KEY_DOWN:
                    read_menu(menu.down);
                    lcd_puts_P(menu.text);
                    break;
                case KEY_LEFT:
                    read_menu(menu.left);
                    lcd_puts_P(menu.text);
                    break;
                case KEY_RIGHT:
                    /*
                     * Check to see if we should show another menu or
                     * run a function
                     */
                    if (!menu.enter_func){
                        /* Just another menu to display */
                        read_menu(menu.right);
                        lcd_puts_P(menu.text);
                        break;
                    }
                    /* Drop through here */
                case KEY_ENTER:
                    /* Call the menu function on right or enter buttons */
                    if (menu.enter_func){
                        menu.enter_func(menu.state);
                        if (menu.state){
                            /*
                             * We just called a selection menu (not a test),
                             * so re-display the text for this menu level
                             */
                            lcd_puts_P(menu.text);
                        }
                        /* After enter key, check the right button menu and display. */
                        read_menu(menu.right);
                        lcd_puts_P(menu.text);
                    }
                    break;
                default:
                    break;
            }
            /* After button press, check for menus... */
            check_menu();
        }
        /* Process any progress frames */
        uart_serial_rcv_frame(false);
    } /* end for(). */
} /* end main(). */

/** \} */
