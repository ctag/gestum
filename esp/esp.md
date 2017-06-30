# ESP Link

Using esp-link firmware on Wemos D1 mini board (esp8266).

## Installation

https://github.com/jeelabs/esp-link/blob/master/FLASHING.md

Download release archive and extract it.

Command to flash D1 mini:

    esptool --chip esp8266 --port /dev/ttyUSB3 write_flash -fm dio -fs detect -ff 20m \
    0x00000 boot_v1.6.bin 0x01000 user1.bin \
    0x3fc000 esp_init_data_default.bin 0x7e000 blank.bin \
    0x3fe000 blank.bin 


By default the esp-link firmware creates an AP which you connect to and then browse the x.x.x.1 IP address's site, which is configuration for the system.