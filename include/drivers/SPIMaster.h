#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#include "nrf51.h"
#include "nrf51_bitfields.h"
#include <stdint.h>
#include "CircularBuffer.h"


static const uint16_t SPI_TX_BUFFER_SIZE = 1024;
static const uint16_t SPI_RX_BUFFER_SIZE = 64;


enum class SPIFrequency: uint32_t
{
	K125 = 0x02000000,
	K250 = 0x04000000,
	K500 = 0x08000000,
	M1 = 0x10000000,
	M2 = 0x20000000,
	M4 = 0x40000000,
	M8 = 0x80000000
};


enum class SPIMode
{
	MODE0,
	MODE1,
	MODE2,
	MODE3
};


struct SPIConfig
{
	NRF_SPI_Type* periph;
	SPIFrequency frequency;
	SPIMode mode;
	bool lsb_first;
	uint32_t pin_sck;
	uint32_t pin_mosi;
	uint32_t pin_miso;
};


static const SPIConfig spi1_config = 
{
	NRF_SPI1,
	SPIFrequency::M8,
	SPIMode::MODE0,
	false,
	23,
	21,
	22,
};


struct SPITxData
{
	uint8_t data;
	uint32_t pin_cs;
	uint32_t pin_dc;
	bool dc;
};


struct SPIRxData
{
	uint8_t data[SPI_RX_BUFFER_SIZE];
	uint16_t length;
};


class SPIMaster
{
	public:
		SPIMaster(const SPIConfig& conf);
		bool write(const uint8_t* data, uint16_t length, void (* callback)(), uint32_t pin_cs, uint32_t pin_dc=0xFFFFFFFF, bool dc=false);
		void read(void (* callback)(SPIRxData));
		void isr();
		bool busy() { return m_busy; }
		
	private:
		const SPIConfig& m_conf;
		
		CircularBuffer<SPITxData> m_tx_buffer;
		CircularBuffer<uint8_t> m_rx_buffer;
		
		uint16_t m_rx_length;
		
		uint32_t m_cur_cs;
		volatile bool m_busy;
};


SPIMaster& spi1();


#endif