/**
 * SPACE SENSEI - Let's learn about IMUs
 * Christopher Bero <csb0019@uah.edu>
 */

#include "main.h"

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

/* Init
 * This function is kept over from the Arduino library for no reason.
 * I have arbitrarily decided that it will be used to initialize
 * low level hardware.
 */
void init ()
{
        // Pin setup
        DDRB = (1 << PB1); // Set as output
        DDRD = (1 << PD6); // Set as output
}

/**
 * setup() - a poem
 * GLaDOS is sleeping \
 * The testing floors are quiet \
 * But where are we now?
 */
void setup()
{
	i2c_init();
	// Practicing bno055 library interaction
	struct bno055_t myBNO;
	myBNO.bus_read = i2c_wrangler_read;
	myBNO.bus_read = i2c_wrangler_write;
	myBNO.delay_msec = delay_ms_wrangler; //_delay_ms;
	myBNO.dev_addr = BNO055_I2C_ADDR1;
	bno055_init(&myBNO);
	bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
	struct bno055_euler_float_t eulerData;
//	bno055_convert_float_euler_hpr_deg(&eulerData);
	uint8_t accel_calib_status = 0;
	bno055_get_accel_calib_stat(&accel_calib_status);
//	printf("Here: 0x%02X\n", accel_calib_status);
}

/* Loop
 * Does all of the main tasks.
 */
void loop()
{
	//
}

/* Main
 * This function is used to override main() provided by the Arduino library.
 * It simply structures the other high-level functions.
 */
int main(void)
{
        init();

    setup();

    for (;;)
    {
        loop();
    }

    return 0;
}
