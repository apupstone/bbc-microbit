#include "nrf51.h"
#include "FreeRTOS.h"
#include "task.h"
#include "I2CMaster.h"
#include "UART.h"


#define COL1 4
#define COL2 5
#define COL3 6
#define COL4 7
#define COL5 8
#define COL6 9
#define COL7 10
#define COL8 11
#define COL9 12


#define ROW1 13
#define ROW2 14
#define ROW3 15


static uint8_t x;


void I2CRxCallback(I2CRxData rx_buffer)
{
	x = rx_buffer.data[0];
}


void I2CTxCallback()
{
	i2c0().read(0x1D, 1, I2CRxCallback);
}


void task_1(void *pvParameters)
{
	volatile bool led1_status = false;

	//uint8_t whoami_reg = 0x0D;
	//uint8_t whoami = 0;
	
	uint8_t data[64] = {0};
	
	for(;;)
	{
		if (led1_status)
		{
			NRF_GPIO->OUTCLR = (1 << ROW1);
			led1_status = false;
		}
		else
		{
			NRF_GPIO->OUTSET = (1 << ROW1);
			led1_status = true;
		}
		
		vTaskDelay(pdMS_TO_TICKS(500));
		
		uint16_t rx_length = uart0().read(&data[0], 64);
		uart0().write(&data[0], rx_length);
	}
}


void task_2(void *pvParameters)
{
	volatile bool led2_status = false;
	
	uint8_t x_reg = 0x01;
	uint8_t ctrl_reg_1[2] = {0x2A, 0x01};

	//uint16_t hello_count = 0;
	//char hello_world[30] = {0};
	
	// Enable ACTIVE mode of accelerometer
	i2c0().write(0x1D, &ctrl_reg_1[0], 2, nullptr, false);
	
	for(;;)
	{
		if (led2_status)
		{
			NRF_GPIO->OUTCLR = (1 << ROW2);
			if (!i2c0().busy())
			{
				i2c0().write(0x1D, &x_reg, 1, I2CTxCallback, true);
			}
			led2_status = false;
		}
		else
		{
			NRF_GPIO->OUTSET = (1 << ROW2);
			led2_status = true;
		}
		
		//sprintf(&hello_world[0], "Hello, World! %d\r\n", x);
		//uart0().write((uint8_t *)&hello_world[0], sizeof(hello_world));
		
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}


int main()
{
	// Set all LED rows and columns as outputs
	NRF_GPIO->DIRSET = (1 << COL1) | (1 << COL2) | (1 << COL3) | (1 << COL4) | (1 << COL5) | (1 << COL6) | (1 << COL7) | (1 << COL8) | (1 << COL9) | (1 << ROW1) | (1 << ROW2) | (1 << ROW3);
	
	// Clear column 1
	NRF_GPIO->OUTCLR = (1 << COL1);
	
	// Set all other columns
	NRF_GPIO->OUTSET = (1 << COL2) | (1 << COL3) | (1 << COL4) | (1 << COL5) | (1 << COL6) | (1 << COL7) | (1 << COL8) | (1 << COL9);

	// Clear all rows
	NRF_GPIO->OUTCLR = (1 << ROW1) | (1 << ROW2) | (1 << ROW3);
	
	xTaskCreate(task_1, "Task 1", 200, NULL, 2, NULL);
	xTaskCreate(task_2, "Task 2", 200, NULL, 1, NULL);
	
	vTaskStartScheduler();

    for(;;);
}