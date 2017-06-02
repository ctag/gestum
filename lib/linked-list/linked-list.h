
#include "bno055.h"

struct bnoList_t {
	struct bno055_t * bnoPtr; // Pointer to bno struct
	uint8_t tca_addr; // i2c address to TCA9548A mux
	uint8_t tca_index; // bno/mux index on TCA
	struct bnoList_t * nextPtr; // ptr to next linked item
};

extern struct bnoList_t * root;

struct bnoList_t * bnoList_append(uint8_t tca_addr, uint8_t tca_index, uint8_t bno_addr);
