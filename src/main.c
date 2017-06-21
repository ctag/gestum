/**
 * SPACE SENSEI - Let's learn about IMUs
 * Christopher Bero <csb0019@uah.edu>
 */

#include "main.h"

#define TCAADDR 0x70
#define INPUT_TIMEOUT 25

const uint8_t tca_addr_list_len = 8;
const uint8_t tca_addr_list[] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};

const uint8_t bno_addr_list_len = 2;
const uint8_t bno_addr_list[] = {BNO055_I2C_ADDR1, BNO055_I2C_ADDR2};

struct bnoList_t * tmpBno;

struct bno055_t myBNO;

uint8_t accel_calib_status = 0;
uint8_t gyro_calib_status = 0;
uint8_t mag_calib_status = 0;
uint8_t sys_calib_status = 0;
//struct bno055_accel_double_t d_accel_xyz;
//struct bno055_linear_accel_double_t accel_xyz_input = {0};
//struct bno055_linear_accel_double_t accel_xyz[2] = {0};
//struct bno055_linear_accel_double_t veloc_xyz[2] = {0};
//struct bno055_linear_accel_double_t posit_xyz[2] = {0};

struct bno055_euler_float_t euler_hpr = {0};

unsigned int read_counter = 0;
char float_str[12];
uint8_t proc_flag = 0;
uint8_t calibrated = 0;
uint8_t interrupt_count = 0;

char input_buffer[16] = {0};
uint8_t input_index = 0;
uint8_t input_timeout = 0;

uint8_t tcaselect(uint8_t tca_addr, uint8_t mux) {
	uint8_t err = 0;
	if (mux > 7) return 1;

	err = i2c_start(tca_addr, I2C_WRITE);
	err += i2c_write(1 << mux);
	i2c_stop();

	return err;
}

uint8_t led_set(uint8_t red, uint8_t green, uint8_t blue)
{
    OCR1A = red;
    OCR1B = green;
    OCR2A = blue;
    return(0);
}

void led_enable()
{
    // Red   - Timer 1A
    // Green - Timer 1B
    // Blue  - Timer 2A
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << CS10);
    TCCR2A = _BV(COM2A1) | _BV(WGM20);
    TCCR2B = _BV(CS20);
}

void led_disable()
{
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCCR2B = 0x00;
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

	// Timer0B setup
	TCCR0B = (1 << CS02) | (1 << CS00);
	TCNT0 = 0;
	OCR0B = 156; // 10ms polling
	TIMSK0 = (1<< OCIE0B);

	led_enable();
	led_set(0, 0, 0);
}

