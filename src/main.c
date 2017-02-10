/**
 * SPACE SENSEI - Let's learn about IMUs
 * Christopher Bero <csb0019@uah.edu>
 */

#include "main.h"

#define FILTER_WINDOW 10

struct bno055_t myBNO;
uint8_t accel_calib_status = 0;
uint8_t gyro_calib_status = 0;
uint8_t mag_calib_status = 0;
uint8_t sys_calib_status = 0;
struct bno055_accel_double_t d_accel_xyz;
struct bno055_linear_accel_double_t accel_xyz_input = {0};
struct bno055_linear_accel_double_t accel_xyz[2] = {0};
struct bno055_linear_accel_double_t veloc_xyz[2] = {0};
struct bno055_linear_accel_double_t posit_xyz[2] = {0};
unsigned int read_counter = 0;
char float_str[12];
uint8_t proc_flag = 0;
uint8_t calibrated = 0;

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
	DDRB = (1 << PB1) | (1 << PB2); // Set as output
	DDRD = (1 << PD6); // Set as output
	// Timer0 setup
	TCCR0B = (1 << CS02) | (1 << CS00);
	TCNT0 = 0;
	OCR0B = 156;
	TIMSK0 = (1<< OCIE0B);
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
	cli();
	wdt_disable();
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

	err=bno055_init(&myBNO);
	printf("bno init: %d\n", err);
	err=bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
	printf("bno power mode: %d\n", err);
	err=bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
	printf("Set bno operating mode: %d\n", err);

	_delay_ms(650); // Some sort of POR setup time..?

	do
	{
		err = 0;
		err+=bno055_get_accel_calib_stat(&accel_calib_status);
		err+=bno055_get_mag_calib_stat(&mag_calib_status);
		err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
		err+=bno055_get_sys_calib_stat(&sys_calib_status);
		printf("Waiting on calibration: %d, %d, %d, %d....\n", accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status);
		_delay_ms(500);
	} while (accel_calib_status != 3 || gyro_calib_status != 3 || mag_calib_status != 3 || err != 0);

	sei();
}

/* Loop
 * Does all of the main tasks.
 */
void loop()
{
//	printf("Calibration:\n\tAccel: 0x%02X\n\tMag: 0x%02X\n\tGyro: 0x%02X\n\tSys: 0x%02X\n\tError: %d\n", accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status, err);
//	printf("Linear Accel: %d, %d, %d\n", (int)d_linear_accel_xyz.x, (int)d_linear_accel_xyz.y, (int)d_linear_accel_xyz.z);

	if (proc_flag)
	{
		if (calibrated)
		{
			if (read_counter%FILTER_WINDOW == 0)
			{
				accel_xyz[1].x = accel_xyz[1].x/FILTER_WINDOW;
				accel_xyz[1].y = accel_xyz[1].y/FILTER_WINDOW;
				accel_xyz[1].z = accel_xyz[1].z/FILTER_WINDOW;

				// Calculate velocity (single integration)
				veloc_xyz[1].x = veloc_xyz[0].x + (((accel_xyz[0].x + accel_xyz[1].x)/2.0)*(FILTER_WINDOW*0.01));
				// Calculate position (double integration)
				posit_xyz[1].x = posit_xyz[0].x + (((veloc_xyz[0].x + veloc_xyz[1].x)/2.0)*(FILTER_WINDOW*0.01));

				// Output current data
				dtostrf(accel_xyz[1].x, 10, 8, float_str);
				printf("%u\t%s\t", read_counter, float_str);
				dtostrf(veloc_xyz[1].x, 10, 8, float_str);
				printf("%s\t", float_str);
				dtostrf(posit_xyz[1].x, 10, 8, float_str);
				printf("%s\n", float_str);

				// Rotate back
				accel_xyz[0] = accel_xyz[1];
				veloc_xyz[0] = veloc_xyz[1];
				posit_xyz[0] = posit_xyz[1];

				accel_xyz[1].x = 0;
				accel_xyz[1].y = 0;
				accel_xyz[1].z = 0;
			}
			else
			{
				// Build acceleration filter
				accel_xyz[1].x = accel_xyz[1].x + accel_xyz_input.x;
				accel_xyz[1].y = accel_xyz[1].y + accel_xyz_input.y;
				accel_xyz[1].z = accel_xyz[1].z + accel_xyz_input.z;
			}
			PORTB = (1<<PB1);
		}
		proc_flag = 0;
	}
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

// Interrupt vector
ISR(TIMER0_COMPB_vect)
{
	int8_t err = 0;
	proc_flag = 1;
	PORTB = 0x00;
	read_counter++;

	err+=bno055_get_accel_calib_stat(&accel_calib_status);
	err+=bno055_get_mag_calib_stat(&mag_calib_status);
	err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
	err+=bno055_get_sys_calib_stat(&sys_calib_status);

	if (accel_calib_status == 3 && gyro_calib_status == 3 && mag_calib_status == 3 && err == 0)
	{
		calibrated = 1;
		bno055_convert_double_linear_accel_xyz_msq(&accel_xyz_input);
	}
	else
	{
		calibrated = 0;
	}
}
