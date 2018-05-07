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
    if (!disk_init()) {
            return 0;
            }
    return 1;

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
 
    return disk_readfromdrivetomemory(context, driveid, tgt_addr, src_offset, bytes);
    return 0;

}



