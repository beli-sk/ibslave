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

#define __delay_cycles(arg) __builtin_avr_delay_cycles(arg);
#define OWI_DELAY_OFFSET_CYCLES    13 // Timing delay when pulling bus low and releasing bus.

#if !defined(F_CPU_M) || !defined(OW_DDR) || !defined(OW_PORT) || \
		!defined(OW_PIN) || !defined(OW_P) || !defined(OW_PCINT)
#	error "Required variable undefined (F_CPU_M, OW_DDR, OW_PORT, OW_PIN or OW_P)"
#endif

#endif // def _OWSLAVE_H
