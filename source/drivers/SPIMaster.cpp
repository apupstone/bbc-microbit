#include "SPIMaster.h"
#include "nrf_nvic.h"


extern "C" void SPI1_TWI1_IRQHandler(void);


SPIMaster::SPIMaster(const SPIConfig& conf)
: m_conf(conf), m_tx_buffer(SPI_TX_BUFFER_SIZE), m_rx_buffer(SPI_RX_BUFFER_SIZE), m_rx_length(0), m_busy(false)
{
	// Disable the SPI peripheral before configuring it
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Disabled;
	
	// Enable interrupt sources
	m_conf.periph->INTENSET = SPI_INTENSET_READY_Msk;
	
	// Enable interrupt in the NVIC
	// Must use sd_nvic_EnableIRQ to play nicely with the SoftDevice
	sd_nvic_EnableIRQ((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12));
	
	// Enable the SPI peripheral
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Enabled;
}


void SPIMaster::write(const uint8_t* data, uint16_t length, void (* callback)())
{
	for (uint16_t i = 0; i < length; i++)
	{
		m_tx_buffer.put(data[i]);
	}
	
	if (!m_busy)
	{
		uint8_t tx_byte;
		if (m_tx_buffer.get(&tx_byte))
		{
			m_busy = true;
			// Write next byte into the TXD register
			m_conf.periph->TXD = tx_byte;
		}
	}
}


uint16_t SPIMaster::read(uint8_t* data, void (* callback)(SPIRxData))
{
	uint16_t length = 0;
	
	while (m_rx_buffer.get(&data[length++]));
	
	return length;
}


void SPIMaster::isr()
{
	if (m_conf.periph->EVENTS_READY)
	{
		// Read received byte from the RXD register
		m_rx_buffer.put(m_conf.periph->RXD);
	
		uint8_t tx_byte;
		if (m_tx_buffer.get(&tx_byte))
		{
			// Write next byte into the TXD register
			m_conf.periph->TXD = tx_byte;
		}
		else
		{
			m_busy = false;
		}
	
		m_conf.periph->EVENTS_READY = 0;
	}
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

