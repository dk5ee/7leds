/*
 * ringuart.h
 *
 *      minicom -b 9600 -D /dev/ttyAMA0
 *      besser: putty
 *      oder: screen /dev/ttyUSB3 9600
 *      	beenden mit Ctrl-a K y
 */

#ifndef RINGUART_H_
#define RINGUART_H_
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//bitsize should be between 2 and 7
#define RINGUART_RX_BUFFER_BITSIZE 7
#define RINGUART_TX_BUFFER_BITSIZE 7
#define RINGUART_BAUD_RATE 9600

void ringuartinit();
void txring_enqueue(uint8_t data); // - am ende einfügen. automatisch senden starten.
uint8_t txring_count(); // - noch zu sendende zeichen im buffer
uint8_t txring_space(); // - noch verfügbarer space im buffer

uint8_t rxring_count(); // - noch zu verarbeitende zeichen im buffer
uint8_t rxring_space(); // - noch verfügbarer space im buffer
uint8_t rxring_dequeue(); // - erstes zeichen entfernen und ausliefern
uint8_t rxring_peek(uint8_t offset); // - kommendes zeichen entfernen
uint8_t rxstatus();
uint8_t rxresetstatus();
uint8_t firstmsglen();
void firstmsgdel();
void savetrim();
void firstmsgtrim() ;
void ring_outchar(uint8_t data); //wrapper
//void ring_outPstr(); //from progmem;
#define ring_space ring_outchar(' ');
#define ring_newline ring_outchar('\n');ring_outchar('\r');
#define ring_EOL ring_outchar(0x0D);
#define ring_NL ring_outchar(0x0A);
void ring_outbyte(uint8_t data);
void ring_outword(uint16_t data);
void ring_outlong(uint32_t data);
void ring_outuint(uint16_t wert);
void ring_outint(int16_t d);

#endif /* RINGUART_H_ */
