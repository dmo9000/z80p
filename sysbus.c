#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "zextest.h"
#include "sysbus.h"
#include "ansitty.h"
#include "ttyinput.h"
#include "disk.h"


DiskDrive *Selected_Drive = NULL;
uint8_t current_drive_id = 0;
uint16_t DMA = 0;
uint32_t pos = 0;
uint8_t rc = 0;

#define SECTOR_SIZE 128

char *banner = "Z80 REFERENCE PLATFORM V0.1\n\n";

int sysbus_init()
{

    char *ptr = banner;
    printf("System bus initialization ...\n");
    ansitty_init();
    while (ptr[0] != '\0') {
        ansitty_putc(ptr[0]);
        ptr++;
        }
    
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

    switch (port) {
    case 0x0E:
        /* DISK IO - acknowledge controller */
        Selected_Drive = GetDriveReference(current_drive_id);
        assert(Selected_Drive);
        assert(Selected_Drive->io_in_progress);
        Selected_Drive->io_in_progress = false;
        printf("    + READSEC_END: DMA=0x%04X reading drive %u, track %u, sector %u\n", DMA, current_drive_id, Selected_Drive->selected_track, Selected_Drive->selected_sector);
        return 0;
        break;
    default:
        printf("UNHANDLED PORT: 0x%04x, 0x%02x\n", port, x);
        exit(1);
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
    case 0x01:
        /* CONOUT */
   		printf("CONOUT->[%c]\n", context->state.registers.byte[Z80_A]);
        ansitty_putc(context->state.registers.byte[Z80_A]);
        return 1;
        break;  
    case 0x0A:
        /* SELDSK */
        current_drive_id = context->state.registers.byte[Z80_A];
        printf("    + SELDSK selecting disk 0x%02x\n", current_drive_id);
        Selected_Drive = GetDriveReference(current_drive_id);
        assert(Selected_Drive);
        return 1;
        break;
    case 0x0B:
        /* SETTRK */
        Selected_Drive = GetDriveReference(current_drive_id);
        assert(Selected_Drive);
        printf("    + SETTRK selecting track %u/%u\n", context->state.registers.byte[Z80_A], Selected_Drive->num_tracks);
        Selected_Drive->selected_track = context->state.registers.byte[Z80_A];
        return 1;
        break;
    case 0x0C:
        /* SETSEC */
        Selected_Drive = GetDriveReference(current_drive_id);
        assert(Selected_Drive);
        printf("    + SETSEC selecting sector %u/%u\n", context->state.registers.byte[Z80_A], Selected_Drive->num_spt);
        Selected_Drive->selected_sector = context->state.registers.byte[Z80_A];
        return 1;
        break;
    case 0x0D:
        /* READSEC */
        Selected_Drive = GetDriveReference(current_drive_id);
        assert(Selected_Drive);
        assert(!Selected_Drive->io_in_progress);
        pos = (((long)Selected_Drive->selected_track) * ((long)Selected_Drive->num_spt) + Selected_Drive->selected_sector - 1) << 7;
        switch (context->state.registers.byte[Z80_A]) {
        case 0x00:
            /* READ OPERATION */
            printf("    + READSEC_START: DMA=0x%04X reading drive %u, track %u, sector %u\n", DMA, current_drive_id, Selected_Drive->selected_track, Selected_Drive->selected_sector);
            Selected_Drive->io_in_progress = true;
            rc = sysbus_ReadFromDriveToMemory(context, current_drive_id, DMA, pos, SECTOR_SIZE);
            memory_dump(context->memory + DMA, DMA, 128);
            break;
        default:
            /* OTHER OPERATION */
            printf("unsupported operation\n");
            assert(NULL);
            break;
        }
        return 1;
        break;
    case 0x0F:
        /* SETDMA (LO) */
        printf("    + SETDMA (LO) set low byte 0x%02x\n",  context->state.registers.byte[Z80_A]);
        DMA = (DMA & 0xFF00) + (context->state.registers.byte[Z80_A]);
        return 1;
        break;
    case 0x10:
        /* SETDMA (HI) */
        printf("    + SETDMA (HI) set high byte 0x%02x\n",  context->state.registers.byte[Z80_A]);
        DMA = (DMA & 0x00FF) + (context->state.registers.byte[Z80_A] * 0x100);
        return 1;
        break;
    default:
        printf("UNHANDLED PORT: 0x%04x, 0x%02x\n", port, x);
        exit(1);
        break;
    }

    return 0;
}

