/*
 * main.c
 *
 */

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ringuart.h"

/*
 * counters with phase shifts
 */

volatile uint16_t pwmcount = 0;

volatile uint16_t pwmval = 0;

volatile int32_t invalue = 0;
volatile uint8_t mode = 0;

uint16_t getinvalue() {
	if (invalue > 65535)
		return 65535;
	uint16_t newval = invalue;
	invalue = 0;
	return newval;
}
void ok() {
	txring_enqueue('o');
	txring_enqueue('k');
	txring_enqueue('\r');
	txring_enqueue('\n');
}

void inparse() {
	while (rxring_count()) {
		uint8_t ch = rxring_dequeue();
		switch (ch) {
		case '0':
			invalue *= 10;
			break;
		case '1':
			invalue *= 10;
			invalue += 1;
			break;
		case '2':
			invalue *= 10;
			invalue += 2;
			break;
		case '3':
			invalue *= 10;
			invalue += 3;
			break;
		case '4':
			invalue *= 10;
			invalue += 4;
			break;
		case '5':
			invalue *= 10;
			invalue += 5;
			break;
		case '6':
			invalue *= 10;
			invalue += 6;
			break;
		case '7':
			invalue *= 10;
			invalue += 7;
			break;
		case '8':
			invalue *= 10;
			invalue += 8;
			break;
		case '9':
			invalue *= 10;
			invalue += 9;
			break;
		case 'a':
		case 'A':
			pwmval = getinvalue();
			ok();
			break;
		case 'o':
		case 'O':
			invalue = 0;
			pwmval = 0;
			ok();
			break;
		default:
			txring_enqueue('.');
			break;
		}
	}

}

void setLED(int32_t value) {
	if (value < 0) {
		pwmval = 0;
		return;
	}
	if (value > 65535) {
		pwmval = 65535;
		return;
	}
	pwmval = value;
}

void startPWM() {
	cli();

	DDRB |= (1 << PB0);

	//clear timer..
	TCCR0A = 0;
	TCCR0B = 0;

	TCCR0B |= (1 << CS00);
	//set interrupt
	TIMSK0 |= (1 << TOIE0);
	sei();
}

int main(void) {
	ringuartinit();
	startPWM();
	while (1) {
		inparse();
	}

	return (0);
}
ISR( BADISR_vect) {
	PORTD |= (1 << PD4);
}
ISR( TIMER0_OVF_vect) {
	uint16_t count;
	count = pwmcount;
#define TRIGGERVAL 25033
#define TRIGGERVAL2 (65535-TRIGGERVAL)
	if (count < pwmval) {
		PORTB |= (1 << PB0);
	} else {
		PORTB &= ~(1 << PB0);
	}
	if (count >= TRIGGERVAL) {
		pwmcount = count - TRIGGERVAL;
	} else {
		pwmcount = count + TRIGGERVAL2;
	}
}
