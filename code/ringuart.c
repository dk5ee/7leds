/*
 * ringuart.c
 *
 */

#include "ringuart.h"

#define RXRING_BUFFER (1<<RINGUART_RX_BUFFER_BITSIZE)
#define RXRING_WRAP (RXRING_BUFFER -1 )
#define TXRING_BUFFER (1<<RINGUART_TX_BUFFER_BITSIZE)
#define TXRING_WRAP (TXRING_BUFFER -1 )

#if defined (UDR)
#define myU2X U2X
#define myUBRRL UBRRL
#define myUBRRH UBRRH
#define myUCSRA UCSRA
#define myUCSRB UCSRB
#define myUCSRC UCSRC
#define myUCSZ0 UCSZ0
#define myUCSZ1 UCSZ1
#define myUDR UDR
#define myUDRIE UDRIE
#define myRXCIE RXCIE
#define myRXEN RXEN
#define myTXCIE TXCIE
#define myTXEN TXEN
#endif
#if defined (UDR0)
#define myU2X U2X0
#define myUBRRL UBRR0L
#define myUBRRH UBRR0H
#define myUCSRA UCSR0A
#define myUCSRB UCSR0B
#define myUCSRC UCSR0C
#define myUCSZ0 UCSZ00
#define myUCSZ1 UCSZ01
#define myUDR UDR0
#define myUDRIE UDRIE0
#define myRXCIE RXCIE0
#define myRXEN RXEN0
#define myTXCIE TXCIE0
#define myTXEN TXEN0
#endif
/*
 // for second uart comment block above, use this block instead:
 #if defined (UDR1)
 #define myU2X U2X1
 #define myUBRRL UBRR1L
 #define myUBRRH UBRR1H
 #define myUCSRA UCSR1A
 #define myUCSRB UCSR1B
 #define myUCSRC UCSR1C
 #define myUCSZ0 UCSZ10
 #define myUCSZ1 UCSZ11
 #define myUDR UDR1
 #define myUDRIE UDRIE1
 #define myRXCIE RXCIE1
 #define myRXEN RXEN1
 #define myTXCIE TXCIE1
 #define myTXEN TXEN1
 #endif
 */

static uint8_t RxRing[RXRING_BUFFER];
static uint8_t TxRing[TXRING_BUFFER];
static volatile uint8_t RxHead = 0;
static volatile uint8_t RxTail = 0;
static volatile uint8_t RxStatus = 0;
static volatile uint8_t TxHead = 0;
static volatile uint8_t TxTail = 0;

