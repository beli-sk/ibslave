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
			OW_DDR |= _BV(OW_P); \
			OW_PORT &= ~_BV(OW_P);

#define OW_RELEASE_BUS \
			OW_DDR &= ~_BV(OW_P); \
			OW_PORT |= _BV(OW_P); \
			transmit = 0;
#define OW_READ (OW_PIN & _BV(OW_P))

// timers (us)
#define T_ZERO 70
#define T_PRESENCE 70
#define T_SAMPLE 40

// main states
#define ST_IDLE 0		// idle
#define ST_RESET 2		// reset pulse detected
#define ST_PRESENCE 3	// transmiting presence pulse
#define ST_RECV_ROM 4	// receiving ROM command
#define ST_SEND_ID 5		// sending ID

#define CMD_READ_ROM 33

uint8_t status;			// main status
uint8_t transmit;

uint8_t buf; 		// bit buffer
uint8_t cnt;		// bit counter

const uint8_t id[] = OW_ID;
uint8_t id_cnt;

#define RESET_TIMER TCNT1
#define IO_TIMER TCNT0

void ows_init(void) {
	status = ST_IDLE;
	transmit = 0;

	// set timer 0 to 1us, timer 1 to 2us ticks
#if F_CPU == 1000000
	// timer 0: clkIO clock source, no prescaling
	TCCR0B |= _BV(CS00);
	// timer 1: clkIO/2
	TCCR1 |= _BV(CS11);
#elif F_CPU == 8000000
	// timer 0: clkIO/8 clock source
	TCCR0B |= _BV(CS01);
	// timer 1: clkIO/16
	TCCR1 |= _BV(CS12);
	TCCR1 |= _BV(CS10);
#else
#	error "Unsupported system frequency (only 1 or 8 MHz supported)"
#endif
	// reset timeout (in 2us units)
	OCR1A = 240; // a little lower than 480us
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
	LED_ON;
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
			break;
		case ST_RECV_ROM:
			if (OW_READ) {
				buf |= _BV(cnt);
			}
			cnt++;
			if (cnt == 6) {
				LED_OFF;
			}
			if (cnt >= 8) {
				if (buf == CMD_READ_ROM) {
					status = ST_SEND_ID;
					id_cnt = 0;
					buf = id[id_cnt];
					cnt = 0;
				}
			}
			break;
	}
}

// should be called when change is detected on the data pin
void ows_pin_change(uint8_t value) {
	if (!value) {
		start_reset_timer();
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
					set_alarm(T_ZERO);
				}
				// set next bit
				cnt++;
				if (cnt >= 8) {
					cnt = 0;
					id_cnt++;
					if (id_cnt < 8) {
						buf = id[id_cnt];
					} else {
						// finished, return to idle
						status = ST_IDLE;
					}
				}
				break;
		}
	} else { // (value)
		stop_reset_timer();
		if (status == ST_RESET) {
			status = ST_PRESENCE;
			OW_PULL_BUS_LOW;
			set_alarm(T_PRESENCE);
		}
	}
}

