/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a 1MB (256*4KB) paging device
 * for QEMU, 256 physical frames start at address FRAME_CACHE_START
 * for Arty, 28 physical frames are cached at address FRAME_CACHE_START
 * and 256 frames (1MB) start at the beginning of the microSD card
 */

#include "egos.h"
#include "disk.h"
#include <stdlib.h>
#include <string.h>
#define ARTY_CACHED_NFRAMES 28
#define NBLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE  /* 4KB / 512B == 8 */

int cache_slots[ARTY_CACHED_NFRAMES];
char *pages_start = (void*)FRAME_CACHE_START;

static uint cache_eviction() {
    /* Randomly select a cache slot to evict */
    uint idx = rand() % ARTY_CACHED_NFRAMES;
    uint frame_id = cache_slots[idx];

    earth->disk_write(frame_id * NBLOCKS_PER_PAGE, NBLOCKS_PER_PAGE, pages_start + PAGE_SIZE * idx);
    return idx;
}

void paging_init() { memset(cache_slots, 0xFF, sizeof(cache_slots)); }

int paging_invalidate_cache(uint frame_id) {
    for (uint j = 0; j < ARTY_CACHED_NFRAMES; j++)
        if (cache_slots[j] == frame_id) cache_slots[j] = -1;
}

int paging_write(uint frame_id, uint page_no) {
    char* src = (void*)(page_no << 12);
    if (earth->platform != ARTY) {
        memcpy(pages_start + frame_id * PAGE_SIZE, src, PAGE_SIZE);
        return 0;
    }

    for (uint i = 0; i < ARTY_CACHED_NFRAMES; i++)
        if (cache_slots[i] == frame_id) {
            memcpy(pages_start + PAGE_SIZE * i, src, PAGE_SIZE) != NULL;
            return 0;
        }

    uint free_idx = cache_eviction();
    cache_slots[free_idx] = frame_id;
    memcpy(pages_start + PAGE_SIZE * free_idx, src, PAGE_SIZE);
}

char* paging_read(uint frame_id, int alloc_only) {
    if (earth->platform != ARTY) return pages_start + frame_id * PAGE_SIZE;

    int free_idx = -1;
    for (uint i = 0; i < ARTY_CACHED_NFRAMES; i++) {
        if (cache_slots[i] == -1 && free_idx == -1) free_idx = i;
        if (cache_slots[i] == frame_id) return pages_start + PAGE_SIZE * i;
    }

    if (free_idx == -1) free_idx = cache_eviction();
    cache_slots[free_idx] = frame_id;

    if (!alloc_only)
        earth->disk_read(frame_id * NBLOCKS_PER_PAGE, NBLOCKS_PER_PAGE, pages_start + PAGE_SIZE * free_idx);

    return pages_start + PAGE_SIZE * free_idx;
}
