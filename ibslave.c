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
#include <avr/sleep.h>

ISR (PCINT0_vect) {
	ows_pin_change(OW_PIN & _BV(OW_P));
}

int main(void) {
	// disable power to USI and ADC
	PRR |= _BV(PRUSI);
	PRR |= _BV(PRADC);

	LED_INIT;

	ows_init();

	// set up pin change interrupts
	GIMSK |= _BV(PCIE);
	PCMSK |= _BV(PCINT3);

	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();
	while(1) {
		sleep_mode();
	}
}
