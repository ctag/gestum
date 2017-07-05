# ESP Link

Using esp-link firmware on Wemos D1 mini board (esp8266). Also works on ESP-01 (smaller board).

## Devices

We're using two types of ESP8266 boards: [D1 Mini by WeMos](https://wiki.wemos.cc/products:d1:d1_mini) and [ESP-01 by AI Thinker](http://ecksteinimg.de/Datasheet/Ai-thinker%20ESP-01%20EN.pdf).

D1 Mini:

![Image of D1 Mini](https://wiki.wemos.cc/_media/products:d1:d1_mini_v2.3.0_1_16x9.jpg)

ESP-01:

![Image of esp-01](https://cdn.instructables.com/FE9/58ZS/IJX7FON7/FE958ZSIJX7FON7.MEDIUM.jpg)

## Installation

Download release archive and extract it.

Command to flash D1 mini:

    esptool --chip esp8266 --port /dev/ttyUSB3 \
    write_flash -fm dio -fs detect -ff 20m \
    0x00000 boot_v1.6.bin 0x01000 user1.bin \
    0x3fc000 esp_init_data_default.bin 0x7e000 blank.bin \
    0x3fe000 blank.bin 

Command to flash ESP-01:

	esptool --chip esp8266 --port /dev/ttyUSB3 \
	write_flash -fm dio -fs detect -ff 40m \
	0x00000 boot_v1.6.bin 0x01000 user1.bin \
	0xfc000 esp_init_data_default.bin \
	0xfe000 blank.bin

This command is a mash-up of the [instructions for flashing esp-link](https://github.com/jeelabs/esp-link/blob/master/FLASHING.md) and Wemos's d1 mini [flashing guide](https://wiki.wemos.cc/tutorials:get_started:revert_to_at_firmware).

By default the esp-link firmware creates an AP which you connect to and then browse the x.x.x.1 IP address's site, which is configuration for the system.

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



