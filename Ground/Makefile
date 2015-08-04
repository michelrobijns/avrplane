#
# Makefile for compiling C projects
#
# Created: 7/22/2015
# Author: Michel Robijns
#

target = main
dependencies = joystick.h serial.h
objects = main.o joystick.o serial.o

cc = /usr/bin/gcc
cFlags = -I -g -Wall -Os -Wextra -std=gnu99 -pthread -lrt

all: $(target)

%.o: %.c $(dependencies)
	$(cc) $(cFlags) -c -o $@ $<

$(target): $(objects)
	$(cc) $(cFlags) -o $@ $^

clean:
	rm -f $(target) *.o

