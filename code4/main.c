/*
 * main.c
 *
 *  Created on: 18 Sep 2020
 *      Author: dk5ee
 *     License: MIT
 *
 *  This code for software pwm got it's inspiration from Rick.
 *  He had to design a routine to modulate three LEDs with only
 *  one timer interrupt.
 *
 *  I have chosen an 8 bit timer interrupt on Atmega.
 *  Then I have chosen to use 7 LEDs instead of 3, driven by a
 *  ULN2003 darlington array.
 *  Each LED is a 1W rated SMD led, powered under the 
 *  maximum rating.
 *
 *  I have chosen an 8 bit timer interrupt on Atmega.
 *  Then I have chosen to use 7 LEDs instead of 3, driven by a
 *  ULN2003 darlington array.
 *  Each LED is a 1W rated SMD led, powered under the 
 *  specificated limit with 200mA, with a different wavelength.
 *
 *  To get proper linear results there must be an overflow after 255, 
 *  not 256 steps, so for each counter an extra condition must be
 *  checked.
 *
 *  After a short thinking two more features were incorporated:
 *  A) there is a phase shift for each LED, so the LEDs start up at 
 *  different times
 *  B) instead of incrementing with one, each counter is incremented
 *  with the chosen prime number 107 to increase the blinking frequency.
 *
 *  Interface:
 *  1. connect via uart
 *  2. repeat: enter number 0..255, then the command.
 *  if no number is input, the value will be zero
 *  commands:
 *  a..g - set led 0.. led 7 to value
 *  o    - all off
 *  s    - set all to value
 *  m    - for future use
 */

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ringuart.h"

/*
 * counters with phase shifts
 */

volatile uint16_t count_0 = 0;
volatile uint16_t count_1 = 197;
volatile uint16_t count_2 = 138;
volatile uint16_t count_3 = 79;
volatile uint16_t count_4 = 20;
volatile uint16_t count_5 = 217;
volatile uint16_t count_6 = 158;

volatile uint16_t pwm_0 = 0;
volatile uint16_t pwm_1 = 0;
volatile uint16_t pwm_2 = 0;
volatile uint16_t pwm_3 = 0;
volatile uint16_t pwm_4 = 0;
volatile uint16_t pwm_5 = 0;
volatile uint16_t pwm_6 = 0;

volatile uint32_t invalue = 0;
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
		uint16_t i = 0;
		txring_enqueue(ch);
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
			pwm_0 = getinvalue();
			ok();
			break;
		case 'b':
		case 'B':
			pwm_1 = getinvalue();
			ok();
			break;
		case 'c':
		case 'C':
			pwm_2 = getinvalue();
			ok();
			break;
		case 'd':
		case 'D':
			pwm_3 = getinvalue();
			ok();
			break;
		case 'e':
		case 'E':
			pwm_4 = getinvalue();
			ok();
			break;
		case 'f':
		case 'F':
			pwm_5 = getinvalue();
			ok();
			break;
		case 'g':
		case 'G':
			pwm_6 = getinvalue();
			ok();
			break;
		case 'm':
		case 'M':
			mode = getinvalue();
			ok();
			break;
		case 'o':
		case 'O':
			invalue = 0;
			pwm_0 = 0;
			pwm_1 = 0;
			pwm_2 = 0;
			pwm_3 = 0;
			pwm_4 = 0;
			pwm_5 = 0;
			pwm_6 = 0;
			ok();
			break;
		case 's':
		case 'S':
			i = getinvalue();
			pwm_0 = i;
			pwm_1 = i;
			pwm_2 = i;
			pwm_3 = i;
			pwm_4 = i;
			pwm_5 = i;
			pwm_6 = i;
			ok();
			break;
		default:
			txring_enqueue('.');
			break;
		}
	}

}

void setLED(uint8_t led, uint16_t value) {

	switch (led) {
	case 0:
		pwm_0 = value;
		break;
	case 1:
		pwm_1 = value;
		break;
	case 2:
		pwm_2 = value;
		break;
	case 3:
		pwm_3 = value;
		break;
	case 4:
		pwm_4 = value;
		break;
	case 5:
		pwm_5 = value;
		break;
	case 6:
		pwm_6 = value;
		break;
	default:
		break;
	}
}

void startPWM() {
	cli();

	DDRD |= (1 << PD2);
	DDRD |= (1 << PD3);
	DDRD |= (1 << PD4);
	DDRD |= (1 << PD5);
	DDRD |= (1 << PD6);
	DDRD |= (1 << PD7);
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
volatile uint8_t nextLED = 0;
ISR( TIMER0_OVF_vect) {

	if (nextLED > 6) {
		nextLED = 0;
	}
	volatile uint16_t *count;
	volatile uint16_t *pwm;
	uint8_t portval = 0;
	volatile uint8_t *pport;
	switch (nextLED) {
	case 0:
		count = &count_0;
		pwm = &pwm_0;
		pport = &PORTD;
		portval = (1 << PD2);
		break;
	case 1:
		count = &count_1;
		pwm = &pwm_1;
		pport = &PORTD;
		portval = (1 << PD3);
		break;
	case 2:
		count = &count_2;
		pwm = &pwm_2;
		pport = &PORTD;
		portval = (1 << PD4);
		break;
	case 3:
		count = &count_3;
		pwm = &pwm_3;
		pport = &PORTD;
		portval = (1 << PD5);
		break;
	case 4:
		count = &count_4;
		pwm = &pwm_4;
		pport = &PORTD;
		portval = (1 << PD6);
		break;
	case 5:
		count = &count_5;
		pwm = &pwm_5;
		pport = &PORTD;
		portval = (1 << PD7);
		break;
	default:
		count = &count_6;
		pwm = &pwm_6;
		pport = &PORTB;
		portval = (1 << PB0);
		break;
	}
	uint16_t counttemp = *count;
	if (counttemp < *pwm) {
		*pport |= portval;
	} else {
		*pport &= ~portval;
	}
	if (counttemp >= 40502) {
		*count = counttemp - 40502;
	} else {
		*count = counttemp + 25033;
	}
	nextLED++;
}
