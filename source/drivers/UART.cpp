#include "UART.h"
#include "nrf_nvic.h"


extern "C" void UART0_IRQHandler(void);

void uart_tx_task(void *pvParameters);
void uart_rx_task(void *pvParameters);


UART::UART(const UARTConfig& conf)
: m_conf(conf), m_tx_buffer(UART_TX_BUFFER_SIZE), m_rx_buffer(UART_RX_BUFFER_SIZE), m_txing(false), m_rxing(false), m_busy(false)
{
	// Disable the UART peripheral before configuring it
	m_conf.periph->ENABLE = UART_ENABLE_ENABLE_Disabled;
	
	// Configure baud rate
	m_conf.periph->BAUDRATE = (uint32_t)m_conf.baud_rate;
	
	// Configure pins
	m_conf.periph->PSELRXD = m_conf.pin_rxd;
	m_conf.periph->PSELTXD = m_conf.pin_txd;
	
	// Enable interrupt sources
	m_conf.periph->INTENSET = UART_INTENSET_RXDRDY_Msk | UART_INTENSET_TXDRDY_Msk | UART_INTENSET_ERROR_Msk | UART_INTENSET_RXTO_Msk;
	
	// Enable interrupt in the NVIC
	// Must use sd_nvic_EnableIRQ to play nicely with the SoftDevice
	sd_nvic_EnableIRQ((IRQn_Type)(((uint32_t)m_conf.periph & 0x0000FFFF) >> 12));
	
	// Enable the UART peripheral
	m_conf.periph->ENABLE = UART_ENABLE_ENABLE_Enabled;
}


void UART::start_tx()
{
	if (!m_txing)
	{
		// Start transmission
		m_conf.periph->TASKS_STARTTX = 1;
	}
}


void UART::stop_tx()
{
	if (m_txing)
	{
		// Stop transmission
		m_conf.periph->TASKS_STOPTX = 1;
	}
}


void UART::start_rx()
{
	if (!m_rxing)
	{
		// Start receive
		m_conf.periph->TASKS_STARTRX = 1;
	}
}


void UART::stop_rx()
{
	if (m_rxing)
	{
		// Stop receive
		m_conf.periph->TASKS_STOPRX = 1;
	}
}


void UART::write(uint8_t* data, uint16_t length)
{
	if (!m_txing)
	{
		start_tx();
	}

	for (uint8_t i = 0; i < length; i++)
	{
		m_tx_buffer.put(data[i]);
	}
	
	if (!m_busy)
	{
		uint8_t tx_byte;
		if (m_tx_buffer.get(&tx_byte))
		{
			m_busy = true;
			// Write next byte into TXD register
			m_conf.periph->TXD = tx_byte;
		}
	}
}


uint16_t UART::read(uint8_t* data, uint16_t length=0)
{
	if (!m_rxing)
	{
		start_rx();
	}

	if (length == 0)
	{
		length = m_rx_buffer.get_capacity();
	}
	
	uint16_t out_length = 0;
	
	while (out_length < length && m_rx_buffer.get(&data[out_length]))
	{
		out_length++;
	}
	
	return out_length;
}


void UART::isr()
{
	if (m_conf.periph->EVENTS_RXDRDY)
	{
		// NB Must clear event flag before RXD is read
		m_conf.periph->EVENTS_RXDRDY = 0;
	
		// Retrieve latest byte from RXD register
		m_rx_buffer.put(m_conf.periph->RXD);
	}
	else if (m_conf.periph->EVENTS_TXDRDY)
	{
		uint8_t tx_byte;
		if (m_tx_buffer.get(&tx_byte))
		{
			// Write next byte into TXD register
			m_conf.periph->TXD = tx_byte;
		}
		else
		{
			m_busy = false;
		}
	
		m_conf.periph->EVENTS_TXDRDY = 0;
	}
	else if (m_conf.periph->EVENTS_ERROR)
	{
		if (m_conf.periph->ERRORSRC & UART_ERRORSRC_OVERRUN_Msk)
		{
		
		}
		else if (m_conf.periph->ERRORSRC & UART_ERRORSRC_PARITY_Msk)
		{
		
		}
		else if (m_conf.periph->ERRORSRC & UART_ERRORSRC_FRAMING_Msk)
		{
		
		}
		else if (m_conf.periph->ERRORSRC & UART_ERRORSRC_BREAK_Msk)
		{
		
		}
		m_conf.periph->EVENTS_ERROR = 0;
	}
	else if (m_conf.periph->EVENTS_RXTO)
	{
		m_conf.periph->EVENTS_RXTO = 0;
	}
}


UART& uart0()
{
	static UART periph(uart0_config);
	return periph;
}


void UART0_IRQHandler()
{
	uart0().isr();
}