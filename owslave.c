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
#include <avr/interrupt.h>
#include <stdint.h>

#define OW_PULL_BUS_LOW \
			transmit = 1; \
			OW_PORT &= ~_BV(OW_P); \
			OW_DDR |= _BV(OW_P);

#define OW_RELEASE_BUS \
			OW_DDR &= ~_BV(OW_P); \
			OW_PORT |= _BV(OW_P); \
			transmit = 0;

#define OW_READ (OW_PIN & _BV(OW_P))

uint8_t transmit;
uint8_t status;			// main status

uint8_t buf; 		// bit buffer
uint8_t cnt;		// bit counter

const uint8_t id[] = OW_ID;
uint8_t id_cnt;

#define RESET_TIMER TCNT1
#define IO_TIMER TCNT0

void ows_init(void) {
	status = ST_IDLE;
	DBG_INIT;
	DBG_OUT(status);
	OW_RELEASE_BUS;

	// set timer 0 to 1us, timer 1 to 2us ticks
#if F_CPU == 1000000UL
#	warning "Works correctly only at 16MHz"
	// timer 0: clkIO clock source, no prescaling
	TCCR0B |= _BV(CS00);
	// timer 1: clkIO/2
	TCCR1 |= _BV(CS11);
#elif F_CPU == 8000000UL
#	warning "Works correctly only at 16MHz"
	// timer 0: clkIO/8 clock source
	TCCR0B |= _BV(CS01);
	// timer 1: clkIO/16
	TCCR1 |= _BV(CS12);
	TCCR1 |= _BV(CS10);
#elif F_CPU == 16000000UL
	// timer 0: clkIO/8 clock source (0.5us, delays adjusted)
	TCCR0B |= _BV(CS01);
	// timer1: clkIO/32
	TCCR1 |= _BV(CS12);
	TCCR1 |= _BV(CS11);
#else
#	error "Unsupported system frequency (only 1, 8 and 16 MHz supported)"
#endif
	// reset timeout (in 2us units)
	OCR1A = 200; // a little lower than 400us

	// set up pin change interrupt
	GIMSK |= _BV(PCIE);
	PCMSK |= _BV(PCINT3);
}

void start_reset_timer(void) {
	RESET_TIMER = 0;
	TIFR = _BV(OCF1A);
	TIMSK |= _BV(OCIE1A);
}

void stop_reset_timer(void) {
	// mask timer interrupt
	TIMSK &= ~_BV(OCIE1A);
}

void set_alarm(uint8_t time) {
	IO_TIMER = 0;
	// enable timer compare interrupt
	OCR0A = time;
	TIFR = _BV(OCF0A);
	TIMSK |= _BV(OCIE0A);
}

void clear_alarm(void) {
	// mask timer interrupt
	TIMSK &= ~_BV(OCIE0A);
}

// reset handler
ISR (TIMER1_COMPA_vect) {
	stop_reset_timer();
	status = ST_RESET;
	DBG_OUT(status);
}

// alarm handler
ISR (TIMER0_COMPA_vect) {
	clear_alarm();
	if (transmit) {
		OW_RELEASE_BUS;
	}
	switch (status) {
		case ST_PRESENCE:
			buf = cnt = 0;
			status = ST_RECV_ROM;
			DBG_OUT(status);
			break;
		case ST_RECV_ROM:
			if (OW_READ) {
				buf |= _BV(cnt);
			} else {
				;
			}
			cnt++;
			if (cnt >= 8) {
				if (buf == CMD_READ_ROM) {
					status = ST_SEND_ID;
					DBG_OUT(status);
					id_cnt = 0;
					buf = id[id_cnt];
					cnt = 0;
				} else {
					status = ST_IDLE;
					DBG_OUT(status);
				}
			}
			break;
		case ST_SEND_ID:
			// set next bit
			buf >>= 1;
			cnt++;
			if (cnt >= 8) {
				cnt = 0;
				id_cnt++;
				if (id_cnt < 8) {
					buf = id[id_cnt];
				} else {
					// finished, return to idle
					status = ST_IDLE;
					DBG_OUT(status);
				}
			}
			break;
	}
}

// change is detected on the data pin
ISR (PCINT0_vect) {
	if (transmit) {
		return;
	}
	if (!OW_READ) {
		// read 0
		start_reset_timer();
		LED_OFF;
		switch (status) {
			case ST_RECV_ROM:
				// receiving bit
				set_alarm(T_SAMPLE);
				break;
			case ST_SEND_ID:
				// sending ID
				if (!(buf & 1)) {
					// write zero
					OW_PULL_BUS_LOW;
				}
				set_alarm(T_ZERO);
				break;
		}
	} else {
		// read 1
		LED_ON;
		stop_reset_timer();
		if (status == ST_RESET) {
			status = ST_PRESENCE;
			DBG_OUT(status);
			OW_PULL_BUS_LOW;
			set_alarm(T_PRESENCE);
		}
	}
}

