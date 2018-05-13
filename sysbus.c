#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "zextest.h"
#include "sysbus.h"
#include "ansitty.h"
#include "ttyinput.h"
#include "disk.h"

int sysbus_init()
{

    printf("System bus initialization ...\n");
    ansitty_init();
    return 1;

}

int sysbus_disks_init()
{
    return disk_init();
}

int sysbus_bootloader(ZEXTEST *context)
{
    printf("sysbus_bootloader()\n");
    return sysbus_ReadFromDriveToMemory(context, 0, 0, 0, 256);
}


int sysbus_ReadFromDriveToMemory(ZEXTEST *context, int driveid, uint16_t tgt_addr, off_t src_offset, uint16_t bytes)
{
    printf("sysbus_ReadFromDriveToMemory(driveid=%d, tgt_addr=0x%04x, src_offset=0x%08x, %u)\n",
           driveid, tgt_addr, (unsigned int) src_offset, bytes);

    disk_readfromdrivetomemory(context, driveid, tgt_addr, src_offset, bytes);

    return 0;

}

void memory_dump(unsigned char *ptr, uint16_t addr, uint16_t size)
{
    int i = 0;
    int c = 0;

    printf("memory_dump(0x%08lx, 0x%04x, 0x%04x)\n", (unsigned long) ptr, addr, size);

    for (i = 0; i < size; i++) {
        if (!(c % 16)) {
            printf("0x%08x: ", addr);
            }
        printf("%02x ", ptr[i]);
        addr++;
        c++;
        if (!(c % 16)) {
            printf("\n");
            }
        }
    printf("\n");
}




int _Z80_INPUT_BYTE(ZEXTEST *context, uint16_t port, uint8_t x)                                             
{                                                                       
        //SystemCall((ZEXTEST *) context);                                
        printf("     _Z80_INPUT_BYTE(0x%02X, %02X)\n", port, x);              
        fflush(NULL);
        exit (1);

        switch (port) {
            case 0:
                /* CONST - get string from console */
                if (tty_processinput()) {
                    context->state.registers.byte[Z80_A] = 0xFF;
                    gfx_sdl_expose();
                    return 1;
                    } else {
                    //printf("** CPU BLOCKED ON INPUT **\n");
                    context->state.registers.byte[Z80_A] = 0x0;
                    gfx_sdl_expose();
                    return 1;
                    }
                break;
            case 1:
                /* CONIN - get character from console buffer */
                while (tty_processinput()) {
                    /* filling buffer */
                    }
                break;
            }

        return 1;
}

int _Z80_OUTPUT_BYTE(ZEXTEST *context, uint16_t port, uint8_t x)                                       
{                                                                       
        printf("    _Z80_OUTPUT_BYTE(0x%02X, %02X)\n", port, x);             
        ((ZEXTEST *) context)->is_done = !0;                            
        fflush(NULL);

        switch (port) {
                case 0x0A:
                    /* SELDSK */
                    printf("    + SELDSK selecting disk 0x%02x\n", context->state.registers.byte[Z80_A]); 
                    return 1;
                    break;
                case 0x0B:
                    /* SETTRK */
                    printf("    + SETTRK selecting track 0x%02x\n", context->state.registers.byte[Z80_A]); 
                    return 1;
                    break;
                case 0x0C:
                    /* SETSEC */
                    printf("    + SETSEC selecting sector 0x%02x\n", context->state.registers.byte[Z80_A]); 
                    return 1;
                    break;
                case 0x0F:
                    /* SETDMA (LO) */
                    printf("    + SETDMA (LO) set low byte 0x%02x",  context->state.registers.byte[Z80_A]);
                    //DMA = (DMA & 0xFF00) + (CPU->context.state.registers.byte[Z80_A]);
                    return 1;
                    break;
                case 0x10:
                    /* SETDMA (HI) */
                    printf("    + SETDMA (HI) set high byte 0x%02x",  context->state.registers.byte[Z80_A]);
                    //DMA = (DMA & 0x00FF) + (CPU->context.state.registers.byte[Z80_A] * 0x100);
                    return 1;
                    break; 
                default:
                    printf("UNHANDLED PORT: 0x%04x, 0x%02x\n", port, x);
                    exit(1);
                    break;
                }

    return 0;
}

