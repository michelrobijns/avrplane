#
# Makefile for compiling ATmega328P projects and programming the microcontroller
#
# Created: 4/6/2015
# Author: Michel Robijns
#

target = main
dependencies = timers.h serial.h adc.h
objects = main.o timers.o serial.o adc.o

microcontroller = 328p
programmerType = arduino
programmerPort = /dev/ttyACM0

cc = /usr/bin/avr-gcc
cFlags = -g -mmcu=atmega$(microcontroller) -Wall -Os -Werror -Wextra -std=gnu99

objectToHex = /usr/bin/avr-objcopy
programmer = /usr/bin/avrdude

all: flash

%.o: %.c $(dependencies)
	$(cc) $(cFlags) -c -o $@ $<

$(target): $(objects)
	$(cc) $(cFlags) -o $(target).elf $^

hex: $(target)
	$(objectToHex) -j .text -j .data -O ihex $(target).elf $(target).hex

flash: hex
	$(programmer) -p m$(microcontroller) -c $(programmerType) -P $(programmerPort) -e -U flash:w:$(target).hex

clean:
	rm -f *.elf *.hex *.obj *.o
