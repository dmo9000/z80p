#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "zextest.h"
#include "sysbus.h"
#include "ansitty.h"
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


    memory_dump(context->memory, 0, 256);

    return 0;

}

void memory_dump(unsigned char *ptr, off_t start, uint16_t size)
{
    uint16_t addr = start;
    int i = 0;
    printf("memory_dump(0x%08lx, 0x%04x, 0x%04x)\n", ptr, start, size);

    while (addr <= start+size) {
        printf("0x%08x %c ", addr, ':');
        for (i = 0 ; i < 16; i++) {
            ptr = (char *) ptr + addr + i;
            printf("%02x ", ptr[0]);
            ptr++;
        }
        ptr -= 16;
        printf(" %c ", ':');

        for (i = 0 ; i < 16; i++) {
            ptr = (char *) ptr + addr + i;
            if (ptr[0] >= 32 && ptr[0] <= 127) {
                printf("%c", ptr[0]);
            } else {
                printf(".");
            }
            ptr++;
        }
        /* line wrap */
        printf("\n");

        addr += 16;
    }

}



