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
 *  Still missing:
 *  some input code for changing the colors with either uart or with 
 *  other inputs
 */

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * counters with phase shifts
 */
volatile uint8_t count_0 = 0;
volatile uint8_t count_1 = 197;
volatile uint8_t count_2 = 138;
volatile uint8_t count_3 = 79;
volatile uint8_t count_4 = 20;
volatile uint8_t count_5 = 217;
volatile uint8_t count_6 = 158;
#define fasterpwmisr
/*
 * fasterpwm leads to more calls of ISR, but ISR is shorter in execution
 */

#ifdef fasterpwmisr
volatile uint8_t nowpwm = 0;
#endif

volatile uint8_t pwm_0 = 0;
volatile uint8_t pwm_1 = 0;
volatile uint8_t pwm_2 = 0;
volatile uint8_t pwm_3 = 0;
volatile uint8_t pwm_4 = 0;
volatile uint8_t pwm_5 = 0;
volatile uint8_t pwm_6 = 0;
void setLED(uint8_t led, uint8_t value) {
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
	//using timer 0

	PORTD |= (1 << PD2);
	PORTD |= (1 << PD3);
	PORTD |= (1 << PD4);
	PORTD |= (1 << PD5);
	PORTD |= (1 << PD6);
	PORTD |= (1 << PD7);
	PORTB |= (1 << PB0);

	//clear timer..
	TCCR0B &= ~(1 << CS02);
	TCCR0B &= ~(1 << CS01);
	TCCR0B &= ~(1 << CS00);

#ifdef fasterpwmisr
	//set timer.. no prescale
	TCCR0B |= (0 << CS02);
	TCCR0B |= (0 << CS01);
	TCCR0B |= (1 << CS00);

#else
	//set timer.. prescale 8
	TCCR0B |= (0 << CS02);
	TCCR0B |= (1 << CS01);
	TCCR0B |= (0 << CS00);
#endif

	//set interrupt
	TIMSK0 |= (1 << TOIE0);
}

int main(void) {
	startPWM();
	uint8_t counter = 0;
	uint8_t selectedLED = 0;
	uint8_t dir = 1;
	while (1) {
		counter++;
		if (counter == 0) {
			//overflow
			if (dir) {
				//return direction;
				dir = 0;
			} else {
				dir = 1;
				setLED(selectedLED, 0); //old led off
				selectedLED++;
				if (selectedLED > 6) {
					selectedLED = 0;
				}
			}

		}
		if (dir) {
			setLED(selectedLED, counter);
		} else {
			setLED(selectedLED, 255 - counter);
		}
		_delay_ms(10);
	}

	return (0);
}
ISR (TIMER0_OVF_vect) {
	uint8_t count;
#ifdef fasterpwmisr
	uint8_t pwmselected = nowpwm;
	switch (pwmselected) {
	case 0:
#endif
		count = count_0;
		if (count >= pwm_0) {
			PORTD |= (1 << PD2);
		} else {
			PORTD &= ~(1 << PD2);
		}
		if (count >= 148) {
			count_0 = count - 148;
		} else {
			count_0 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 1:
#endif
		count = count_1;
		if (count >= pwm_1) {
			PORTD |= (1 << PD3);
		} else {
			PORTD &= ~(1 << PD3);
		}
		if (count >= 148) {
			count_1 = count - 148;
		} else {
			count_1 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 2:
#endif
		count = count_2;
		if (count >= pwm_2) {
			PORTD |= (1 << PD4);
		} else {
			PORTD &= ~(1 << PD4);
		}
		if (count >= 148) {
			count_2 = count - 148;
		} else {
			count_2 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 3:
#endif
		count = count_3;
		if (count >= pwm_3) {
			PORTD |= (1 << PD5);
		} else {
			PORTD &= ~(1 << PD5);
		}
		if (count >= 148) {
			count_3 = count - 148;
		} else {
			count_3 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 4:
#endif
		count = count_4;
		if (count >= pwm_4) {
			PORTD |= (1 << PD6);
		} else {
			PORTD &= ~(1 << PD6);
		}
		if (count >= 148) {
			count_4 = count - 148;
		} else {
			count_4 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 5:
#endif
		count = count_5;
		if (count >= pwm_5) {
			PORTD |= (1 << PD7);
		} else {
			PORTD &= ~(1 << PD7);
		}
		if (count >= 148) {
			count_5 = count - 148;
		} else {
			count_5 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	case 6:
#endif
		count = count_6;
		if (count >= pwm_6) {
			PORTB |= (1 << PB0);
		} else {
			PORTB &= ~(1 << PB0);
		}
		if (count >= 148) {
			count_6 = count - 148;
		} else {
			count_6 = count + 107;
		}
#ifdef fasterpwmisr
		break;
	default:
		nowpwm = 0;
		return;
		break;

	}
	nowpwm = pwmselected + 1;
#endif
}
