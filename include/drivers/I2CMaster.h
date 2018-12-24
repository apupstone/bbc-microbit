#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include "nrf51.h"
#include "nrf51_bitfields.h"
#include <stdint.h>
#include "CircularBuffer.h"


static const uint8_t I2C_TX_BUFFER_SIZE = 64;
static const uint8_t I2C_RX_BUFFER_SIZE = 64;


enum class I2CFrequency
{
	K100 = 0x01980000,
	K250 = 0x04000000,
	K400 = 0x06680000
};


struct I2CConfig
{
	NRF_TWI_Type* periph;
	I2CFrequency frequency;
	uint32_t pin_scl;
	uint32_t pin_sda;
};


static const I2CConfig i2c0_config = 
{
	NRF_TWI0,
	I2CFrequency::K100,
	0,
	30
};


struct I2CRxData
{
	uint8_t data[I2C_RX_BUFFER_SIZE];
	uint16_t length;
};


class I2CMaster
{
	public:
		I2CMaster(const I2CConfig& conf);
		void write(uint8_t address, const uint8_t* data, uint8_t length, void (* callback)(), bool repeated_start=false);
		void read(uint8_t address, uint8_t length, void (* callback)(I2CRxData));
		void isr();
		bool busy() { return m_busy; }
		
	private:
		const I2CConfig& m_conf;
		bool m_repeated_start;
		
		CircularBuffer<uint8_t> m_tx_buffer;
		CircularBuffer<uint8_t> m_rx_buffer;
		
		void (* m_tx_callback)();
		
		uint16_t m_rx_length;
		void (* m_rx_callback)(I2CRxData);

		volatile bool m_busy;
};


I2CMaster& i2c0();


#endif