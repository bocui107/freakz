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
    \file desc.c
    \ingroup cdc_class
*/
/*******************************************************************/
#include "freakusb.h"
#include <avr/pgmspace.h>

U8 dev_desc[] PROGMEM =
{
	0x12,       // bLength
	DEV_DESCR,  // bDescriptorType
	0x00,
	0x02,       // bcdUSB: 0200 (2.0)
	0x02,       // bDeviceClass: CDC
	0x00,       // bDeviceSubClass: Get from cfg descr
	0x00,       // bDeviceProtocol: Get from cfg descr
	MAX_BUF_SZ, // bMaxPacketSize: MAX_BUF_SZ
	0xEB,
	0x03,       // idVendor: 0x03EB (use Atmel's VID for now)
	0x18,
	0x20,       // idProduct: 0x2018 (use Atmel's PID for now)
	0x00,
	0x01,       // bcdDevice: 0100 (v1.00)
	1,          // Index of string descriptor describing manufacturer
	2,          // Index of string descriptor describing product
	3,          // Index of string descriptor describing the device's serial number
	0x01        // bNumConfigurations
};

/* cfg descriptor */
U8 cfg_desc[] PROGMEM =
{
	/* cfg descr */
	0x09,           // bLength: cfg desc len
	CFG_DESCR,      // bDescriptorType: Configuration
	0x43,           // wTotalLength: 0x43 (67 bytes = total size of cfg + sub descriptors)
	0x00,
	0x02,           // bNumInterfaces: 2 interface
	0x01,           // bConfigurationValue: Configuration value
	0x00,           // iConfiguration: Index of string descriptor describing the configuration
	0xA0,           // bmAttributes: bus powered
	0x32,           // MaxPower 100 mA

	/* intf descr */
	0x09,           // bLength: Interface Descriptor size
	INTF_DESCR,     // bDescriptorType: Interface
	0x00,           // bInterfaceNumber: Number of Interface
	0x00,           // bAlternateSetting: Alternate setting
	0x01,           // bNumEndpoints: One endpoints used
	0x02,           // bInterfaceClass: Communication Interface Class
	0x02,           // bInterfaceSubClass: Abstract Control Model
	0x01,           // bInterfaceProtocol: Common AT commands
	0x00,           // iInterface:

	/* hdr functional descr */
	0x05,           // bLength: Endpoint Descriptor size
	0x24,           // bDescriptorType: CS_INTERFACE
	0x00,           // bDescriptorSubtype: Header Func Desc
	0x10,           // bcdCDC: spec release number 0110 (1.1)
	0x01,

	// call mgmt functional descriptors
	0x05,           // bFunctionLength
	0x24,           // bDescriptorType: CS_INTERFACE
	0x01,           // bDescriptorSubtype: Call Management Func Desc
	0x00,           // bmCapabilities: D0+D1
	0x01,           // bDataInterface: 1

	// acm functional descr
	0x04,           // bFunctionLength
	0x24,           // bDescriptorType: CS_INTERFACE
	0x02,           // bDescriptorSubtype: Abstract Control Management desc
	0x02,           // bmCapabilities

	// union functional descr
	0x05,           // bFunctionLength
	0x24,           // bDescriptorType: CS_INTERFACE
	0x06,           // bDescriptorSubtype: Union func desc
	0x00,           // bMasterInterface: Communication class interface
	0x01,           // bSlaveInterface0: Data Class Interface

	// ep 2 descr
	0x07,           // bLength: Endpoint Descriptor size
	EP_DESCR,       // bDescriptorType: Endpoint
	0x82,           // bEndpointAddress: (IN2)
	0x03,           // bmAttributes: Interrupt
	0x08,           // wMaxPacketSize:
	0x00,
	0xff,           // bInterval: 255 msec
			// not really using this endpoint for anything so just set
			// it to a fucking long interval

	// data class intf descr
	0x09,           // bLength: Interface Descriptor size
	INTF_DESCR,     // bDescriptorType: interface descriptor
	0x01,           // bInterfaceNumber: Number of Interface
	0x00,           // bAlternateSetting: Alternate setting
	0x02,           // bNumEndpoints: Two endpoints used
	0x0A,           // bInterfaceClass: CDC
	0x00,           // bInterfaceSubClass:
	0x00,           // bInterfaceProtocol:
	0x00,           // iInterface:

	// ep 3 descr
	0x07,           // bLength: Endpoint Descriptor size
	EP_DESCR,       // bDescriptorType: Endpoint
	0x03,           // bEndpointAddress: (OUT3)
	0x02,           // bmAttributes: Bulk
	MAX_BUF_SZ,     // wMaxPacketSize: MAX_BUF_SZ
	0x00,
	0x00,           // bInterval: ignore for Bulk transfer

	// ep 1 descr
	0x07,           // bLength: Endpoint Descriptor size
	EP_DESCR,       // bDescriptorType: Endpoint
	0x81,           // bEndpointAddress: (IN1)
	0x02,           // bmAttributes: Bulk
	MAX_BUF_SZ,     // wMaxPacketSize: MAX_BUF_SZ
	0x00,
	0x00            // bInterval: ignore for Bulk transfer
};

