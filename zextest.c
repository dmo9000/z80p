/* zextest.c
 * Example program using z80emu to run the zexall and zexdoc tests. This will
 * check if the Z80 is correctly emulated.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 * Copyright (c) 2012 Chris Pressey
 *
 * This code is free, do whatever you want with it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "zextest.h"
#include "z80emu.h"
#include "sysbus.h"
#include "ttyinput.h"
#include "ansitty.h"

#define Z80_CPU_SPEED           8000000   /* In Hz. */
//#define Z80_CPU_SPEED           32000000   /* In Hz. */
#define CYCLES_PER_STEP         (Z80_CPU_SPEED / 50)
#define MAXIMUM_STRING_LENGTH   100

ZEXTEST	context;
double 	total;
static void	emulate (char *filename);

int main (int argc, char *argv[])
{
    time_t	start, stop;
    int i = 1;

    if (!sysbus_init()) {
        printf("error initializing system bus\n");
        assert(NULL);
    }

    if (!sysbus_disks_init()) {
        printf("error initializing disk controller\n");
        assert(NULL);
    }

    start = time(NULL);
    if (argc > 1) {
        while (i < argc) {
            context.state.pc = 0x100;
            emulate(argv[i]);
            i++;
        }
    } else {
        printf("Attempting to boot from disks/drivea.disk ...\n");
        Z80Reset(&context.state);

        context.state.pc = 0x0;
        total = 0.0;

        sysbus_bootloader(&context);

        while (!context.is_done) {
            total += Z80Emulate(&context.state, CYCLES_PER_STEP, &context);
    //        total += Z80Emulate(&context.state, 0, &context);
        }

    }
    stop = time(NULL);
    printf("Execution of all modules in %d second(s).\n", (int) (stop - start));

    while (1) {
        tty_processinput();
        usleep(20000);
    }

    return EXIT_SUCCESS;
}

/* Emulate "zexdoc.com" or "zexall.com". */

static void emulate (char *filename)
{
    FILE   	*file;
    long   	l;

    printf("Testing \"%s\"...\n", filename);
    if ((file = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open file!\n");
        exit(EXIT_FAILURE);
    }


    fseek(file, 0, SEEK_END);
    l = ftell(file);

    fseek(file, 0, SEEK_SET);
    fread(context.memory + 0x100, 1, l, file);

    fclose(file);

    /* Patch the memory of the program. Reset at 0x0000 is trapped by an
     * OUT which will stop emulation. CP/M bdos call 5 is trapped by an IN.
    * See Z80_INPUT_BYTE() and Z80_OUTPUT_BYTE() definitions in z80user.h.
           */

    context.memory[0] = 0xd3;       /* OUT N, A */
    context.memory[1] = 0x00;

    context.memory[5] = 0xdb;       /* IN A, N */
    context.memory[6] = 0x00;
    context.memory[7] = 0xc9;       /* RET */

    context.is_done = 0;

    /* Emulate. */

    Z80Reset(&context.state);
    context.state.pc = 0x100;
    total = 0.0;
    do {
        total += Z80Emulate(&context.state, CYCLES_PER_STEP, &context);
    } while (!context.is_done);


    printf("\n%.0f cycle(s) emulated.\n"
           "For a Z80 running at %.2fMHz, "
           "that would be %d second(s) or %.2f hour(s).\n",
           total,
           Z80_CPU_SPEED / 1000000.0,
           (int) (total / Z80_CPU_SPEED),
           total / ((double) 3600 * Z80_CPU_SPEED));
}

/* Emulate CP/M bdos call 5 functions 2 (output character on screen) and 9
 * (output $-terminated string to screen).
 */

void SystemCall (ZEXTEST *zextest)
{
    if (zextest->state.registers.byte[Z80_C] == 2) {
        //printf("%c", zextest->state.registers.byte[Z80_E]);
        ansitty_putc(zextest->state.registers.byte[Z80_E]);
    }

    else if (zextest->state.registers.byte[Z80_C] == 9) {

        int     i, c;

        for (i = zextest->state.registers.word[Z80_DE], c = 0;
                zextest->memory[i] != '$';
                i++) {

            //printf("<%c>", zextest->memory[i & 0xffff]);
            ansitty_putc(zextest->memory[i & 0xffff]);
            if (c++ > MAXIMUM_STRING_LENGTH) {

                fprintf(stderr,
                        "String to print is too long!\n");
                exit(EXIT_FAILURE);

            }

        }

    }
}
