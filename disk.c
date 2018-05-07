#include "zextest.h"
#include "disk.h"


#define MAX_DRIVES      26
#define SECTOR_SIZE     128

DiskDrive drives[MAX_DRIVES];

int disk_init()
{
    off_t offset = 0;
    int id = 0;
    struct stat statbuf;
    FILE *fh = NULL;
    int detected = 0;
    for (id = 0; id < MAX_DRIVES; id++) {
        memset(&drives[id], 0, sizeof(DiskDrive));
        memset(&drives[id].diskfilename, 0, 256);
        snprintf((char *) &drives[id].diskfilename, 255, "disks/drive%c.dsk", 97 + id);
        if (!lstat(drives[id].diskfilename, &statbuf)) {
            drives[id].size = statbuf.st_size;
            drives[id].present = true;
            printf(" * detected drive %c: %s, size = %lu", 65 + id, drives[id].diskfilename, drives[id].size);
            drives[id].backingstore = malloc(drives[id].size);
            assert(drives[id].backingstore);
            fh = fopen(drives[id].diskfilename, "rb");
            assert(fh);
            for (offset = 0; offset < drives[id].size; offset += SECTOR_SIZE) {
                fread(drives[id].backingstore + offset, SECTOR_SIZE, 1, fh);
            }
            assert(offset == drives[id].size);
            printf(" ... OK\n");
            detected++;
            fclose(fh);
        }
    }

    return detected;
}

int disk_readfromdrivetomemory(ZEXTEST *context, int driveid, uint16_t tgt_addr, off_t src_offset, uint16_t bytes)
{
    printf("disk_readfromdrivetomemory(0x%08lx, %u, 0x%04x, 0x%08lx, %u)\n", (long unsigned int) context, driveid, (unsigned int) tgt_addr, src_offset, bytes);
    assert(NULL);
}

