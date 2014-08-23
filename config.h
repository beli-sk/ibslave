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

//#define DEBUG
#define DEBUG_LED

#define F_CPU 8000000UL

#define OW_ID {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xf3}
#define OW_PORT PORTB
#define OW_DDR DDRB
#define OW_PIN PINB
#define OW_P PB3

#if defined(DEBUG) || defined(DEBUG_LED)
#	define LED_PORT PORTB
#	define LED_DDR DDRB
#	define LED_P PB4
#	define LED_INIT LED_PORT &= ~_BV(LED_P); LED_DDR |= _BV(LED_P);
#	define LED_ON LED_PORT |= _BV(LED_P);
#	define LED_OFF LED_PORT &= ~_BV(LED_P);
#	define LED_SW LED_PORT ^= _BV(LED_P);
#else
#	define LED_INIT
#	define LED_ON
#	define LED_OFF
#	define LED_SW
#endif // def DEBUG || DEBUG_LED

#ifdef DEBUG
#	define DBG_PORT PORTB
#	define DBG_DDR DDRB
#	define DBG_MASK 0x07
#	define DBG_INIT (DBG_DDR |= DBG_MASK)
#	define DBG_OUT(x) (DBG_PORT = (DBG_PORT & ~DBG_MASK) | (x & DBG_MASK))
#else
#   define DBG_INIT
#   define DBG_OUT(x)
#endif // def DEBUG

#endif // ndef _CONFIG_H
