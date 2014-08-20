/*
 * iB Slave
 * 1-wire slave implementation emulating ID-only iButton (1990A/R)
 *
 * Copyright (C) 2014 Michal Belica <devel@beli.sk>
 *
 * This file is part of iB Slave.
 *
 * iB Slave is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iB Slave is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iB Slave.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _CONFIG_H
#define _CONFIG_H

#define F_CPU 8000000UL

#define OW_ID {0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6, 0x07, 0x18}
#define OW_PORT PORTB
#define OW_DDR DDRB
#define OW_PIN PINB
#define OW_P PB3

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_P PB4
#define LED_INIT LED_PORT &= ~_BV(LED_P); LED_DDR |= _BV(LED_P);
#define LED_ON LED_PORT |= _BV(LED_P);
#define LED_OFF LED_PORT &= ~_BV(LED_P);
#define LED_SW LED_PORT ^= _BV(LED_P);

#endif // ndef _CONFIG_H
