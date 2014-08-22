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
#ifndef _OWSLAVE_H
#define _OWSLAVE_H

#include "config.h"
#include <stdint.h>

#if !defined(F_CPU) || !defined(OW_DDR) || !defined(OW_PORT) || \
		!defined(OW_PIN) || !defined(OW_P) || !defined(OW_ID)
#	error "Required variable undefined (F_CPU, OW_DDR, OW_PORT, OW_PIN, OW_P, OW_ID)"
#endif

// timers (us)
#if F_CPU == 16000000UL
// counter runs at 0.5us intervals, double the values
#	define T_ZERO 90
#	define T_PRESENCE 140
#	define T_SAMPLE 30
#else
#	define T_ZERO 45
#	define T_PRESENCE 70
#	define T_SAMPLE 15
#endif

// main states
#define ST_IDLE 0		// idle
#define ST_RESET 2		// reset pulse detected
#define ST_PRESENCE 3	// transmiting presence pulse
#define ST_RECV_ROM 4	// receiving ROM command
#define ST_SEND_ID 5		// sending ID

#define CMD_READ_ROM 0x33

extern uint8_t status;			// main status

void ows_init(void);
void ows_pin_change(uint8_t value);

#endif // def _OWSLAVE_H
