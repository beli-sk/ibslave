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

// code is tuned for ATtiny85

#include "config.h"
#include "owslave.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

const uint8_t id[] = OW_ID;
uint8_t id_cnt;

int main(void) {
	// disable power to USI and ADC
	PRR |= _BV(PRUSI);
	PRR |= _BV(PRADC);
	PRR |= _BV(PRTIM1);
	// disable pull-ups
	MCUCR |= _BV(PUD);

	LED_INIT;

	ows_init();

	sei();

	while(1) {
		ow_wait_reset();
		ow_present();
		if (ow_read_byte() == CMD_READ_ROM) {
			for (id_cnt = 0; id_cnt < 8; ++id_cnt) {
				ow_write_byte(id[id_cnt]);
			}
		}
		if (status == ST_TIMEOUT) {
			status = ST_NORMAL;
		} else {
			ow_stop_timeout();
		}
	} // while(1)
}
