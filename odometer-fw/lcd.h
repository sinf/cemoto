#ifndef LCD_CMD_PREFIX

#include <stdint.h>

/* 
 * PB3/PCINT3: WR
 * PB4/PCINT4: DAT
 * PB1: OC0B/INT0/PCINT1 : CS
 */

#define LCD_PIN_CS 0
#define LCD_PIN_WR 1
#define LCD_PIN_DAT 2

#define LCD_CS (1u<<LCD_PIN_CS)
#define LCD_WR (1u<<LCD_PIN_WR)
#define LCD_DAT (1u<<LCD_PIN_DAT)

/* datasheet says command and write address are sent MSB-first but data is sent LSB-first. wtf....
 * here I reversed the bits of each command so a function sending LSB first could send them */
#define LCD_CMD_PREFIX(x) ((x)<<3|1) /* command begins with bits 101 */
#define LCD_CMD_BIAS_OFF LCD_CMD_PREFIX(0x40)
#define LCD_CMD_BIAS_ON LCD_CMD_PREFIX(0xc0)
#define LCD_CMD_BIAS_COM(ab,c) LCD_CMD_PREFIX(4|(ab)<<4|(c)<<7)
#define LCD_CMD_OSC_ON LCD_CMD_PREFIX(0x80) // turn on system oscillator
#define LCD_CMD_OSC_RC256K LCD_CMD_PREFIX(0x18) // select on-chip 256 kHz RC osc
#define LCD_CMD_OSC_XTAL32K LCD_CMD_PREFIX(0x28) // external 32 kHz crystal
#define LCD_CMD_OSC_EXT256K LCD_CMD_PREFIX(0x38) // external 256 kHz
#define LCD_CMD_TIMER_EN LCD_CMD_PREFIX(0x60) // enable time base output
#define LCD_CMD_NORMAL_MODE LCD_CMD_PREFIX(0xc7) // normal mode (as opposed to test mode)
#define LCD_CMD_TIMEBASE_WDT(tt) LCD_CMD_PREFIX(5|((tt)&1)<<7|((tt)&2)<<5|((tt)&4)<<3)
#define LCD_CMD_CLEAR_WDT LCD_CMD_PREFIX(0x70)

#define AB_2COMMONS 0
#define AB_3COMMONS 2
#define AB_4COMMONS 1
#define C_BIAS_HALF 0
#define C_BIAS_THIRD 1

/* LCD_CMD_BIAS_COM(ab,c):
 * c=0: 1/2 bias option
 * c=1: 1/3 bias option
 * ab=00: 2 commons option
 * ab=01: 3 commons option
 * ab=10: 4 commons option
 */

// digits
#define SEG_0 SEG(1,1,1,1,1,1,0)
#define SEG_1 SEG(0,1,1,0,0,0,0)
#define SEG_2 SEG(1,1,0,1,1,0,1)
#define SEG_3 SEG(1,1,1,1,0,0,1)
#define SEG_4 SEG(0,1,1,0,0,1,1)
#define SEG_5 SEG(1,0,1,1,0,1,1)
#define SEG_6 SEG(1,0,1,1,1,1,1)
#define SEG_7 SEG(1,1,1,0,0,0,0)
#define SEG_8 SEG(1,1,1,1,1,1,1)
#define SEG_9 SEG(1,1,1,1,0,1,1)

#define SEGDP (1<<3)
#define LCD_VFLIP 0

#if LCD_VFLIP
// rotate the segments 180 degrees for upside-down mounted lcd
#define SEG(a,b,c,d,e,f,g) (a<<7|b<<2|c<<1|d<<0|e<<4|f<<6|g<<5)
#else
#define SEG(a,b,c,d,e,f,g) (a<<0|b<<4|c<<6|d<<7|e<<2|f<<1|g<<5)
#endif

extern uint8_t lcd_mem[4];

void lcd_cmd(uint16_t cmd);
void lcd_refresh(void);

void lcd_init(void);

/* returns 7-segment bitmask corresponding to a digit 0-9 */
uint8_t lcd_num(uint8_t i);

#endif

