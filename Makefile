# AVR-GCC Makefile
PROJECT = Transistor-LC-Freq-Meter
SOURCES = main.cpp lcd.c
CC = avr-g++
OBJCOPY = avr-objcopy
MMCU = atmega48

CFLAGS = -mmcu=$(MMCU) -Os -Wall -Werror -Wextra -pedantic

$(PROJECT).bin: $(PROJECT).out
	$(OBJCOPY) -j .text -O binary $(PROJECT).out $(PROJECT).bin

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)
	avr-size $(PROJECT).out

program: $(PROJECT).bin
	avrdude -p m48 -c stk500v2 -e -U flash:w:$(PROJECT).bin

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).bin
	rm -f *~
