#include "LCDDisplay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "UART.h"


void DataIgnorer(SPIRxData rx_data)
{
	// Gobbles up data coming in the SPI interface without doing anything with it.
}


void LCDCallback()
{
	spi1().read(DataIgnorer);
}


LCDDisplay::LCDDisplay(const LCDConfig& conf)
: m_conf(conf)
{
	// Set CS, DC and RST pins as digital outputs
	NRF_GPIO->DIRSET = (1 << m_conf.pin_cs);
	NRF_GPIO->DIRSET = (1 << m_conf.pin_dc);
	NRF_GPIO->DIRSET = (1 << m_conf.pin_rst);

	hardware_reset();
	
	// Deassert high CS
	NRF_GPIO->OUTSET = (1 << m_conf.pin_cs);
	
	software_reset();
	
	sleep_out();
	
	partial_mode_off();
	display_inversion_off();
	
	column_address_set(2, 129);
	row_address_set(1, 128);
	rgb_set(); // TODO
	tearing_effect_off();
	memory_data_access_control(false, false, false, false, false, false);
	interface_pixel_format(6); // 18 bits/pixel - TODO enum class
	
	idle_mode_off();
	display_on();
}


void LCDDisplay::lcd_callback()
{
	
}


void LCDDisplay::hardware_reset()
{
	// Drive pin high and wait for a bit
	NRF_GPIO->OUTSET = (1 << m_conf.pin_rst);
	vTaskDelay(pdMS_TO_TICKS(100));
	
	// Drive pin low and wait for at least 10 us
	NRF_GPIO->OUTCLR = (1 << m_conf.pin_rst);
	vTaskDelay(pdMS_TO_TICKS(1));
	
	// Drive pin high and wait for at least 120 ms
	NRF_GPIO->OUTSET = (1 << m_conf.pin_rst);
	vTaskDelay(pdMS_TO_TICKS(120));
}


void LCDDisplay::write_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
	// Frame buffer goes RGB RGB RGB ... RGB where R, G and B are each padded out to 8 bits

	column_address_set(x, x);
	row_address_set(y, y);
	
	uint8_t data[3] = {r, g, b};
	
	ram_write(&data[0], 3);
}


void LCDDisplay::draw_bbc()
{
	column_address_set(2, 129);
	row_address_set(1, 128);

	ram_write(&bbc[0], HEIGHT * WIDTH * 3);
}


void LCDDisplay::draw_microbit()
{
	column_address_set(2, 129);
	row_address_set(1, 128);

	ram_write(&microbit[0], HEIGHT * WIDTH * 3);
}


void LCDDisplay::draw_alexander()
{
	column_address_set(2, 129);
	row_address_set(1, 128);

	ram_write(&alexander[0], HEIGHT * WIDTH * 3);
}


void LCDDisplay::draw_red()
{
	column_address_set(2, 129);
	row_address_set(1, 128);

	ram_write(&red[0], HEIGHT * WIDTH * 3);
}


void LCDDisplay::nop()
{
	uint8_t data[1] = {0x00};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::software_reset()
{
	uint8_t data[1] = {0x01};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::sleep_in()
{
	uint8_t data[1] = {0x10};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::sleep_out()
{
	uint8_t data[1] = {0x11};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::partial_mode_on()
{
	uint8_t data[1] = {0x12};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::partial_mode_off()
{
	uint8_t data[1] = {0x13};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::display_inversion_off()
{
	uint8_t data[1] = {0x20};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::display_inversion_on()
{
	uint8_t data[1] = {0x21};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::gamma_curve_select(uint8_t gc)
{
	uint8_t data[1] = {0x26};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	m_conf.spi_periph.write(&gc, 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::display_off()
{
	uint8_t data[1] = {0x28};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::display_on()
{
	uint8_t data[1] = {0x29};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::column_address_set(uint16_t x_start, uint16_t x_end)
{
	uint8_t data[2] = {0x2A, 0x00};
	m_conf.spi_periph.write(&data[0], 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	data[0] = x_start >> 8;
	data[1] = x_start & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
	data[0] = x_end >> 8;
	data[1] = x_end & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::row_address_set(uint16_t y_start, uint16_t y_end)
{
	uint8_t data[2] = {0x2B, 0x00};
	m_conf.spi_periph.write(&data[0], 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	data[0] = y_start >> 8;
	data[1] = y_start & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
	data[0] = y_end >> 8;
	data[1] = y_end & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::ram_write(const uint8_t* wr_data, uint16_t length)
{
	char debug_info[64] = {0};

	uint8_t data[1] = {0x2C};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	for (uint16_t i = 0; i < length; i++)
	{
		// Keep attempting to write (returns false if SPI transmit buffer is currently full)
		while (!m_conf.spi_periph.write(&wr_data[i], 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true));
		
		if ((i % 100000) == 0)
		{
			sprintf(&debug_info[0], "%d\n", i);
			uart0().write((uint8_t *)&debug_info[0], sizeof(debug_info));
		}
	}
}


void LCDDisplay::rgb_set()
{
	// TODO
}


void LCDDisplay::partial_address_set(uint16_t start, uint16_t end)
{
	uint8_t data[2] = {0x30, 0x00};
	m_conf.spi_periph.write(&data[0], 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	data[0] = start >> 8;
	data[1] = start & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
	data[0] = end >> 8;
	data[1] = end & 0x00FF;
	m_conf.spi_periph.write(&data[0], 2, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::tearing_effect_off()
{
	uint8_t data[1] = {0x34};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


// Mode1: mode = false
// Mode2: mode = true
void LCDDisplay::tearing_effect_on(bool mode)
{
	uint8_t data[1] = {0x35};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	data[0] = (mode ? 0x01 : 0x00);
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::memory_data_access_control(bool m_y, bool m_x, bool m_v, bool m_l, bool rgb, bool m_h)
{
	uint8_t data[1] = {0x36};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	data[0] = ((1 << 7) & m_y) |
			  ((1 << 6) & m_x) |
			  ((1 << 5) & m_v) |
			  ((1 << 4) & m_l) |
			  ((1 << 3) & rgb) |
			  ((1 << 2) & m_h);
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


void LCDDisplay::idle_mode_off()
{
	uint8_t data[1] = {0x38};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::idle_mode_on()
{
	uint8_t data[1] = {0x39};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
}


void LCDDisplay::interface_pixel_format(uint8_t format)
{
	uint8_t data[1] = {0x3A};
	m_conf.spi_periph.write(&data[0], sizeof(data), lcd_callback, m_conf.pin_cs, m_conf.pin_dc, false);
	m_conf.spi_periph.write(&format, 1, lcd_callback, m_conf.pin_cs, m_conf.pin_dc, true);
}


LCDDisplay& lcd()
{
	static LCDDisplay disp(lcd_config);
	return disp;
}