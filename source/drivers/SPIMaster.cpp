#include "SPIMaster.h"
#include "nrf_nvic.h"


extern "C" void SPI1_TWI1_IRQHandler(void);


SPIMaster::SPIMaster(const SPIConfig& conf)
: m_conf(conf), m_repeated_start(false), m_tx_length(0), m_tx_pointer(&m_tx_buffer[0]), m_rx_length(0), m_rx_base(nullptr), m_rx_pointer(nullptr), m_busy(false)
{
	// Disable the SPI peripheral before configuring it
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Disabled;
	
	// Enable interrupt sources
	m_conf.periph->INTENSET = 
	
	// Enable interrupt in the NVIC
	// Must use sd_nvic_EnableIRQ to play nicely with the SoftDevice
	sd_nvic_EnableIRQ((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12));
	
	// Enable the SPI peripheral
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Enabled;
}


void SPIMaster::write(uint8_t address, const uint8_t* data, uint8_t length, bool repeated_start)
{
	
}


void SPIMaster::read(uint8_t address, uint8_t* data, uint8_t length)
{
	
}


void SPIMaster::isr()
{
	
}


SPIMaster& spi1()
{
	static SPIMaster periph(spi1_config);
	return periph;
}


void SPI1_TWI1_IRQHandler()
{
	spi1().isr();
}

