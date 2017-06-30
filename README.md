# Space Sensei (せんせい)

*Firmware which automatically pairs Bosch SensorTec BNO055 library to any number of sensors and shuttles quaternions to a host computer.*

![Learn from the sensei.](https://68.media.tumblr.com/avatar_ef804b42fa43_128.png)

## Firmware setup

### Serial settings

38400 baud, 8N1

## TLV Packets

TLV (Type-Length-Value-Checksum) is used to minimize data required to send quaternions to host computer.

### Format

* Type - 1 byte opcode (e.g. 0x0C)
* Length - 1 byte unsigned int length of value field.
	* Note - Entire packet is always 'length + 3'
* Value - Raw bytearray.
* Checksum - 1 byte. Validates data integrity.https://68.media.tumblr.com/avatar_ef804b42fa43_128.png
	* Type + Length + [Value1 + Value2 + ValueN] + Checksum = 0x00

### Notes

* Not using preamble/postamble because timeout takes care of synchronization.
	* Timeout is ~30ms. So if comms go down, wait a while and try again.
* Not parsed without length byte.
	* Minimum packet length is 3 bytes.

### Opcodes

Even opcodes are master->mcu. Odd ones are mcu->master.

* 0x02: Heartbeat request (ping)
	* No Value field.
	* Packet: 0x0200FE
* 0x03: Heartbeat response (pong)
	* No Value field.
	* Packet: 0x0300FD
* 0x04: Request System State
	* No Value field.
	* Packet: 0x0400FC
* 0x05: Return System State
	* Value field is 1-2 uint8. 
	* See System state section below.
* 0x06: Request BNO count
	* No Value field.
	* Packet: 0x0600FA
* 0x07: Return BNO count
	* Value field is one uint8
* 0x08: Request BNO Calibration Status
	* Value field is uint8 index of BNO. Must be less than BNO count.
* 0x09: Return BNO Calibration Status
	* Value field is 4 uint8.
	* In order: accel, gyro, mag, sys.
	* Page 48 of datasheet for more.
* 0x0A: Request BNO Quaternion
	* Value field is BNO index.
* 0x0B: Return BNO Quaternion
	* Value field is quaternion. 4 bytes.
* 0xAA: Generic system error
* 0xAB: Generic system error
* 0xAC: Checksum error
* 0xAD: Checksum error
* 0xAE: Timeout with partial packet
* 0xAF: Timeout with partial packet

### System state

Sent as a uint8. Constructed such that any nonzero number indicates a problem (except bootup, which just means wait a bit).

* 0 - Everything's good
* 1 - Bootup (everything's still good)
* 2 - No BNOs detected during scan. Connect them and reboot system.
* 3 - Error initializing BNO. Cycle power to system. Should send erroneous BNO #.

## BNO055

We're using Bosch's BNO055 9-axis fusion IMU. It kinda sucks, and looks like this:

![From Adafruit's website](https://cdn-learn.adafruit.com/assets/assets/000/024/666/medium800/sensors_pinout.jpg?1429726694)

It has a [datasheet](doc/datasheet.pdf), and [C microcontroller library](https://github.com/BoschSensortec/BNO055_driver), with a [3rd party guide](doc/IntegrateBNO055.pdf) to help you integrate the library to your project.

### Bootup

On boot the BNO automatically falls into calibration mode. The space sensei firmware will wait until the minimum (maybe) calibration is met. Then it will proceed to normal operation.

On the debugging hardware, an LED is yellow during calibration, then switches to green for normal operation.

For NDEF mode (which we're using) the most basic calibration required is magnotometer (rotate sensor 360 like a compass).

### Calibration

Also listed in BNO datasheet page 47. Reiterated here for convenience.

* Mag
	* Rotate sensor in a circle like a compass.
* Gyro
	* Place sensor in a stable position for a few seconds.
* Accel
	* Place the device in 6 different stable positions for a period of few seconds to allow the
accelerometer to calibrate.
	* Make sure that there is slow movement between 2 stable positions
	* The 6 stable positions could be in any direction, but make sure that the device is lying at
least once perpendicular to the x, y and z axis.

Calibration status (as returned by command) is 0-3 for each with 0 being uncalibrated and 3 being nicely calibrated.

## Dev Notes

* 3 BNOs + TCA mux is right at the edge of the max power draw for my ftdi cable..
* Dupont/header connections make i2c unreliable. For implementation either solder connections or remove any stress/movement in wire connections.
* Firmware startup has a ~1 second delay which allows BNOs to boot. Is mentioned in the BNO datasheet.

## To Do  

Explain checksumming

Add quaternion member byte order in return tlv

