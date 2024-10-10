/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2023 Serge Vakulenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "tusb.h"

#include <pico/unique_id.h>

extern "C" {

// String Descriptor Index
enum {
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIAL,
};

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0200,

	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,

	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

	// TODO: get unique VID/PID from https://pid.codes.
	.idVendor = 0x0000,  // TODO
	.idProduct = 0x0000, // TODO
	.bcdDevice = 0x0100,

	.iManufacturer = STRID_MANUFACTURER,
	.iProduct = STRID_PRODUCT,
	.iSerialNumber = STRID_SERIAL,

	.bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
	return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum { ITF_NUM_MSC, ITF_NUM_TOTAL };

#define EPNUM_MSC_OUT 0x01
#define EPNUM_MSC_IN 0x81

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

// full speed configuration
uint8_t const desc_fs_configuration[] = {
	// Config number, interface count, string index, total length, attribute,
	// power in mA
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

	// Interface number, string index, EP Out & EP In address, EP size
	TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
	(void)index; // for multiple configurations
	return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

extern char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

// array of pointer to string descriptors
char const *const string_desc_arr[] = {
	(const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
	"yocto-8",       // 1: Manufacturer
	"flash storage", // 2: Product
	serial,          // 3: Serials will use unique ID if possible
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	(void)langid;
	size_t chr_count;

	switch (index) {
	case STRID_LANGID:
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
		break;

	default:
		// Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

		if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
			return NULL;

		const char *str = string_desc_arr[index];

		// Cap at max char
		chr_count = strlen(str);
		size_t const max_count =
			sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
		if (chr_count > max_count)
			chr_count = max_count;

		// Convert ASCII string into UTF-16
		for (size_t i = 0; i < chr_count; i++) {
			_desc_str[1 + i] = str[i];
		}
		break;
	}

	// first byte is length (including header), second byte is string type
	_desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

	return _desc_str;
}
}
