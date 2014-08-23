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
#include <util/delay.h>
#include <avr/sleep.h>

#define OW_PULL_BUS_LOW \
			OW_PORT &= ~_BV(OW_P); \
			OW_DDR |= _BV(OW_P);

#define OW_RELEASE_BUS \
			OW_DDR &= ~_BV(OW_P); \
			OW_PORT |= _BV(OW_P);

#define OW_READ (OW_PIN & _BV(OW_P))

volatile uint8_t status;	// status event

uint8_t buf; 		// bit buffer
uint8_t cnt;		// bit counter

/*** INTERNAL FUNCTIONS ***/

/* low level signaling */

// wait for low level
void wait_fall(void) {
	while(OW_READ && !EVENT);
}

// wait for high level
void wait_raise(void) {
	while(!OW_READ && !EVENT);
}

void write_one(void) {
	wait_fall();
	wait_raise();
}

void write_zero(void) {
	wait_fall();
	OW_PULL_BUS_LOW;
	_delay_us(T_ZERO);
	OW_RELEASE_BUS;
}

// returns zero/non-zero
uint8_t read_bit(void) {
	wait_raise();
	wait_fall();
	_delay_us(T_SAMPLE);
	return OW_READ;
}

/* timer control */

void enable_pcint(void) {
	GIFR = _BV(PCIF);
	GIMSK |= _BV(PCIE);
}

void disable_pcint(void) {
	GIMSK &= ~_BV(PCIE);
}

void reset_timer(void) {
	TCNT0 = 0;
	TIFR = _BV(OCF0A);
	TIFR = _BV(TOV0);
	TIMSK |= _BV(OCIE0A);
	TIMSK |= _BV(TOIE0);
}

void stop_reset_timer(void) {
	// mask timer interrupt
	TIMSK &= ~_BV(OCIE0A);
}

// reset handler
ISR (TIMER0_COMPA_vect) {
	disable_pcint();
	stop_reset_timer();
	status = ST_RESET;
}

// timeout handler
ISR (TIMER0_OVF_vect) {
	ow_stop_timeout();
	status = ST_TIMEOUT;
}

// change is detected on the data pin
ISR (PCINT0_vect) {
	if (!OW_READ) {
		// read 0
		reset_timer();
	} else {
		// read 1
		stop_reset_timer();
	}
}

/*** PUBLIC INTERFACE ***/

// stop timer after end of session
void ow_stop_timeout(void) {
	TIMSK &= ~_BV(TOIE0);
}

uint8_t ow_read_byte(void) {
	wait_raise();
	buf = 0;
	for (cnt = 0; cnt < 8 && !EVENT; ++cnt) {
		if (read_bit()) {
			buf |= _BV(cnt);
		}
	}
	return buf;
}

void ow_write_byte(uint8_t val) {
	wait_raise();
	for (cnt = 0; cnt < 8 && !EVENT; ++cnt) {
		if (val & 1) {
			write_one();
		} else {
			write_zero();
		}
		val >>= 1;
	}
}

// sleep and wait for reset
void ow_wait_reset(void) {
	enable_pcint();
	while(1) {
		cli();
		if (status != ST_RESET && OW_READ) {
			LED_ON;
			sleep_enable();
			sei();
			sleep_cpu();
			sleep_disable();
		}
		sei();
		LED_OFF;
		if (status == ST_RESET) {
			break;
		}
	}
	sei();
	status = ST_NORMAL;
}

// send presence pulse
void ow_present(void) {
	wait_raise();
	_delay_us(15);
	OW_PULL_BUS_LOW;
	_delay_us(T_PRESENCE);
	OW_RELEASE_BUS;
}

void ows_init(void) {
	status = ST_NORMAL;
	DBG_INIT;
	DBG_OUT(status);
	OW_RELEASE_BUS;

#if F_CPU == 8000000UL
	// timer 0: clkIO/256 clock source, 32ms ticks
	TCCR0B |= _BV(CS02);
	OCR0A = T_RESET /32;
#elif F_CPU == 1000000UL
	// timer 0: clkIO/64 clock source, 64ms ticks
	TCCR0B |= _BV(CS01);
	TCCR0B |= _BV(CS00);
	OCR0A = T_RESET /64;
#else
#	error "Unsupported system frequency"
#endif

	// set up pin change interrupt
	PCMSK |= _BV(PCINT3);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

