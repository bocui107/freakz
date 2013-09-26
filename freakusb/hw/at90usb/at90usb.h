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
    \file at90usb.h
    \ingroup hw_at90usb
*/
/*******************************************************************/
#ifndef AT90USB_H
#define AT90USB_H

// banks
#define SINGLE          0
#define DUAL            1

#define PLL_LOCKED          (PLLCSR & (1 << PLOCK))
#define FIFO_BYTE_CNT       UEBCX

#define TX_DATA()           (UEINTX &= ~TX_IN_INT_MASK)
#define TX_FIFO_READY       (UEINTX &   TX_IN_INT_MASK)
#define SET_DEVICE_MODE()   (UHWCON |= (1<<UIMOD))

#define DEV_INT_MASK(intp)  (1 << intp)
#define VBUS_INT_MASK       DEV_INT_MASK(VBUSTI)
#define SUSP_INT_MASK       DEV_INT_MASK(SUSPI)
#define RESM_INT_MASK       DEV_INT_MASK(EORSMI)
#define EOR_INT_MASK        DEV_INT_MASK(EORSTI)
#define WAKEUP_INT_MASK     DEV_INT_MASK(WAKEUPI)

#define DEV_INT_ENB_MASK(intp) (1 << intp)
#define VBUS_INT_ENB_MASK   DEV_INT_ENB_MASK(VBUSTE)
#define SUSP_INT_ENB_MASK   DEV_INT_ENB_MASK(SUSPE)
#define RESM_INT_ENB_MASK   DEV_INT_ENB_MASK(EORSME)
#define EOR_INT_ENB_MASK    DEV_INT_ENB_MASK(EORSTE)
#define WAKEUP_INT_ENB_MASK DEV_INT_ENB_MASK(WAKEUPE)

// global intps
#define VBUS_INT            (USBINT & VBUS_INT_MASK)
#define VBUS_INT_ENABLED    (USBCON & VBUS_INT_ENB_MASK)
#define VBUS_INT_CLR()      (USBINT &= ~VBUS_INT_MASK)
#define VBUS_INT_ENB()      (USBCON |= VBUS_INT_ENB_MASK)
#define VBUS_INT_DIS()      (USBCON &= ~VBUS_INT_ENB_MASK)

// device intps
#define DEV_INT(mask)       (UDINT & mask)
#define SUSP_INT            DEV_INT(SUSP_INT_MASK)
#define RESM_INT            DEV_INT(RESM_INT_MASK)
#define EOR_INT             DEV_INT(EOR_INT_MASK)
#define WAKEUP_INT          DEV_INT(WAKEUP_INT_MASK)

#define DEV_INT_ENABLED(mask)       (UDIEN & mask)
#define SUSP_INT_ENABLED    DEV_INT_ENABLED(SUSP_INT_ENB_MASK)
#define RESM_INT_ENABLED    DEV_INT_ENABLED(RESM_INT_ENB_MASK)
#define EOR_INT_ENABLED     DEV_INT_ENABLED(EOR_INT_ENB_MASK)
#define WAKEUP_INT_ENABLED  DEV_INT_ENABLED(WAKEUP_INT_ENB_MASK)

#define DEV_INT_CLR(mask)   (UDINT &= ~mask)
#define SUSP_INT_CLR()      DEV_INT_CLR(SUSP_INT_MASK)
#define RESM_INT_CLR()      DEV_INT_CLR(RESM_INT_MASK)
#define EOR_INT_CLR()       DEV_INT_CLR(EOR_INT_MASK)
#define WAKEUP_INT_CLR()    DEV_INT_CLR(WAKEUP_INT_MASK)

#define DEV_INT_ENB(mask)   (UDIEN |= mask)
#define SUSP_INT_ENB()      DEV_INT_ENB(SUSP_INT_ENB_MASK)
#define RESM_INT_ENB()      DEV_INT_ENB(RESM_INT_ENB_MASK)
#define EOR_INT_ENB()       DEV_INT_ENB(EOR_INT_ENB_MASK)
#define WAKEUP_INT_ENB()    DEV_INT_ENB(WAKEUP_INT_ENB_MASK)

#define DEV_INT_DIS(mask)  (UDIEN &= ~mask)
#define SUSP_INT_DIS()      DEV_INT_DIS(SUSP_INT_ENB_MASK)
#define RESM_INT_DIS()      DEV_INT_DIS(RESM_INT_ENB_MASK)
#define EOR_INT_DIS()       DEV_INT_DIS(EOR_INT_ENB_MASK)
#define WAKEUP_INT_DIS()    DEV_INT_DIS(WAKEUP_INT_ENB_MASK)

// endpoint intps
#define EP_INT_MASK(intp)   (1 << intp)
#define TX_IN_INT_MASK      EP_INT_MASK(TXINI)
#define STALL_INT_MASK      EP_INT_MASK(STALLEDI)
#define FIFOCON_INT_MASK    EP_INT_MASK(FIFOCON)
#define NAK_IN_INT_MASK     EP_INT_MASK(NAKINI)
#define RWAL_INT_MASK       EP_INT_MASK(RWAL)
#define NAK_OUT_INT_MASK    EP_INT_MASK(NAKOUTI)
#define RX_SETUP_INT_MASK   EP_INT_MASK(RXSTPI)
#define RX_OUT_INT_MASK     EP_INT_MASK(RXOUTI)

