


Checksum

XOR'd to create 0x00;

opcodes:


## Notes

* 3 BNOs + TCA mux is right at the edge of the max power draw for my ftdi cable..
* Dupont/header connections make i2c unreliable. For implementation either solder connections or remove any stress/movement in wire connections.

TLV:
* Not using preamble/postamble because timeout takes care of synchronization.
* Not parsed without length byte.
	* Minimum packet length is 3 bytes.
* 