void setup()
{
	int8_t err = 0;
	cli();
	wdt_disable();

	led_set(25, 25, 0);

	usart_init();
	init_printf(NULL, usart_putchar);
	printf("Printf init.\n");
	i2c_init();
	printf("i2c init.\n");

	_delay_ms(1000); // POR setup time from datasheet

	for (uint8_t tca_i = 0; tca_i < tca_addr_list_len; tca_i++)
	{
		printf("Checking TCA address 0x%02X\n", tca_addr_list[tca_i]);
		err=i2c_start(tca_addr_list[tca_i], I2C_READ);
		i2c_stop();
		if (err)
		{
			printf("\tNo TCA found.\n");
			continue;
		}
		for (uint8_t mux_i = 0; mux_i < 8; mux_i++)
		{
			tcaselect(tca_addr_list[tca_i], mux_i);
			for (uint8_t bno_i = 0; bno_i < bno_addr_list_len; bno_i++)
			{
				err=i2c_start(bno_addr_list[bno_i], I2C_READ);
				i2c_stop();
				if (!err)
				{
					struct bnoList_t * newBno = bnoList_append(tca_addr_list[tca_i], mux_i, bno_addr_list[bno_i]);
					printf("Found BNO 0x%02X at index %d on TCA 0x%02X\n", newBno->bnoPtr->dev_addr, newBno->tca_index, newBno->tca_addr);
				}
			}
		}
	}

	if (!root)
	{
		led_set(25, 0, 0);
		printf("No BNO055's detected. Connect them and reboot system.\n");
		while(1);
	}
	else
	{
		struct bnoList_t * tmpBno = root;
		while (tmpBno)
		{
			printf("Bno: %d; 0x%02X addr at TCA 0x%02X\n", tmpBno->tca_index, tmpBno->bnoPtr->dev_addr, tmpBno->tca_addr);

			tcaselect(tmpBno->tca_addr, tmpBno->tca_index);
			err=bno055_init(tmpBno->bnoPtr);
			printf("bno init: %d\n", err);
			err+=bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
			printf("bno power mode: %d\n", err);
			err+=bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
			printf("Set bno operating mode: %d\n", err);
			if (err)
			{
				printf("\tError initializing BNO %d... Cycle power to system.\n", tmpBno->tca_index);
				while(1);
			}

			do
			{
//				led_set(25, 25, 0);
				printf("\nCalibrating sensor %d:\n", tmpBno->tca_index);
				err=bno055_get_accel_calib_stat(&accel_calib_status);
				printf("\tAccel: %d [%d]\n", accel_calib_status, err);
				err=bno055_get_mag_calib_stat(&mag_calib_status);
				printf("\tMag: %d [%d]\n", mag_calib_status, err);
				err=bno055_get_gyro_calib_stat(&gyro_calib_status);
				printf("\tGyro: %d [%d]\n", gyro_calib_status, err);
				err=bno055_get_sys_calib_stat(&sys_calib_status);
				printf("\tSys: %d [%d]\n", sys_calib_status, err);
				_delay_ms(500);
			} while (sys_calib_status != 3 || err != 0);

			tmpBno = tmpBno->nextPtr;
		}
	}

	while (1);

//struct bnoList_t * newBno = bnoList_append(tca_addr_list[0], 0, bno_addr_list[0]);
//printf("Created bno node.\n");
//
////	for (uint8_t i = 0; i < 3; ++i)
////	{
//
//		err=tcaselect(0x70, 0);
//		if (err) printf("tca select error: 0x%02X\n", err);
//
//		err=bno055_init(newBno->bnoPtr);
//		printf("bno init: %d\n", err);
//		err=bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
//		printf("bno power mode: %d\n", err);
//		err=bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
//		printf("Set bno operating mode: %d\n", err);
////	}
//
//	do
//	{
////		led_set(25, 25, 0);
//		err = 0;
////		err+=bno055_get_accel_calib_stat(&accel_calib_status);
////		err+=bno055_get_mag_calib_stat(&mag_calib_status);
////		err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
//		err+=bno055_get_sys_calib_stat(&sys_calib_status);
//		printf("Calibrating sensor %d:\t%d, %d, %d, %d [%d]\n", 0,
//						accel_calib_status, mag_calib_status,
//						gyro_calib_status, sys_calib_status, err);
//		_delay_ms(500);
//	} while (1||sys_calib_status != 3 || err != 0);
//
//	while(1);

	sei();
	led_set(0, 25, 0);
}

uint8_t get_euler()
{
	uint8_t err = 0;
	err+=bno055_get_accel_calib_stat(&accel_calib_status);
	err+=bno055_get_mag_calib_stat(&mag_calib_status);
	err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
	err+=bno055_get_sys_calib_stat(&sys_calib_status);

	if (sys_calib_status == 3 && err == 0)
	{
		bno055_convert_float_euler_hpr_deg(&euler_hpr);
		return 0;
	}
	else
	{
//		printf("Device uncalibrated.\n");
		return 1;
	}
}

uint8_t checksum_valid()
{
	uint8_t checksum = input_buffer[0];
	uint8_t len = input_buffer[1]+2;
	for (uint8_t index = 1; index < len; ++index)
	{
		checksum += input_buffer[index]; // All bytes XOR'd should = 0x00
	}
	if (checksum == 0x00)
	{
		return 0; // checksum is good
	}
	else
	{
		return 1;
	}
}