U8 dev_qualifier_desc[] PROGMEM =
{
	0x0A,           // bLength
	0x06,           // bDescriptorType: Device Qualifier
	0x00,
	0x02,           // bcdUSB Spec Version: 0200 (2.0)
	0x02,           // bDeviceClass: CDC
	0x00,           // bDeviceSubClass: Get from cfg descr
	0x00,           // bDeviceProtocol: Get from cfg descr
	MAX_BUF_SZ,     // bMaxPacketSize: MAX_BUF_SZ
	0x01,           // bNumConfigurations: 1
	0x00            // bReserved: Don't touch this or the device explodes
};

U8 lang_str_desc[] PROGMEM =
{
	0x4,        // bLength
	STR_DESCR,  // bDescriptorType: String
		    // Language: English
	0x09, 0x04
};

U8 vendor_str_desc[] PROGMEM =
{
	0x14,       // bLength
	STR_DESCR,  // bDescriptorType: String
		    // Manufacturer: "FreakLabs"
	'F',0, 'r',0, 'e',0, 'a',0, 'k',0, 'L',0, 'a',0, 'b',0,
	's',0
};

U8 prod_str_desc[] PROGMEM =
{
	0x4A,      // bLength: 0x3E (62 bytes = sizeof str fields + string)
	STR_DESCR, // bDescriptorType: String
		   // Product name: "FreakLabs RP AVRUSB Virtual COM Port"

	'F',0, 'r',0, 'e',0, 'a',0, 'k',0, 'L',0, 'a',0, 'b',0,
	's',0, ' ',0, 'R',0, 'P',0, ' ',0, 'A',0, 'V',0, 'R',0,
	'U',0, 'S',0, 'B',0, ' ',0, 'V',0, 'i',0, 'r',0, 't',0,
	'u',0, 'a',0, 'l',0, ' ',0, 'C',0, 'O',0, 'M',0, ' ',0,
	'P',0, 'o',0, 'r',0, 't',0,
};

U8 serial_str_desc[] PROGMEM =
{
	0x14,       // bLength: 0x14 (20 bytes = sizeof str fields + string)
	STR_DESCR,  // bDescriptorType: String
		    // Serial: Beta 0.50
	'B',0, 'e',0, 't',0, 'a',0, ' ',0, '0',0, '.',0, '5',0,
	'0',0,
};

/* Return a pointer to the device descriptor. */
U8 *desc_dev_get()
{
	return dev_desc;
}

/*
 * Return the length of the device descriptor. The length is stored in the
 * first byte of the device descriptor.
 */
U8 desc_dev_get_len()
{
	return pgm_read_byte(dev_desc);
}

/*
 * Return a pointer to the configuration descriptor.
 */
U8 *desc_cfg_get()
{
	return cfg_desc;
}

/*
 * Return the length of the configuration descriptor. The length of the complete
 * configuration descriptor is stored in the third byte of the config
 * descriptor.
 */
U8 desc_cfg_get_len()
{
	return pgm_read_byte(cfg_desc + 2);
}

/* Return a pointer to the device qualifier. */
U8 *desc_dev_qual_get()
{
	return dev_qualifier_desc;
}

/*
 * Return the length of the device qualifier. The length is stored in the
 * first byte of the qualifier.
 */
U8 desc_dev_qual_get_len()
{
	return pgm_read_byte(dev_qualifier_desc);
}

/* Return a pointer to the specified string descriptor. */
U8 *desc_str_get(U8 index)
{
	switch (index) {
	case LANG_DESC_IDX:
		return (U8 *)lang_str_desc;
	case MANUF_DESC_IDX:
		return (U8 *)vendor_str_desc;
	case PROD_DESC_IDX:
		return (U8 *)prod_str_desc;
	case SERIAL_DESC_IDX:
		return (U8 *)serial_str_desc;
	default:
		return NULL;
	}
}

/* Return the length of the specified string descriptor. */
U8 desc_str_get_len(U8 index)
{
	switch (index) {
	case LANG_DESC_IDX:
		return pgm_read_byte(lang_str_desc);
	case MANUF_DESC_IDX:
		return pgm_read_byte(vendor_str_desc);
	case PROD_DESC_IDX:
		return pgm_read_byte(prod_str_desc);
	case SERIAL_DESC_IDX:
		return pgm_read_byte(serial_str_desc);
	default:
		return 0;
	}
}
