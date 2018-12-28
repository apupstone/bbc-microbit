#include "SPIMaster.h"
#include "nrf_nvic.h"


extern "C" void SPI1_TWI1_IRQHandler(void);


SPIMaster::SPIMaster(const SPIConfig& conf)
: m_conf(conf), m_tx_buffer(SPI_TX_BUFFER_SIZE), m_rx_buffer(SPI_RX_BUFFER_SIZE), m_rx_length(0), m_cur_cs(0xFFFFFFFF), m_busy(false)
{
	// Disable the SPI peripheral before configuring it
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Disabled;
	
	m_conf.periph->FREQUENCY = (uint32_t)m_conf.frequency;
	
	// Configure pins for correct behaviour when the SPI peripheral is disabled
	NRF_GPIO->DIRSET = (1 << m_conf.pin_sck);
	NRF_GPIO->OUTSET = (1 << m_conf.pin_sck);
	NRF_GPIO->DIRSET = (1 << m_conf.pin_mosi);
	NRF_GPIO->OUTSET = (1 << m_conf.pin_mosi);
	NRF_GPIO->DIRCLR = (1 << m_conf.pin_miso);
	
	m_conf.periph->PSELSCK = m_conf.pin_sck;
	m_conf.periph->PSELMOSI = m_conf.pin_mosi;
	m_conf.periph->PSELMISO = m_conf.pin_miso;
	
	// Set up CONFIG register
	m_conf.periph->CONFIG = (((m_conf.lsb_first ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst) << SPI_CONFIG_ORDER_Pos) & SPI_CONFIG_ORDER_Msk) |
							(((uint32_t)m_conf.mode << SPI_CONFIG_CPHA_Pos) & (SPI_CONFIG_CPHA_Msk | SPI_CONFIG_CPOL_Msk));
	
	// Enable interrupt sources
	m_conf.periph->INTENSET = SPI_INTENSET_READY_Msk;
	
	// Enable interrupt in the NVIC
	// Must use sd_nvic_EnableIRQ to play nicely with the SoftDevice
	sd_nvic_EnableIRQ((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12));
	
	// Enable the SPI peripheral
	m_conf.periph->ENABLE = SPI_ENABLE_ENABLE_Enabled;
}


bool SPIMaster::write(const uint8_t* data, uint16_t length, void (* callback)(), uint32_t pin_cs, uint32_t pin_dc, bool dc)
{
	if (m_tx_buffer.get_capacity() - m_tx_buffer.get_size() < length)
	{
		return false;
	}

	for (uint16_t i = 0; i < length; i++)
	{
		SPITxData tx_data = 
		{
			data[i],
			pin_cs,
			pin_dc,
			dc
		};
	
		m_tx_buffer.put(tx_data);
	}
	
	if (!m_busy)
	{
		SPITxData tx_data;
		if (m_tx_buffer.get(&tx_data))
		{
			m_busy = true;
			
			// Assert low CS
			NRF_GPIO->OUTCLR = (1 << tx_data.pin_cs);
			
			if (tx_data.dc)
			{
				// Assert high DC
				NRF_GPIO->OUTSET = (1 << tx_data.pin_dc);
			}
			else
			{
				// Deassert low DC
				NRF_GPIO->OUTCLR = (1 << tx_data.pin_dc);
			}
			
			// Write next byte into the TXD register
			m_conf.periph->TXD = tx_data.data;
		}
	}
	
	return true;
}


void SPIMaster::read(void (* callback)(SPIRxData))
{
	SPIRxData rx_data;
	
	while (m_rx_buffer.get(&rx_data.data[rx_data.length++]));
	
	callback(rx_data);
}


void SPIMaster::isr()
{
	if (m_conf.periph->EVENTS_READY)
	{
		// Read received byte from the RXD register
		m_rx_buffer.put(m_conf.periph->RXD);
	
		SPITxData tx_data;
		if (m_tx_buffer.get(&tx_data))
		{
			m_busy = true;
			
			if (m_cur_cs != tx_data.pin_cs)
			{
				// Deassert high CS
				NRF_GPIO->OUTSET = (1 << m_cur_cs);
			}
			
			// Assert low CS
			m_cur_cs = tx_data.pin_cs;
			NRF_GPIO->OUTCLR = (1 << tx_data.pin_cs);
			
			if (tx_data.dc)
			{
				// Assert high DC
				NRF_GPIO->OUTSET = (1 << tx_data.pin_dc);
			}
			else
			{
				// Deassert low DC
				NRF_GPIO->OUTCLR = (1 << tx_data.pin_dc);
			}
			
			// Write next byte into the TXD register
			m_conf.periph->TXD = tx_data.data;
		}
		else
		{
			// Deassert high CS
			NRF_GPIO->OUTSET = (1 << m_cur_cs);
			
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

