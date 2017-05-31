
#include "bno055.h"

struct bnoList_t {
	struct bno055_t * bnoPtr; // Pointer to bno struct
	uint8_t tca_addr; // i2c address to TCA9548A mux
	struct bnoList_t * nextPtr; // ptr to next linked item
};

struct bnoList_t * bnoList_append(struct bnoList_t * root, struct bno055_t * bno055, uint8_t tca_addr);
