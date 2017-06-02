#include "linked-list.h"
#include <util/delay.h>
#include "avr-i2c.h"

struct bnoList_t * root = NULL;

int8_t i2c_read_wrangler(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
	return(i2c_readReg(dev_addr, reg_addr, reg_data, (uint16_t)count));
}

int8_t i2c_write_wrangler(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
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

struct bnoList_t * bnoList_append(uint8_t tca_addr, uint8_t tca_index, uint8_t bno_addr)
{
	struct bnoList_t * tmp;
	struct bnoList_t * newBno = (struct bnoList_t *) malloc(sizeof(struct bnoList_t));
	newBno->bnoPtr = (struct bno055_t *) malloc(sizeof(struct bno055_t));

	newBno->bnoPtr->bus_read = i2c_read_wrangler;
	newBno->bnoPtr->bus_write = i2c_write_wrangler;
	newBno->bnoPtr->delay_msec = delay_ms_wrangler;
	newBno->bnoPtr->dev_addr = (bno_addr);

	newBno->nextPtr = NULL;
	newBno->tca_addr = tca_addr;
	newBno->tca_index = tca_index;

	if (root == NULL)
	{
		root = newBno;
		return newBno;
	}

	tmp = root;
	while (tmp->nextPtr != NULL)
	{
		tmp = tmp->nextPtr;
	}
	tmp->nextPtr = newBno;

	return newBno;
}