void ringuartinit() {
	cli();
#if defined BAUD
#undef BAUD
#endif
#define BAUD RINGUART_BAUD_RATE
#include <util/setbaud.h>
	myUBRRL = UBRRL_VALUE;
	myUBRRH = UBRRH_VALUE;
	myUCSRB = ((1 << myRXCIE) | (1 << myRXEN) | (1 << myTXEN));
	myUCSRC = (1 << myUCSZ1) | (1 << myUCSZ0);
#if USE_2X
	myUCSRA |= (1 << myU2X);
#else
	myUCSRA &= ~(1 << myU2X);
#endif
	sei();
}
void txring_enqueue(uint8_t data) {
	//wait while txring is full..
	while ((((TxTail + 1) - TxHead) & TXRING_WRAP)==0){
	//do nothing
	//todo: idle function
	}
	uint8_t tempadr = (TxTail + 1) & TXRING_WRAP;
	TxRing[tempadr] = data;
	TxTail = tempadr;
	//now: allways activate interupt for sending
	myUCSRB |= (1 << myUDRIE);
}
uint8_t txring_count() {
	return ((TxTail - TxHead) & TXRING_WRAP);
}
uint8_t txring_space() {
	return (TXRING_WRAP - ((TxTail - TxHead) & TXRING_WRAP));
}
uint8_t rxring_count() {
	return ((RxTail - RxHead) & RXRING_WRAP);
}
uint8_t rxring_space() {
	return (RXRING_WRAP - ((RxTail - RxHead) & RXRING_WRAP));
}
uint8_t rxring_dequeue() {
	if (RxTail == RxHead) {
		return 255;
	} else {
		uint8_t tempadr = (RxHead + 1) & RXRING_WRAP;
		RxHead = tempadr;
		return RxRing[tempadr];
	}
}
uint8_t rxring_peek(uint8_t offset) {
	uint8_t tempadr =((1 + offset + RxHead) & RXRING_WRAP);
	return RxRing[tempadr];
}
const char hex[] = "0123456789ABCDEF";
void ring_outbyte(uint8_t data) {
	txring_enqueue('0');
	txring_enqueue('x');
	txring_enqueue(hex[(data >> 4)]);
	txring_enqueue(hex[data & 15]);
}
void ring_outword(uint16_t data) {
	txring_enqueue('0');
	txring_enqueue('x');
	txring_enqueue(hex[(data >> 12)]);
	txring_enqueue(hex[(data >> 8) & 15]);
	txring_enqueue(hex[(data >> 4) & 15]);
	txring_enqueue(hex[data & 15]);
}
void ring_outlong(uint32_t data) {
	txring_enqueue('0');
	txring_enqueue('x');
	txring_enqueue(hex[(data >> 28)]);
	txring_enqueue(hex[(data >> 24) & 15]);
	txring_enqueue(hex[(data >> 20) & 15]);
	txring_enqueue(hex[(data >> 16)& 15]);
	txring_enqueue(hex[(data >> 12)& 15]);
	txring_enqueue(hex[(data >> 8) & 15]);
	txring_enqueue(hex[(data >> 4) & 15]);
	txring_enqueue(hex[data & 15]);
}
#define NullZeichen '0'
void ring_outuint(uint16_t wert) {
	uint16_t laufend = wert; //diese variable wird vermutlich vom compiler wegoptimiert
	uint8_t merker = NullZeichen;
	uint8_t erstes = 0; // um führende nullen zu eliminieren
	while (laufend >= 10000) {
		laufend -= 10000;
		merker++;
		erstes = 1;
	}
	if (erstes) {
		txring_enqueue((uint8_t) merker);
	}
	merker = NullZeichen;
	while (laufend >= 1000) {
		laufend -= 1000;
		merker++;
		erstes = 1;
	}
	if (erstes) {
		txring_enqueue((uint8_t) merker);
	}
	merker = NullZeichen;
	while (laufend >= 100) {
		laufend -= 100;
		merker++;
		erstes = 1;
	}
	if (erstes) {
		txring_enqueue((uint8_t) merker);
	}
	merker = NullZeichen;
	while (laufend >= 10) {
		laufend -= 10;
		merker++;
		erstes = 1;
	}
	if (erstes) {
		txring_enqueue((uint8_t) merker);
	}
	//auf jeden fall letzte ziffer auspucken
	txring_enqueue((uint8_t) laufend + NullZeichen);

}
void ring_outint(int16_t d) {
	if (d < 0) {
		uint16_t e = 0 - d;
		txring_enqueue('-');
		ring_outuint(e);
	} else
		ring_outuint(d);
}
inline void ring_outchar(uint8_t data) {
	//wrapper
	txring_enqueue(data);
}
uint8_t firstmsglen() {
	uint8_t maxlen = rxring_count();
	if (maxlen == 0) {
		return 0;
	}
	uint8_t stepper = 0;
	for (stepper = 0; stepper <= maxlen; stepper++) {
		char thischar = rxring_peek(stepper);
		//txring_enqueue(thischar);
		if ((thischar == '\n') || (thischar == '\r')){
			return stepper;
		}
	}
	return 0;
}
void savetrim(){
	if(rxring_peek(0)<32) rxring_dequeue();
}
void firstmsgtrim() {
	uint8_t c=(rxring_count());
//	ring_outchar('p');
//	ring_outbyte(rxring_peek(0));
	while ((c > 1) && (rxring_peek(0) < 32)) {
		c--;
		rxring_dequeue();
	}
}
void firstmsgdel() {
	savetrim();
	uint8_t maxlen = rxring_count();
	if (maxlen == 0) {
		return;
	}
	uint8_t stepper = 0;
	for (stepper = 0; stepper <= maxlen; stepper++) {
		char thischar = rxring_dequeue();
		if ((thischar == '\n') || (thischar == '\r')) {
			return;
		}
	}
}

ISR(USART_RX_vect) {
	uint8_t data;
	data = UDR0;
	/*if (data == '\n')
	 RxStatus |= 1;
	 if (data == '\r')
	 RxStatus |= 2;*/
	//if ((data < 128) && (data > 9)) {
	//	if ((data >= 'a') && (data <= 'z'))
	//		data = data - 'a' + 'A';
		uint8_t temp;
		temp = ((RxTail + 1) & RXRING_WRAP);
		if (temp == RxHead) {
			RxStatus |= 4; //overflow happened.. OMG!
		} else {
			RxTail = temp;
			RxRing[temp] = data;
		}
	//} else {
	//	//RxStatus |= 8; //no printable ascii
	//}
}
ISR(USART_UDRE_vect) {
	if ((TxTail - TxHead) & TXRING_WRAP) {
		uint8_t temp;
		temp = ((TxHead + 1) & TXRING_WRAP);
		TxHead = temp;
		myUDR = TxRing[temp];
	} else {
		myUCSRB &= ~(1 << myUDRIE); //disable sending interrupt
	}

}
