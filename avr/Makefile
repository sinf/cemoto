
.PHONY: clean flash dump_fuses set_fuses size

SRC=main.c lcd.c odo.c
HEADERS=lcd.h odo.h
PART=t13a
MCU=attiny13a
DUDE=avrdude -p $(PART) -c avrisp2
CFLAGS+=-Os -std=c99 -Wall -Wextra -DF_CPU=9600000UL

prog.hex: prog.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

prog.elf: $(SRC) $(HEADERS)
	avr-gcc -mmcu=$(MCU) $(CFLAGS) -o $@ $(SRC)

# -B sets programmer speed in microseconds. if AVR is running at 128 kHz we need it
.flash_timestamp: prog.hex
	$(DUDE) -e -U flash:w:prog.hex -B 100
	touch $@

flash: .flash_timestamp ;

# int osc 9.6 MHz, startup= 14CK + 0ms, brown-out=4.3v
set_fuses:
	$(DUDE) -U lfuse:w:0x72:m -U hfuse:w:0xF9:m -B 1000

# int osc 9.6 MHz, startup= 14CK + 64ms
#	$(DUDE) -U lfuse:w:0x7a:m -U hfuse:w:0xFF:m -B 1000

dump_fuses:
	$(DUDE) -U lfuse:r:lfuse:h -B 1000
	$(DUDE) -U hfuse:r:hfuse:h -B 1000
	cat lfuse hfuse

clean:
	rm -f prog.hex prog.elf lfuse hfuse

size: prog.elf
	objdump --wide -h $<

