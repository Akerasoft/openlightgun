/*  Extenmote : NES, SNES, N64 and Gamecube to Wii remote adapter firmware
 *  Copyright (C) 2012-2014  Raphael Assenat <raph@raphnet.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>

#include "classic.h"
#include "eeprom.h"
#include "analog.h"

#define C_DEFLECTION	100

/*       |                 Bit                                |
 * Byte  |   7   |   6   |   5   |  4  |  3 |  2 |  1  |  0   |
 * ------+---------------+-------+----------------------------+
 *  0    |    RX<4:3>    |            LX<5:0>                 |
 *  1    |    RX<2:1>    |           LY<5:0>                  |
 *  2    | RX<0> |    LT<4:3>    |      RY<4:0>               |
 *  3    |        LT<2:0>        |      RT<4:0>               |
 *  4    | BDR   |  BDD  |  BLT  |  B- | BH | B+  | BRT | 1   |
 *  5    | BZL   |   BB  |  BY   | BA  | BX | BZR | BDL | BDU |
 *
 *  6    | 0x52 ('R')                                         |
 *  7    | Controller ID byte 0                               |
 *  8    | Controller ID byte 1                               |
 *
 *  9    | Controller specific data 0                         |
 *  10   | Controller specific data 1                         |
 *  11   | Controller specific data 2                         |
 *  12   | Controller specific data 3                         |
 *  13   | Controller specific data 4                         |
 *  14   | Controller specific data 5                         |
 *  15   | Controller specific data 6                         |
 *  16   | Controller specific data 7                         |
 *
 *  Controller ID:
 *
 *   0   |  1
 *  -----+-----
 *   '6' | '4'    N64 controller
 *   'G' | 'C'    Gamecube controller
 *   'S' | 'F'    SNES
 *   'F' | 'C'    NES
 *   'G' | 'N'    NES Lightgun (Zapper)
 *
 * Writing at byte 6 might one day control the rumble motor. Rumbles on when non-zero.
 */
void pack_classic_data_mode1(classic_pad_data *src, unsigned char dst[PACKED_CLASSIC_DATA_SIZE], int analog_style)
{
	unsigned char rx,ry,lx=0x20,ly=0x20; // down sized
	unsigned char shoulder_left=0, shoulder_right=0; // lower 5 bits only

	memset(dst, 0x00, PACKED_CLASSIC_DATA_SIZE);

	lx = ((0x80 + src->lx) >> 2) & 0x3F;
	ly = ((0x80 + src->ly) >> 2) & 0x3F;

	rx = ((0x80 + src->rx) >> 3) & 0x1F;
	ry = ((0x80 + src->ry) >> 3) & 0x1F;

	shoulder_left = (src->lt >> 3);
	shoulder_right = (src->rt >> 3);

	dst[0] = ((rx<<3) & 0xC0) | lx;
	dst[1] = ((rx<<5) & 0xC0) | ly;
	dst[2] = rx<<7 | ry | (shoulder_left & 0x18) << 2;

	dst[3] = (shoulder_left << 5) | (shoulder_right & 0x1F);
	dst[4] = (src->buttons >> 8) ^ 0xFF;
	dst[5] = (src->buttons) ^ 0xFF;

	dst[6] = 'R';
	memcpy(dst+7, src->controller_id, 2);
	memcpy(dst+9, src->controller_raw_data, 8);
}

void pack_classic_data_mode2(classic_pad_data *src, unsigned char dst[PACKED_CLASSIC_DATA_SIZE], int analog_style)
{
	unsigned char rx=0x80,ry=0x80,lx=0x80,ly=0x80;
	unsigned char shoulder_left=0, shoulder_right=0;

	memset(dst, 0x00, PACKED_CLASSIC_DATA_SIZE);


	lx = 0x80 + src->lx;
	ly = 0x80 + src->ly;
	rx = 0x80 + src->rx;
	ry = 0x80 + src->ry;
	shoulder_left = src->lt;
	shoulder_right = src->rt;

	dst[0] = lx;
	dst[1] = rx;
	dst[2] = ly;
	dst[3] = ry;
	dst[4] = 0; // I think this can hold extra bits for the axes
	dst[5] = shoulder_left;
	dst[6] = shoulder_right;
	dst[7] = (src->buttons >> 8) ^ 0xFF;
	dst[8] = (src->buttons) ^ 0xFF;
}

void pack_classic_data_mode3(classic_pad_data *src, unsigned char dst[PACKED_CLASSIC_DATA_SIZE], int analog_style)
{
	unsigned char rx=0x80,ry=0x80,lx=0x80,ly=0x80;
	unsigned char shoulder_left=0, shoulder_right=0;

	memset(dst, 0x00, PACKED_CLASSIC_DATA_SIZE);

	lx = 0x80 + src->lx;
	ly = 0x80 + src->ly;
	rx = 0x80 + src->rx;
	ry = 0x80 + src->ry;

	shoulder_left = src->lt;
	shoulder_right = src->rt;

	dst[0] = lx;
	dst[1] = rx;
	dst[2] = ly;
	dst[3] = ry;
	dst[4] = shoulder_left;
	dst[5] = shoulder_right;
	dst[6] = (src->buttons >> 8) ^ 0xFF;
	dst[7] = (src->buttons) ^ 0xFF;
}


void pack_classic_data(classic_pad_data *src, unsigned char dst[PACKED_CLASSIC_DATA_SIZE], int analog_style, int mode)
{
	switch (mode)
	{
		default:
		case CLASSIC_MODE_1:
			pack_classic_data_mode1(src, dst, analog_style);
			break;
		case CLASSIC_MODE_2:
			pack_classic_data_mode2(src, dst, analog_style);
			break;
		case CLASSIC_MODE_3:
			pack_classic_data_mode3(src, dst, analog_style);
			break;
	}
}


void dataToClassic(const gamepad_data *src, classic_pad_data *dst, char first_read)
{
	memset(dst, 0, sizeof(classic_pad_data));

	// gun
	dst->controller_id[0] = 'G';
	dst->controller_id[1] = 'N';
	memcpy(dst->controller_raw_data, src->gun.raw_data, GUN_RAW_SIZE);

	if (src->gun.buttons & GUN_BTN_TRIGGER) { dst->buttons |= CPAD_BTN_A; }
	if (src->gun.buttons & GUN_BTN_SENSOR) { dst->buttons |= CPAD_BTN_B; }
}
