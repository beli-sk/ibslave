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
#include "config.h"
#include "owslave.h"

#include <avr/io.h>
#include <stdint.h>

#define OW_PULL_BUS_LOW \
			transmit = 1; \
            OW_DDR |= _BV(OW_P); \
            OW_PORT &= ~_BV(OW_P);

#define OW_RELEASE_BUS \
            OW_DDR &= ~_BV(OW_P); \
            OW_PORT |= _BV(OW_P); \
			transmit = 0;

// main states
#define ST_IDLE 0		// idle
#define ST_IDLE_LOW 1	// bus low when idle
#define ST_RESET		// reset pulse detected
#define ST_PRESENCE 2	// transmiting presence pulse
#define ST_RECVROM 3	// receiving ROM command

uint8_t status;			// main status
uint8_t transmit;		// transmitting (holding bus low)

uint8_t recv_buf; 		// receive bit buffer
uint8_t recv_cnt;		// receive bit counter

#define TIMER TCNT0

void ows_init(uint8_t) {
	status = ST_IDLE;
	transmit = 0;

	// set counter to 1us ticks
#if F_CPU_M == 1
	// clkIO clock source, no prescaling
	TCCR0B |= _BV(CS00);
#elif F_CPU_M == 8
	// clkIO/8 clock source
	TCCR0B |= _BV(CS01);
#else
#	error "Unsupported system frequency (only 1 or 8 MHz supported)"
#endif
}

void reset_timer(uint8_t) {
	// stop timer
	GTCCR |= _BV(TSM);
	GTCCR |= _BV(PSR0);

	TIMER = 0;

	// start timer
	GTCCR &= ~_BV(TSM);
}

void set_alarm(uint8_t time) {
	// stop timer
	GTCCR |= _BV(TSM);
	GTCCR |= _BV(PSR0);

	// enable timer compare interrupt
	TIMSK |= _BV(OCIE0A);
	OCR0A = time;

	TIMER = 0;

	// start timer
	GTCCR &= ~_BV(TSM);
}

// alarm handler
ISR (TIMER0_COMPA_vect) {
	switch (status) {
		case ST_IDLE_LOW:
			// reset pulse threshold reached
			status = ST_RESET;
			break;
		case ST_PRESENCE:
			OW_RELEASE_BUS;
			recv_buf = recv_cnt = 0;
			status = ST_RECVROM;
	} else if (transmit) {
			OW_RELEASE_BUS;
	}
}

// should be called when change is detected on the data pin
void ows_pin_change(uint8_t value) {
	switch (status) {
		case ST_RESET:
			if (value) {
				status = ST_PRESENCE;
				OW_PULL_BUS_LOW;
				set_alarm(60);
			}
			break;
	}
}

