#!/bin/bash
./clean.sh

avr-gcc -Wall -g -Os -mmcu=atmega328p -DF_CPU=16000000UL -o 7leds.bin "../main.c"
avr-objcopy -j .text -j .data -O ihex 7leds.bin 7leds.hex
avr-size --format=avr --mcu=atmega328p 7leds.bin
