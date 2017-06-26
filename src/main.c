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

struct bno055_t myBNO;

uint8_t accel_calib_status = 0;
uint8_t gyro_calib_status = 0;
uint8_t mag_calib_status = 0;
uint8_t sys_calib_status = 0;

struct bno055_quaternion_t quat = {0};

unsigned int read_counter = 0;
char float_str[12];
uint8_t proc_flag = 0;
uint8_t calibrated = 0;
uint8_t interrupt_count = 0;

unsigned char input_buffer[16] = {0};
uint8_t input_index = 0;
uint8_t input_timeout = 0;

void send_packet(uint8_t opcode, uint8_t len, uint8_t * data);

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
	// LED pin setup
	DDRB = (1<<PIN1) | (1<<PIN2) | (1<<PIN3);
	led_enable();
	led_set(0, 0, 0);
}

void setup()
{
	int8_t err = 0;
	cli();
	wdt_disable();

	led_set(5, 5, 0);

	usart_init();
	init_printf(NULL, usart_putchar);
//	printf("Printf init.\n");
	i2c_init();
//	printf("i2c init.\n");

	_delay_ms(1000); // POR setup time from datasheet

	led_set(25, 25, 0);

	for (uint8_t tca_i = 0; tca_i < tca_addr_list_len; tca_i++)
	{
//		printf("Checking TCA address 0x%02X\n", tca_addr_list[tca_i]);
		err=i2c_start(tca_addr_list[tca_i], I2C_READ);
		i2c_stop();
		if (err)
		{
//			printf("\tNo TCA found.\n");
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
					bnoList_append(tca_addr_list[tca_i], mux_i, bno_addr_list[bno_i]);
//					printf("Found BNO 0x%02X at index %d on TCA 0x%02X\n", newBno->bnoPtr->dev_addr, newBno->tca_index, newBno->tca_addr);
				}
			}
		}
	}

	if (!root)
	{
		led_set(25, 0, 0);
//		printf("No BNO055's detected. Connect them and reboot system.\n");
		while(1);
	}
	else
	{
		struct bnoList_t * tmpBno = root;
		while (tmpBno)
		{
//			printf("Bno: %d; 0x%02X addr at TCA 0x%02X\n", tmpBno->tca_index, tmpBno->bnoPtr->dev_addr, tmpBno->tca_addr);
			tcaselect(tmpBno->tca_addr, tmpBno->tca_index);
			err=bno055_init(tmpBno->bnoPtr);
//			printf("bno init: %d\n", err);
			err+=bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
//			printf("bno power mode: %d\n", err);
			err+=bno055_set_operation_mode(BNO055_OPERATION_MODE_NDOF);
//			printf("Set bno operating mode: %d\n", err);
			if (err)
			{
				led_set(25, 0, 0);
//				printf("\tError initializing BNO %d... Cycle power to system.\n", tmpBno->tca_index);
				while(1);
			}

			do
			{
//				led_set(25, 25, 0);
//				printf("\nCalibrating sensor %d:\n", tmpBno->tca_index);
				err=bno055_get_accel_calib_stat(&accel_calib_status);
//				printf("\tAccel: %d [%d]\n", accel_calib_status, err);
				err=bno055_get_mag_calib_stat(&mag_calib_status);
//				printf("\tMag: %d [%d]\n", mag_calib_status, err);
				err=bno055_get_gyro_calib_stat(&gyro_calib_status);
//				printf("\tGyro: %d [%d]\n", gyro_calib_status, err);
				err=bno055_get_sys_calib_stat(&sys_calib_status);
//				printf("\tSys: %d [%d]\n", sys_calib_status, err);
//				_delay_ms(500);
			} while (sys_calib_status != 3 || err != 0);

			tmpBno = tmpBno->nextPtr;
		}
	}

	sei();
	led_set(0, 25, 0);
}

uint8_t get_quaternion()
{
	uint8_t err = 0;
//	err+=bno055_get_accel_calib_stat(&accel_calib_status);
//	err+=bno055_get_mag_calib_stat(&mag_calib_status);
	err+=bno055_get_gyro_calib_stat(&gyro_calib_status);
	err+=bno055_get_sys_calib_stat(&sys_calib_status);

	if (gyro_calib_status >= 2 && err == 0)
	{
		bno055_read_quaternion_wxyz(&quat);
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
	for (uint8_t index = 1; index <= len; ++index)
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

void send_packet(uint8_t opcode, uint8_t len, uint8_t * data)
{
	uint8_t checksum = 0xff;
	checksum += opcode;
	usart_putchar(NULL, opcode);
	checksum += len;
	usart_putchar(NULL, len);
	for (uint8_t index = 0; index < len; index++)
	{
		checksum += data[index];
		usart_putchar(NULL, data[index]);
	}
	usart_putchar(NULL, ~checksum);
}

void send_num_bnos()
{
	uint8_t count = 0;
	struct bnoList_t * tmpBno = root;
	while (tmpBno)
	{
		count++;
		tmpBno = tmpBno->nextPtr;
	}
	printf("%d\n", count);
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
	if (input_index >= 2) // minimum length: 3
	{
		printf("len: %d\n", input_buffer[1]);
		if (input_index == (input_buffer[1] + 2)) // data len + type/len/chk bytes
		{
			printf("Passed len check.\n");
			// Check checksum, run command
			if (checksum_valid() == 0)
			{
				printf("passed checksum validation.\n");
				switch (input_buffer[0])
				{
				case 0xC0: // Check number of devices detected
					send_num_bnos();
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
	proc_input();
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
        _delay_ms(50);
    }

    return 0;
}




































