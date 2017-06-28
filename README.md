# Space Sensei (せんせい)

*Firmware which automatically pairs Bosch SensorTec BNO055 library to any number of sensors and shuttles quaternions to a host computer.*

![Learn from the sensei.](https://68.media.tumblr.com/avatar_ef804b42fa43_128.png)

## BNO055

We're using Bosch's BNO055 9-axis fusion IMU. It kinda sucks, and looks like this:

![From Adafruit's website](https://cdn-learn.adafruit.com/assets/assets/000/024/666/medium800/sensors_pinout.jpg?1429726694)

It has a [datasheet](doc/datasheet.pdf), and [C microcontroller library](https://github.com/BoschSensortec/BNO055_driver), with a [3rd party guide](doc/IntegrateBNO055.pdf) to help you integrate the library to your project.

## TLV Packets

TLV (Type-Length-Value-Checksum) is used to minimize data required to send quaternions to host computer.

### Format

* Type - 1 byte opcode (e.g. 0x0C)
* Length - 1 byte unsigned int length of value field.
	* Note - Entire packet is always 'length + 3'
* Value - Raw bytearray.
* Checksum - Validates data integrity.https://68.media.tumblr.com/avatar_ef804b42fa43_128.png
	* Type + Length + Value1 + ValueN + Checksum = 0x00

### Notes

* Not using preamble/postamble because timeout takes care of synchronization.
* Not parsed without length byte.
	* Minimum packet length is 3 bytes.

### Opcodes

Even opcodes are master->mcu. Odd ones are mcu->master.

* 0x02: h


## Dev Notes

* 3 BNOs + TCA mux is right at the edge of the max power draw for my ftdi cable..
* Dupont/header connections make i2c unreliable. For implementation either solder connections or remove any stress/movement in wire connections.
* Firmware startup has a ~1 second delay which allows BNOs to boot. Is mentioned in the BNO datasheet.


