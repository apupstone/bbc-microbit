#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "nrf51.h"
#include <stdint.h>
#include "SPIMaster.h"
#include "bbc.h"
#include "microbit.h"
#include "alexander.h"
#include "red.h"


static const uint16_t WIDTH = 128;
static const uint16_t HEIGHT = 128;


struct LCDConfig
{
	SPIMaster& spi_periph;
	uint32_t pin_cs;
	uint32_t pin_dc;
	uint32_t pin_rst;
	void (* tx_callback)();
};


void DataIgnorer(SPIRxData rx_data);

void LCDCallback();


static const LCDConfig lcd_config = 
{
	spi1(),
	1,
	2,
	3,
	
};


class LCDDisplay
{
	public:
		LCDDisplay(const LCDConfig& conf);
		
		static void lcd_callback();
		
		void write_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
		void write_region(uint16_t x_start, uint16_t x_end, uint16_t y_start, uint16_t y_end, uint8_t* data);
		
		void draw_microbit();
		void draw_alexander();
		void draw_bbc();
		void draw_red();
		
		void hardware_reset();
		
		void nop();
		void software_reset();
		void sleep_in();
		void sleep_out();
		void partial_mode_on();
		void partial_mode_off();
		void display_inversion_off();
		void display_inversion_on();
		void gamma_curve_select(uint8_t gc);
		void display_off();
		void display_on();
		void column_address_set(uint16_t x_start, uint16_t x_end);
		void row_address_set(uint16_t y_start, uint16_t y_end);
		void ram_write(const uint8_t* wr_data, uint16_t length);
		void rgb_set(); // TODO
		void partial_address_set(uint16_t start, uint16_t end);
		void tearing_effect_off();
		void tearing_effect_on(bool mode);
		void memory_data_access_control(bool m_y, bool m_x, bool m_v, bool m_l, bool rgb, bool m_h);
		void idle_mode_off();
		void idle_mode_on();
		void interface_pixel_format(uint8_t format);
	
	private:
		const LCDConfig& m_conf;
	
};


LCDDisplay& lcd();


#endif


/*

At power on:

Sleep In/Out = In					-> Out
Display On/Off = Off				-> On
Display Mode = Normal
Display Inversion = Off
Display Idle = Off
Column Start Addr (XS) = 0000h
Column End Addr (XE) = 007Fh
Row Start Addr (YS) = 0000h
Row End Addr (YE) = 009Fh			-> 007Fh
Gamma setting = GC0
Partial Start Addr (PSL) = 0000h
Partial End Addr (PEL) = 009Fh
Tearing On/Off = Off
Tearing Effect Mode = 0 (Mode1)
Mem Data Access Ctrl (MY/MX/MV/ML/RGB) = 0/0/0/0/0
Interface Pixel Colour Format = 6 (18 bits/pixel)
RDDPM = 08h
RDDMADCTL = 00h
RDDCOLMOD = 6 (18 bits/pixel)
RDDIM = 00h
RDDSM = 00h
ID2 = NV value
ID3 = NV value

*/