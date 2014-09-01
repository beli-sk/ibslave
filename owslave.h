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

// delays (us)
#define T_ZERO 45
#define T_PRESENCE_DELAY 20
#define T_PRESENCE 140
#define T_SAMPLE 15
#define T_RESET 400

// main states
#define ST_NORMAL 0		// idle
#define ST_RESET 1		// reset pulse detected
#define ST_TIMEOUT 2	// session timeout

#define EVENT (status != ST_NORMAL)

#define CMD_READ_ROM 0x33

extern volatile uint8_t status;			// main status

void ows_init(void);
uint8_t ow_read_byte(void);
void ow_write_byte(uint8_t);
void ow_wait_reset(void);
void ow_present(void);
void ow_stop_timeout(void);

#endif // def _OWSLAVE_H
