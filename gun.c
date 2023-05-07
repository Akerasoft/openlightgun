/* Nes/Snes to Wiimote
 * Original Copyright (C) 2012 Raphaël Assénat
 * Changes Copyright by (C) 2022 Akerasoft
 * Credits:
 * Robert Kolski is the programmer for Akerasoft.
 *
 * Based on earlier work:
 *
 * Nes/Snes/Genesis/SMS/Atari to USB
 * Copyright (C) 2006-2007 Raphaël Assénat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The original author may be contacted at raph@raphnet.net
 * The author for changes is robert.kolski@akerasoft.com
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "gamepads.h"
#include "gun.h"

#define GAMEPAD_BYTES	1

/******** IO port definitions **************/
#define GUN_8_BUTTONS_DDR  DDRD
#define GUN_8_BUTTONS_PORT PORTD
#define GUN_8_BUTTONS_PIN  PIND

/*********** prototypes *************/
static char gunInit(void);
static char gunUpdate(void);


// the most recent bytes we fetched from the controller
static unsigned char last_read_controller_bytes[GAMEPAD_BYTES];
// the most recently reported bytes
static unsigned char last_reported_controller_bytes[GAMEPAD_BYTES];

static char nes_mode = 0;

static char gunInit(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();

	// 8 NES buttons are input - all bits off
	GUN_8_BUTTONS_DDR = 0;

	// 8 NES buttons are normally high - all bits one
	GUN_8_BUTTONS_PORT = 0xFF;

	gunUpdate();

	SREG = sreg;

	return 0;
}


/*
 *
       Bit position     Button Reported           NES Button Reported        PIN
        ===========     ===============           =====================      ===================
        0               D4                        D4                         PD7
        1               D3                        D3                         PD6
        2               ?                         ?                          PD5
        3               ?                         ?                          PD4
        4               ?                         ?                          PD3
        5               ?                         ?                          PD2
        6               ?                         ?                          PD1
        7               ?                         ?                          PD0

 *
 */
 
static char gunUpdate(void)
{
	unsigned char tmp=0;

	tmp = ~GUN_8_BUTTONS_PIN;
	last_read_controller_bytes[0] = tmp & 0xC0;
	
	return 0;
}

static char gunChanged(void)
{
	return memcmp(last_read_controller_bytes,
					last_reported_controller_bytes, GAMEPAD_BYTES);
}

static void gunGetReport(gamepad_data *dst)
{
	unsigned char l;

	if (dst != NULL)
	{
		l = last_read_controller_bytes[0];

		// in this version we compile it as GUN
		nes_mode = 0;
		dst->gun.pad_type = PAD_TYPE_GUN;
		dst->gun.buttons = l;
		dst->gun.raw_data[0] = l;
	}
	memcpy(last_reported_controller_bytes,
			last_read_controller_bytes,
			GAMEPAD_BYTES);
}

static Gamepad GunGamepad = {
	.init		= gunInit,
	.update		= gunUpdate,
	.changed	= gunChanged,
	.getReport	= gunGetReport
};

Gamepad *gunGetGamepad(void)
{
	return &GunGamepad;
}
