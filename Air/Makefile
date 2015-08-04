#
# Makefile for compiling ATmega328P projects and programming the microcontroller
#
# Created: 6/4/2015
# Author: Michel Robijns
#

target = main
dependencies =
objects =

microcontroller = 328p
programmerType = arduino
programmerPort = /dev/ttyACM0

cc = /usr/bin/avr-gcc
cFlags = -g -mmcu=atmega$(microcontroller) -Wall -Os -Werror -Wextra -std=gnu99

objectToHex = /usr/bin/avr-objcopy
programmer = /usr/bin/avrdude

all: flash

object:
	$(cc) $(cFlags) -c -o $(target).o $(target).c

elf: object
	$(cc) $(cFlags) -o $(target).elf $(target).o

hex: elf
	$(objectToHex) -j .text -j .data -O ihex $(target).elf $(target).hex

flash: hex
	$(programmer) -p m$(microcontroller) -c $(programmerType) -P $(programmerPort) -e -U flash:w:$(target).hex

clean:
	rm -f *.hex *.obj *.o

