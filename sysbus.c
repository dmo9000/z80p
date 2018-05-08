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

    return 0;

}

void memory_dump(unsigned char *ptr, uint16_t addr, uint16_t size)
{
    int i = 0;
    int c = 0;

    printf("memory_dump(0x%08x, 0x%04x, 0x%04x)\n", ptr, addr, size);

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



