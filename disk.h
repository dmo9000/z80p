#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

struct _disk_drive {
                    int driveid;
                    char diskfilename[256];
                    bool present;
                    size_t size;
                    uint8_t num_tracks;
                    uint8_t num_spt;
                    uint16_t selected_track;
                    uint16_t selected_sector;
                    bool io_in_progress;
                    unsigned char *backingstore;
                    };

typedef struct _disk_drive DiskDrive;


int disk_init();
int disk_readfromdrivetomemory(ZEXTEST *context, int driveid, uint16_t tgt_addr, off_t src_offset, uint16_t bytes);
