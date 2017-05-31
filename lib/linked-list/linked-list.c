#include "linked-list.h"

int8_t i2c_wrangler_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
	return(i2c_readReg(dev_addr, reg_addr, reg_data, (uint16_t)count));
}

int8_t i2c_wrangler_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
	return(i2c_writeReg(dev_addr, reg_addr, reg_data, (uint16_t)count));
}

void delay_ms_wrangler(u32 msek)
{
	for (uint32_t loop = 0; loop < msek; loop++)
	{
		_delay_ms(1);
	}
}

struct bnoList_t * bnoList_append(struct bnoList_t * root, struct bno055_t * bno055, uint8_t tca_addr)
{
	struct bnoList_t * tmp = root;
	struct bnoList_t * newBno = malloc(sizeof(struct bnoList_t));

	bno055->bus_read = i2c_wrangler_read;
	bno055->bus_write = i2c_wrangler_write;
	bno055->delay_msec = delay_ms_wrangler; //_delay_ms;
	bno055->dev_addr = (BNO055_I2C_ADDR1<<1);

	newBno->nextPtr = NULL;
	newBno->tca_addr = tca_addr;
	newBno->bnoPtr = bno055;

	if (tmp == NULL)
	{
		tmp = newBno;
		return newBno;
	}

	while (tmp->nextPtr != NULL)
	{
		tmp = tmp->nextPtr;
	}
	tmp->nextPtr = newBno;

	return newBno;
}
