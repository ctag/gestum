# ESP Link

Using esp-link firmware on Wemos D1 mini board (esp8266). Also works on ESP-01 (smaller board).

![They're eccentric and I don't like them, just like Tingle here.](https://vignette2.wikia.nocookie.net/zelda/images/6/6f/Tingle_%28Super_Smash_Bros._Brawl%29.png/revision/latest?cb=20100226195556)

## Devices

We're using two types of ESP8266 boards: [D1 Mini by WeMos](https://wiki.wemos.cc/products:d1:d1_mini) and [ESP-01 by AI Thinker](http://ecksteinimg.de/Datasheet/Ai-thinker%20ESP-01%20EN.pdf).

WeMos D1 is good for debugging and testing, but I'd like a final product to have the esp-01 because it's much smaller.

Command to check flash memory size:

	esptool --port /dev/ttyUSB3 --baud 115200 flash_id

### D1 Mini

The D1 mini has 8MB of flash, runs at 80MHz, costs less than $10, and is pretty big. Can be programmed over built-in usb port. Accepts 3V3 or 5V VCC.

![Image of D1 Mini](https://wiki.wemos.cc/_media/products:d1:d1_mini_v2.3.0_1_16x9.jpg)

### ESP-01

The esp-01 has 1MB of flash, runs at 40MHz, costs less than $5 and is pretty small. Can be programmed with an [external FTDI adapter](http://www.14core.com/wiring-and-upgrading-the-esp8266-firmware-via-ttl-uart-bridge/) and a sacrifice to the gods of poor PCB design. Accepts only 3V3 officially... but will work with 5V and get scalding hot.

The ESP-01 has two onboard LEDs, red and blue. Red is connected to power, blue is connected to the board's TX.

ESP-link adds an external green LED for wireless status. It is active-low; 3.3V draw.

![Image of esp-01](https://cdn.instructables.com/FE9/58ZS/IJX7FON7/FE958ZSIJX7FON7.MEDIUM.jpg)

## Installation

From the [esp-link flashing guide](https://github.com/jeelabs/esp-link/blob/master/FLASHING.md), download release archive and extract it.

Command to flash D1 mini:

    esptool --chip esp8266 --port /dev/ttyUSB3 \
    write_flash -fm dio -fs detect -ff 20m \
    0x00000 boot_v1.6.bin 0x01000 user1.bin \
    0x3fc000 esp_init_data_default.bin 0x7e000 blank.bin \
    0x3fe000 blank.bin 

This command is a mash-up of the [instructions for flashing esp-link](https://github.com/jeelabs/esp-link/blob/master/FLASHING.md) and Wemos's d1 mini [flashing guide](https://wiki.wemos.cc/tutorials:get_started:revert_to_at_firmware).

Default AP ssid: `ESP_XXXXXX`

Command to flash ESP-01:

	esptool --chip esp8266 --port /dev/ttyUSB3 \
	write_flash -fm dio -fs detect -ff 40m \
	0x00000 boot_v1.6.bin 0x01000 user1.bin \
	0xfc000 esp_init_data_default.bin \
	0xfe000 blank.bin

Default AP ssid: `AI-THINKER_XXXXXX`

By default the esp-link firmware creates an AP which you connect to and then browse the 192.168.4.1 IP address's site, which is configuration for the system.

## Configuration

Connect to the `ESP-XXXXX` network. You should receive a DHCP address in 192.168.4.0/24.

The ESP runs mDNS, so you can connect to the ESP as either `esp-link` hostname or `192.168.4.1` IP address.

Telnet is primarily on port 23. There is a 5 minute timeout for inactivity, after which you need to reconnect.

I have debug logging turned off to keep junk from being sent to the microcontroller.

## Serial Connection

To connect the esp's telnet to a virtual serial port:

    sudo socat TCP4:192.168.4.1:23 PTY,link=/dev/ttyS0

Then open `/dev/ttyS0` with root.

If your machine uses generic DNS, then you can replace the above IP with the esp's mDNS hostname: `esp-link`

Reference:

http://technostuff.blogspot.com/2008/10/some-useful-socat-commands.html

Apparently you can also just use normal TCP, but I haven' tested it yet.

## Testing

With a simple loopback test (FTDI cable wired to the d1 mini) the latency from serial to telnet was imperceptible. Latency from telnet to serial was ~300ms.

## Dev Notes

esp-link will close the telnet connection after 5 minutes of inactivity. Be warned, software needs to be able to account for this and reconnect.



