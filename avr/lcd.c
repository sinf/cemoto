#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include "lcd.h"

// driver code for Holtek HT1621B

_Static_assert(sizeof(int)==2);
uint8_t lcd_mem[4] = {0};

uint8_t lcd_num(uint8_t i)
{
	static const uint8_t num[10] = {
		SEG(1,1,1,1,1,1,0), //0
		SEG(0,1,1,0,0,0,0), //1
		SEG(1,1,0,1,1,0,1), //2
		SEG(1,1,1,1,0,0,1), //3
		SEG(0,1,1,0,0,1,1), //4
		SEG(1,0,1,1,0,1,1), //5
		SEG(1,0,1,1,1,1,1), //6
		SEG(1,1,1,0,0,0,0), //7
		SEG(1,1,1,1,1,1,1), //8
		SEG(1,1,1,1,0,1,1), //9
	};
	return i < 10 ? num[i] : 0;
}

void lcd_delay(void) {
	// didn't bother testing if there is an upper speed limit
	_delay_us(1e6/8000);
}

void lcd_cs_low(void) {
	DDRB |= LCD_WR | LCD_CS | LCD_DAT;
	PORTB |= LCD_WR;
	PORTB &= ~LCD_CS; // LCD is listening when CS goes low
	lcd_delay();
}

void lcd_cs_high(void) {
	PORTB |= LCD_WR | LCD_CS;
	lcd_delay();
}

/* LSB first */
static void lcd_bitbang(uint16_t bits, uint8_t n)
{
	const uint8_t wr_dat_low = PORTB & ~(LCD_WR | LCD_DAT);
	// bits clocked in at rising edge of WR
	while (n) {
		uint8_t databit = bits & 1u;
		databit <<= LCD_PIN_DAT;
		PORTB = wr_dat_low | databit;
		lcd_delay();

		n -= 1;
		bits >>= 1;
		PORTB |= LCD_WR;
		lcd_delay();
	}
}

void lcd_cmd(uint16_t cmd)
{
	lcd_cs_low();
	lcd_bitbang(cmd, 3 + 9);
	lcd_cs_high();
}

void lcd_refresh(void)
{
	uint16_t *src = (uint16_t*) lcd_mem;
	lcd_cs_low();
	lcd_bitbang(5u, 3 + 6); // begin write command at address=0
	lcd_bitbang(src[0], 16);
	lcd_bitbang(src[1], 16);
	lcd_cs_high();
}

void lcd_init(void)
{
	lcd_cmd(LCD_CMD_NORMAL_MODE); // normal, not test mode
	lcd_cmd(LCD_CMD_OSC_ON); // enable oscillator
	lcd_cmd(LCD_CMD_OSC_RC256K); // want the internal oscillator
	// for external oscillators
	//lcd_cmd(LCD_CMD_OSC_XTAL32K);
	//lcd_cmd(LCD_CMD_OSC_EXT256K);
	lcd_cmd(LCD_CMD_BIAS_ON);
	lcd_cmd(LCD_CMD_BIAS_COM(AB_4COMMONS, C_BIAS_HALF));
	lcd_cmd(LCD_CMD_CLEAR_WDT);
	// datasheet recommends explicitly initializing all settings, TODO
}