void proc_input()
{
	char input_char;
	if(usart_getchar(&input_char))
	{
		input_timeout++;
		if (input_timeout > INPUT_TIMEOUT)
		{
			input_timeout = 0;
			input_index = 0;
		}
		return;
	}
	input_buffer[input_index] = input_char;
	if (input_index > 1)
	{
		if (input_index == (input_buffer[1] + 2))
		{
			// Check checksum, run command
			if (checksum_valid() == 0)
			{
				switch (input_buffer[0])
				{
				case 0xC0: // Check number of devices detected
					break;
				case 0xC1: // Check calibration
					printf("Check calibration\n");
					break;
				case 0xD0: // Device Read
					break;
				case 0xD1: // Device calibration status
					break;
				default:
					printf("Invalid command?\n");
					break;
				}
				input_index = 0;
				return;
			}
		}
	}
	input_index++;
}

/* Loop
 * Does all of the main tasks.
 */
void loop()
{
//	printf("Calibration:\n\tAccel: 0x%02X\n\tMag: 0x%02X\n\tGyro: 0x%02X\n\tSys: 0x%02X\n\tError: %d\n", accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status, err);
//	printf("Linear Accel: %d, %d, %d\n", (int)d_linear_accel_xyz.x, (int)d_linear_accel_xyz.y, (int)d_linear_accel_xyz.z);

//	proc_input();

	if (proc_flag)
	{
		if (calibrated||1)
		{
//			if (read_counter%FILTER_WINDOW == 0)
//			{
//				accel_xyz[1].x = accel_xyz[1].x/FILTER_WINDOW;
//				accel_xyz[1].y = accel_xyz[1].y/FILTER_WINDOW;
//				accel_xyz[1].z = accel_xyz[1].z/FILTER_WINDOW;
//
//				// Calculate velocity (single integration)
//				veloc_xyz[1].x = veloc_xyz[0].x + (((accel_xyz[0].x + accel_xyz[1].x)/2.0)*(FILTER_WINDOW*0.01));
//				// Calculate position (double integration)
//				posit_xyz[1].x = posit_xyz[0].x + (((veloc_xyz[0].x + veloc_xyz[1].x)/2.0)*(FILTER_WINDOW*0.01));
//
//				// Output current data
//				dtostrf(accel_xyz[1].x, 10, 8, float_str);
//				printf("%u\t%s\t", read_counter, float_str);
//				dtostrf(veloc_xyz[1].x, 10, 8, float_str);
//				printf("%s\t", float_str);
//				dtostrf(posit_xyz[1].x, 10, 8, float_str);
//				printf("%s\n", float_str);
//
//				// Rotate back
//				accel_xyz[0] = accel_xyz[1];
//				veloc_xyz[0] = veloc_xyz[1];
//				posit_xyz[0] = posit_xyz[1];
//
//				accel_xyz[1].x = 0;
//				accel_xyz[1].y = 0;
//				accel_xyz[1].z = 0;
//			}
//			else
//			{
//				// Build acceleration filter
//				accel_xyz[1].x = accel_xyz[1].x + accel_xyz_input.x;
//				accel_xyz[1].y = accel_xyz[1].y + accel_xyz_input.y;
//				accel_xyz[1].z = accel_xyz[1].z + accel_xyz_input.z;
//			}
//			PORTB = (1<<PB1);

			// Output current data
			dtostrf(euler_hpr.h, 6, 4, float_str);
			printf("%u\t%s\t", read_counter, float_str);
			dtostrf(euler_hpr.r, 6, 4, float_str);
			printf("%s\t", float_str);
			dtostrf(euler_hpr.p, 6, 4, float_str);
			printf("%s\n", float_str);

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

    tmpBno = root;
    for (;;)
    {
        loop();
    }

    return 0;
}

// Interrupt vector
ISR(TIMER0_COMPB_vect)
{
//	int8_t err = 0;
//
//	interrupt_count++;
//
//	if (interrupt_count < 10)
//	{
//		return;
//	}
//	interrupt_count = 0;
//
	proc_flag = 1;
////	PORTB = 0x00;
//	read_counter++;


}




































