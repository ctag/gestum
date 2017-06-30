# ESP Link

Using esp-link firmware on Wemos D1 mini board (esp8266).

## Installation

Download release archive and extract it.

Command to flash D1 mini:

    esptool --chip esp8266 --port /dev/ttyUSB3 \
    write_flash -fm dio -fs detect -ff 20m \
    0x00000 boot_v1.6.bin 0x01000 user1.bin \
    0x3fc000 esp_init_data_default.bin 0x7e000 blank.bin \
    0x3fe000 blank.bin 

This command is a mash-up of the [instructions for flashing esp-link](https://github.com/jeelabs/esp-link/blob/master/FLASHING.md) and Wemos's d1 mini [flashing guide](https://wiki.wemos.cc/tutorials:get_started:revert_to_at_firmware).

By default the esp-link firmware creates an AP which you connect to and then browse the x.x.x.1 IP address's site, which is configuration for the system.

## Configuration

I'm using 'swapped' uart on the esp in order to avoid the boot-up spray of useless serial output.

I have debug logging turned off to keep junk from being sent to the microcontroller.

## Connection

To connect the esp's telnet to a virtual serial port:

    sudo socat TCP4:192.168.4.1:23 PTY,link=/dev/ttyS0

Then open `/dev/ttyS0` with root.

If your machine uses generic DNS, then you can replace the above IP with the esp's mDNS hostname: `esp-link`

Reference:

http://technostuff.blogspot.com/2008/10/some-useful-socat-commands.html


## Testing

With a simple loopback test (FTDI cable wired to the d1 mini) the latency from serial to telnet was imperceptible. Latency from telnet to serial was ~300ms.

## Dev Notes

esp-link will close the telnet connection after 5 minutes of inactivity. Be warned, software needs to be able to account for this and reconnect.



