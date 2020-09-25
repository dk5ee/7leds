#!/bin/bash

avrdude -p m328p -b 57600 -c arduino -U flash:w:7leds.hex:i -F -P /dev/ttyUSB3