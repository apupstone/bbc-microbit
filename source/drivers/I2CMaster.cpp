#include "I2CMaster.h"
#include "nrf_nvic.h"


extern "C" void SPI0_TWI0_IRQHandler(void);


I2CMaster::I2CMaster(const I2CConfig& conf)
: m_conf(conf), m_repeated_start(false), m_tx_buffer(I2C_TX_BUFFER_SIZE), m_rx_buffer(I2C_RX_BUFFER_SIZE), m_rx_length(0), m_busy(false)
{
	// Disable the TWI peripheral before configuring it
	m_conf.periph->ENABLE = TWI_ENABLE_ENABLE_Disabled;
	
	m_conf.periph->FREQUENCY = (uint32_t)m_conf.frequency;

	// Configure SCL and SDA pins for correct behaviour when the TWI peripheral is disabled
	NRF_GPIO->PIN_CNF[m_conf.pin_scl] = (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos);
	NRF_GPIO->PIN_CNF[m_conf.pin_sda] = (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos);
	
	// Configure SCL and SDA pins for use with the TWI peripheral
	m_conf.periph->PSELSCL = m_conf.pin_scl;
	m_conf.periph->PSELSDA = m_conf.pin_sda;
	
	// Enable interrupt sources
	m_conf.periph->INTENSET = TWI_INTENSET_STOPPED_Msk | TWI_INTENSET_RXDREADY_Msk | TWI_INTENSET_TXDSENT_Msk | TWI_INTENSET_ERROR_Msk | TWI_INTENSET_BB_Msk;
	
	sd_nvic_SetPriority((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12), 1);
	
	// Enable interrupt in the NVIC
	// Must use sd_nvic_EnableIRQ to play nicely with the SoftDevice
	// e.g. TWI0 base is 0x40003000
	// 0x40003000 & 0x0000FFFF == 0x00003000
	// 0x00003000 >> 12 == 0x00000003 which is the IRQn for TWI0
	sd_nvic_EnableIRQ((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12));
	
	// Enable the TWI peripheral
	m_conf.periph->ENABLE = TWI_ENABLE_ENABLE_Enabled;
}


void I2CMaster::write(uint8_t address, const uint8_t* data, uint8_t length, void (* callback)(), bool repeated_start)
{
	if (m_busy)
	{
		return;
	}

	// Set slave address
	m_conf.periph->ADDRESS = address;
	
	// Disable shorts
	m_conf.periph->SHORTS = 0;
	
	// Set repeated start flag
	m_repeated_start = repeated_start;
	
	for (uint8_t i = 0; i < length; i++)
	{
		m_tx_buffer.put(data[i]);
	}
	
	m_busy = true;
	
	m_tx_callback = callback;
	
	// Start transmission
	m_conf.periph->TASKS_STARTTX = 1;
	
	// Transmit first byte
	uint8_t tx_byte;
	if(m_tx_buffer.get(&tx_byte))
	{
		m_conf.periph->TXD = tx_byte;
	}
}


void I2CMaster::read(uint8_t address, uint8_t length, void (* callback)(I2CRxData))
{
	// Set slave address
	m_conf.periph->ADDRESS = address;
	
	if (length > 1)
	{
		// Enable BB/SUSPEND short
		m_conf.periph->SHORTS = TWI_SHORTS_BB_SUSPEND_Msk;
	}
	else
	{
		// Enable BB/STOP short
		m_conf.periph->SHORTS = TWI_SHORTS_BB_STOP_Msk;
	}
	
	m_rx_length = length;
	
	m_busy = true;
	
	m_rx_callback = callback;
	
	// Start receive
	m_conf.periph->TASKS_STARTRX = 1;
}


void I2CMaster::isr()
{
	if (m_conf.periph->EVENTS_STOPPED)
	{
		m_busy = false;
		m_conf.periph->EVENTS_STOPPED = 0;
	}
	else if (m_conf.periph->EVENTS_RXDREADY)
	{
		m_rx_buffer.put(m_conf.periph->RXD);
		if (m_rx_buffer.get_size() == m_rx_length - 1)
		{
			// Enable BB/STOP short
			m_conf.periph->SHORTS = TWI_SHORTS_BB_STOP_Msk;
		}
		else if (m_rx_buffer.get_size() == m_rx_length)
		{
			I2CRxData rx_data;
			rx_data.length = 0;
			while(m_rx_buffer.get(&rx_data.data[rx_data.length++]));
		
			// Call the Rx callback function
			m_rx_callback(rx_data);
		}
		m_conf.periph->EVENTS_RXDREADY = 0;
	}
	else if (m_conf.periph->EVENTS_TXDSENT)
	{
		uint8_t tx_byte;
		if(m_tx_buffer.get(&tx_byte))
		{
			// Transmit next byte
			m_conf.periph->TXD = tx_byte;
		}
		else
		{
			if (!m_repeated_start)
			{
				// Stop transmission
				m_conf.periph->TASKS_STOP = 1;
				
				// Clear repeated start flag
				m_repeated_start = false;
			}
			
			if (m_tx_callback != nullptr)
			{
				m_tx_callback();
			}
		}
		m_conf.periph->EVENTS_TXDSENT = 0;
	}
	else if (m_conf.periph->EVENTS_ERROR)
	{
		if (m_conf.periph->ERRORSRC & TWI_ERRORSRC_OVERRUN_Msk)
		{
			// Overrun Error Occurred
			
		}
		else if (m_conf.periph->ERRORSRC & TWI_ERRORSRC_ANACK_Msk)
		{
			// Address NACK Occurred
			
		}
		else if (m_conf.periph->ERRORSRC & TWI_ERRORSRC_DNACK_Msk)
		{
			// Data NACK Occurred
			
		}
		m_conf.periph->EVENTS_ERROR = 0;
	}
	else if (m_conf.periph->EVENTS_BB)
	{
		m_conf.periph->EVENTS_BB = 0;
	}
}


I2CMaster& i2c0()
{
	static I2CMaster periph(i2c0_config);
	return periph;
}


void SPI0_TWI0_IRQHandler()
{
	i2c0().isr();
}

