CC = gcc
CFLAGS = -g -ggdb -Wall -O2 -fomit-frame-pointer -I../ansiutils

all: zextest

tables.h: maketables.c
	$(CC) -Wall $< -o maketables
	./maketables > $@

z80emu.o: z80emu.c z80emu.h z80config.h z80user.h \
	instructions.h macros.h tables.h
	$(CC) $(CFLAGS) -c $<

zextest.o: zextest.c zextest.h z80emu.h z80config.h
	$(CC) -Wall -c $<

OBJECT_FILES = zextest.o z80emu.o sysbus.o ansitty.o disk.o ttyinput.o 

zextest: $(OBJECT_FILES)
	$(CC) -g -ggdb -L/usr/lib64 -L/usr/lib $(OBJECT_FILES) -L/usr/lib -lansicanvas -lansisdlcanvas -lSDL2 -lm -o $@

dumpldr:
	dd if=disks/drivea.dsk of=disks/bootldr.bin bs=1 count=256
	z80dasm -t -a -l -g 0 disks/bootldr.bin | more

clean:
	rm -f *.o zextest maketables disks/bootldr.bin
