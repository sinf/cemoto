
.PHONY: clean flash dump_fuses test

SRC=main.c
HEADERS=speed.c
TESTSRC=test.c
CFLAGS=-DF_CPU=9600000UL -Os -std=c99 -Wall -Wno-parentheses

PART=t13a
MCU=attiny13a
DUDE=avrdude -p $(PART) -c avrisp2
OUTDIR=build
ELF=$(OUTDIR)/prog.elf
HEX=$(OUTDIR)/prog.hex

$(HEX): $(ELF) $(OUTDIR)
	avr-objcopy -j .text -j .data -O ihex $< $@

$(ELF): $(SRC) $(HEADERS) $(OUTDIR)
	avr-gcc -mmcu=$(MCU) $(CFLAGS) -o $@ $(SRC)

$(OUTDIR)/test.elf: $(TESTSRC) $(HEADERS) $(OUTDIR)
	gcc $(CFLAGS) -o $@ $(TESTSRC)

test: $(OUTDIR)/test.elf
	./$(OUTDIR)/test.elf

$(OUTDIR):
	@if [ ! -d $(OUTDIR) ]; then mkdir -pv $@; fi

# -B: set programmer speed in microseconds. need >200 for cpu=128kHz
flash: $(HEX)
	$(DUDE) -e -U flash:w:$(HEX) -B 10

# int osc 9.6 MHz, startup= 14CK + 64ms
set_fuses:
	$(DUDE) -U lfuse:w:0x7a:m -U hfuse:w:0xFF:m -B 1000

dump_fuses:
	$(DUDE) -U lfuse:r:lfuse:b
	$(DUDE) -U hfuse:r:hfuse:b
	cat lfuse hfuse

dump_fuses_h:
	$(DUDE) -U lfuse:r:lfuse:h
	$(DUDE) -U hfuse:r:hfuse:h
	cat lfuse hfuse

clean:
	rm -f $(HEX) $(ELF) lfuse hfuse prog.hex prog.elf

size: $(ELF)
	objdump --wide -h $<