#define EP_INT_ENB_MASK(intp)   (1 << intp)
#define TX_IN_INT_ENB_MASK      EP_INT_ENB_MASK(TXINE)
#define STALL_INT_ENB_MASK      EP_INT_ENB_MASK(STALLEDE)
#define FLOW_ERR_INT_ENB_MASK   EP_INT_ENB_MASK(FLERRE)
#define NAK_IN_INT_ENB_MASK     EP_INT_ENB_MASK(NAKINE)
#define NAK_OUT_INT_ENB_MASK    EP_INT_ENB_MASK(NAKOUTE)
#define RX_SETUP_INT_ENB_MASK   EP_INT_ENB_MASK(RXSTPE)
#define RX_OUT_INT_ENB_MASK     EP_INT_ENB_MASK(RXOUTE)

#define EP_INT(mask)        (UEINTX & mask)
#define TX_IN_INT           EP_INT(TX_IN_INT_MASK)
#define STALL_INT           EP_INT(STALL_INT_MASK)
#define FIFOCON_INT         EP_INT(FIFOCON_INT_MASK)
#define NAK_IN_INT          EP_INT(NAK_IN_INT_MASK)
#define RWAL_INT            EP_INT(RWAL_INT_MASK)
#define NAK_OUT_INT         EP_INT(NAK_OUT_INT_MASK)
#define RX_SETUP_INT        EP_INT(RX_SETUP_INT_MASK)
#define RX_OUT_INT          EP_INT(RX_OUT_INT_MASK)

#define EP_INT_ENABLED(mask)    (UEIENX & mask)
#define TX_IN_INT_ENABLED       EP_INT_ENABLED(TX_IN_INT_ENB_MASK)
#define STALL_INT_ENABLED       EP_INT_ENABLED(STALL_INT_ENB_MASK)
#define FLOW_ERR_INT_ENABLED    EP_INT_ENABLED(FLOW_ERR_INT_ENB_MASK)
#define NAK_IN_INT_ENABLED      EP_INT_ENABLED(NAK_IN_INT_ENB_MASK)
#define NAK_OUT_INT_ENABLED     EP_INT_ENABLED(NAK_OUT_INT_ENB_MASK)
#define RX_SETUP_INT_ENABLED    EP_INT_ENABLED(RX_SETUP_INT_ENB_MASK)
#define RX_OUT_INT_ENABLED      EP_INT_ENABLED(RX_OUT_INT_ENB_MASK)

#define EP_INT_CLR(mask)    (UEINTX &= ~mask)
#define TX_IN_INT_CLR()     EP_INT_CLR(TX_IN_INT_MASK)
#define STALL_INT_CLR()     EP_INT_CLR(STALL_INT_MASK)
#define FIFOCON_INT_CLR()   EP_INT_CLR(FIFOCON_INT_MASK)
#define NAK_IN_INT_CLR()    EP_INT_CLR(NAK_IN_INT_MASK)
#define RWAL_INT_CLR()      EP_INT_CLR(RWAL_INT_MASK)
#define NAK_OUT_INT_CLR()   EP_INT_CLR(NAK_OUT_INT_MASK)
#define RX_SETUP_INT_CLR()  EP_INT_CLR(RX_SETUP_INT_MASK)
#define RX_OUT_INT_CLR()    EP_INT_CLR(RX_OUT_INT_MASK)

#define EP_INT_ENB(mask)    (UEIENX |= mask)
#define TX_IN_INT_ENB()     EP_INT_ENB(TX_IN_INT_ENB_MASK)
#define STALL_INT_ENB()     EP_INT_ENB(STALL_INT_ENB_MASK)
#define FLOW_ERR_INT_ENB()  EP_INT_ENB(FLOW_ERR_INT_ENB_MASK)
#define NAK_IN_INT_ENB()    EP_INT_ENB(NAK_IN_INT_ENB_MASK)
#define NAK_OUT_INT_ENB()   EP_INT_ENB(NAK_OUT_INT_ENB_MASK)
#define RX_SETUP_INT_ENB()  EP_INT_ENB(RX_SETUP_INT_ENB_MASK)
#define RX_OUT_INT_ENB()    EP_INT_ENB(RX_OUT_INT_ENB_MASK)

#define EP_INT_DIS(mask)    (UEIENX &= ~mask)
#define TX_IN_INT_DIS()     EP_INT_DIS(TX_IN_INT_ENB_MASK)
#define STALL_INT_DIS()     EP_INT_DIS(STALL_INT_ENB_MASK)
#define FLOW_ERR_INT_DIS()  EP_INT_DIS(FLOW_ERR_INT_ENB_MASK)
#define NAK_IN_INT_DIS()    EP_INT_DIS(NAK_IN_INT_ENB_MASK)
#define NAK_OUT_INT_DIS()   EP_INT_DIS(NAK_OUT_INT_ENB_MASK)
#define RX_SETUP_INT_DIS()  EP_INT_DIS(RX_SETUP_INT_ENB_MASK)
#define RX_OUT_INT_DIS()    EP_INT_DIS(RX_OUT_INT_ENB_MASK)

/** LED mask for the first LED on the board. */
#define LED1    (1 << 4)
#define LED2    (1 << 5)
#define LED3    (1 << 7)
#define LED4    (1 << 6)

#define JOY_BMASK  ((1 << 5) | (1 << 6) | (1 << 7))
#define JOY_EMASK  ((1 << 4) | (1 << 5))

#endif

