#!/bin/bash
avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"main.d" -MT"main.d" -c -o "main.o" "../main.c"
avr-gcc -Wl,-Map,7leds.map -mmcu=atmega328p -o "7leds.elf"  ./main.o   
avr-objdump -h -S 7leds.elf  >"7leds.lss"
avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex 7leds.elf  "7leds.hex"
avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex 7leds.elf  "7leds.eep"
avr-size --format=avr --mcu=atmega328p 7leds.elf
