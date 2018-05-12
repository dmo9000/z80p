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
        SystemCall((ZEXTEST *) context);                                
        printf("- %llu: _Z80_INPUT_BYTE(0x%02X, %02X)\n",                
                (unsigned long long) time(NULL), port, x);              
        while (port == 0 && x == 0) {                                   
            tty_processinput();                                         
            }                                                           
        return 0;
}

int _Z80_OUTPUT_BYTE(ZEXTEST *context, uint16_t port, uint8_t x)                                       
{                                                                       
        printf("_Z80_OUTPUT_BYTE(0x%02X, %02X)\n", port, x);             
        ((ZEXTEST *) context)->is_done = !0;                            
        return 0;
}

