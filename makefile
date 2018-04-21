# Don't change these
CC = avr-gcc
CFLAGS = -g -mmcu=atmega32m1 -Os -mcall-prologues
PROG = stk500
MCU = m32m1
INCLUDES = -I./lib-common/include/

# Change this line depending on what you're using
LIB = -L./lib-common/lib -luart -lspi -lcan -ltimer -lqueue

# Change this line based on your OS and port
#PORT = /dev/tty.usbmodem00187462
PORT = COM4

SRC = $(wildcard ./src/*.c)
OBJ = $(SRC:./src/%.c=./build/%.o)
DEP = $(OBJ:.o=.d)

heartbeat: $(OBJ)
	$(CC) $(CFLAGS) -o ./build/$@.elf $(OBJ) $(LIB)
	avr-objcopy -j .text -j .data -O ihex ./build/$@.elf ./build/$@.hex

./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

-include $(DEP)

./build/%.d: ./src/%.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean upload debug

clean:
	rm -f $(OBJ)
	rm -f $(DEP)

upload: heartbeat
	avrdude -c $(PROG) -p $(MCU) -P $(PORT) -U flash:w:./build/$^.hex

debug:
	@echo ————————————
	@echo $(SRC)
	@echo ————————————
	@echo $(OBJ)
	@echo ————————————
