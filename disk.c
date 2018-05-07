#include "zextest.h"
#include "disk.h"


#define MAX_DRIVES      26
#define SECTOR_SIZE     128

DiskDrive drives[MAX_DRIVES];

int disk_init()
{
    off_t offset = 0;
    int id = 0;
    int r = 0;
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
            memset(drives[id].backingstore, 0, drives[id].size);
            assert(drives[id].backingstore);
            fh = fopen(drives[id].diskfilename, "rb");
            assert(fh);
            fseek(fh, 0, SEEK_SET);
            printf("backingstore = 0x%08lx\n", drives[id].backingstore);
            for (offset = 0; offset < drives[id].size; offset += SECTOR_SIZE) {
                unsigned char *ptr = drives[id].backingstore;
                printf("offset = %u\n", offset);
                printf("backingstore + offset = 0x%08lx\n", ptr);
                ptr  += offset;
                memory_dump(ptr, 0, 256);
                r = fread(ptr, SECTOR_SIZE, 1, fh);
                printf("r = %u\n", r);
                memory_dump(ptr, 0, 256);
                assert(NULL);
            }
            assert(offset == drives[id].size);
            printf(" ... OK\n");
            drives[id].present = true;
            detected++;
            fclose(fh);
        }
    }

    return detected;
}

int disk_readfromdrivetomemory(ZEXTEST *context, int driveid, uint16_t tgt_addr, off_t src_offset, uint16_t bytes)
{
    printf("disk_readfromdrivetomemory(0x%08lx, %u, 0x%04x, 0x%08lx, %u)\n", (long unsigned int) context->memory, driveid, (unsigned int) tgt_addr, src_offset, bytes);
    /* ensure drive has valid size */
    assert(drives[driveid].size);
    /* ensure drive is present */
    assert(drives[driveid].present);
    /* ensure backingstore is present */
    assert(drives[driveid].backingstore);
    /* ensure read is within limits of device */
    assert(src_offset < drives[driveid].size);
    assert(src_offset + bytes <= drives[driveid].size);
    
    memory_dump(drives[driveid].backingstore, 0, 256);
    assert(NULL);
    memcpy(context->memory + tgt_addr, drives[driveid].backingstore + src_offset, bytes);
    return 1;
}

