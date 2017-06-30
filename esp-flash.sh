#!/bin/bash
avrdude -p atmega328p -c arduino -P net:192.168.4.1:23 -b 38400 -U flash:w:bin/Debug/space_sensei.hex:i
