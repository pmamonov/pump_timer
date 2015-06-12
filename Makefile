DEVICE  = atmega8
F_CPU   = 8000000	# in Hz

CFLAGS  = -I. -DDEBUG_LEVEL=0
OBJECTS = main.o

COMPILE = avr-gcc -std=c99 -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -R .eeprom -O ihex main.elf main.hex
	avr-size main.elf

main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf -Wl,-Map,main.map $(OBJECTS)

.c.o:
	$(COMPILE) -c $< -o $@

clean:
	rm -f main.hex main.elf main.map $(OBJECTS)

fuse:
	avrdude -P usb -c usbasp -p m8 -U lfuse:w:0xe4:m -U hfuse:w:0xd9:m

load: main.hex
	avrdude -P usb -c usbasp -p m8 -U flash:w:main.hex:i
