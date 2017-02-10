/**
 * SPACE SENSEI - Let's learn about IMUs
 * Christopher Bero <csb0019@uah.edu>
 */

#include "main.h"

struct bno055_t myBNO;
uint8_t accel_calib_status = 0;
uint8_t gyro_calib_status = 0;
uint8_t mag_calib_status = 0;
uint8_t sys_calib_status = 0;
struct bno055_accel_double_t d_accel_xyz;
struct bno055_linear_accel_double_t d_linear_accel_xyz;
double vel_x = 0;
double pos_x = 0;
unsigned int output_counter = 0;
char float_str[12];

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
	int8_t err = 0;
	wdt_disable();
	cli();
	usart_init();
	init_printf(NULL, usart_putchar);
	printf("Printf init.\n");
	i2c_init();
	printf("i2c init.\n");
	// Practicing bno055 library interaction
	myBNO.bus_read = i2c_wrangler_read;
	myBNO.bus_write = i2c_wrangler_write;
	myBNO.delay_msec = delay_ms_wrangler; //_delay_ms;
	myBNO.dev_addr = (BNO055_I2C_ADDR1<<1);
	_delay_ms(1000);

//	uint8_t dat;
//	err=i2c_readReg(0x28<<1, 0x39, &dat, 1);
//	printf("0x%02X, err: 0x%02X\n", dat, err);

	err=bno055_init(&myBNO);
	printf("bno init: %d\n", err);
	err=bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
	printf("bno power mode: %d\n", err);
	err=bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
	printf("Set bno operating mode: %d\n", err);

//	struct bno055_euler_float_t eulerData;
//	bno055_convert_float_euler_hpr_deg(&eulerData);
}

/* Loop
 * Does all of the main tasks.
 */
void loop()
{
	int8_t err = 0;
	output_counter++;
	err += bno055_get_accel_calib_stat(&accel_calib_status);
	err+= bno055_get_mag_calib_stat(&mag_calib_status);
	err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
	err+=bno055_get_sys_calib_stat(&sys_calib_status);
//	printf("Calibration:\n\tAccel: 0x%02X\n\tMag: 0x%02X\n\tGyro: 0x%02X\n\tSys: 0x%02X\n\tError: %d\n", accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status, err);
	if (accel_calib_status == 3 && gyro_calib_status == 3 && mag_calib_status == 3 && err == 0)
	{
//		bno055_convert_double_accel_xyz_msq(&d_accel_xyz);
//		printf("Accel: %d, %d, %d\n", (int)d_accel_xyz.x, (int)d_accel_xyz.y, (int)d_accel_xyz.z);
		bno055_convert_double_linear_accel_xyz_msq(&d_linear_accel_xyz);
//		printf("Linear Accel: %d, %d, %d\n", (int)d_linear_accel_xyz.x, (int)d_linear_accel_xyz.y, (int)d_linear_accel_xyz.z);
		vel_x += (d_linear_accel_xyz.x * 0.01);
		pos_x += (vel_x * 0.01);
		if (output_counter%16 == 0)
		{
			dtostrf(d_linear_accel_xyz.x, 10, 8, float_str);
			printf("%u\t%s\t", output_counter, float_str);
			dtostrf(vel_x, 10, 8, float_str);
			printf("%s\t", float_str);
			dtostrf(pos_x, 10, 8, float_str);
			printf("%s\n", float_str);
		}
	}
	_delay_ms(10);
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
