# Space Sensei (せんせい)

*Firmware which automatically pairs Bosch SensorTec BNO055 library to any number of sensors and shuttles quaternions to a host computer.*

## TLV Packets

TLV (Type-Length-Value-Checksum) is used to minimize data required to send quaternions to host computer.

### Format

* Type - 1 byte opcode (e.g. 0x0C)
* Length - 1 byte unsigned int length of value field.
	* Note - Entire packet is always 'length + 3'
* Value - Raw bytearray.
* Checksum - Validates data integrity.
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



