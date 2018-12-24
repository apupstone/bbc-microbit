#ifndef UART_H
#define UART_H


#include "nrf51.h"
#include "nrf51_bitfields.h"
#include <stdint.h>
#include "CircularBuffer.h"


static const uint8_t UART_TX_BUFFER_SIZE = 64;
static const uint8_t UART_RX_BUFFER_SIZE = 64;


enum class UARTBaudRate
{
	Baud1200 = 0x0004F000,
	Baud2400 = 0x0009D000,
	Baud4800 = 0x0013B000,
	Baud9600 = 0x00275000,
	Baud14400 = 0x003B0000,
	Baud19200 = 0x004EA000,
	Baud28800 = 0x0075F000,
	Baud38400 = 0x009D5000,
	Baud57600 = 0x00EBF000,
	Baud76800 = 0x013A9000,
	Baud115200 = 0x01D7E000,
	Baud230400 = 0x03AFB000,
	Baud250000 = 0x04000000,
	Baud460800 = 0x075F7000,
	Baud921600 = 0x0EBEDFA4,
	Baud1M = 0x10000000
};


struct UARTConfig
{
	NRF_UART_Type* periph;
	UARTBaudRate baud_rate;
	uint32_t pin_rxd;
	uint32_t pin_txd;
};


static const UARTConfig uart0_config = 
{
	NRF_UART0,
	UARTBaudRate::Baud9600,
	18, //25,
	24
};


struct UARTRxData
{
	uint8_t data[UART_RX_BUFFER_SIZE];
	uint16_t length;
};


class UART
{
	public:
		UART(const UARTConfig& conf);
		void start_tx();
		void stop_tx();
		void start_rx();
		void stop_rx();
		void write(uint8_t* data, uint16_t length);
		uint16_t read(uint8_t* data, uint16_t length);
		void isr();
		
	private:
		const UARTConfig& m_conf;
		
		CircularBuffer<uint8_t> m_tx_buffer;
		CircularBuffer<uint8_t> m_rx_buffer;
		
		bool m_txing;
		bool m_rxing;
		
		bool m_busy;
};


UART& uart0();


#endif