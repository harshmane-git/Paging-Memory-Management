#include <string.h>
#include "memory.h"
#include "address.h"
#include "stats.h"
#include "lru.h"

// ── PAGE FAULT HANDLER (LRU replacement) ─────────────────

void handlePageFault(int vp) {

    int frame = -1;

    // search for a free data frame
    for (int i = DATA_FRAME_START; i < MAIN_MEM_FRAMES; i++) {
        if (frameLastUsed[i] == 0) {
            frame = i;
            break;
        }
    }

    // no free frame — evict LRU frame
    if (frame == -1) {
        int min = frameLastUsed[DATA_FRAME_START];
        frame   = DATA_FRAME_START;

        for (int i = DATA_FRAME_START + 1; i < MAIN_MEM_FRAMES; i++) {
            if (frameLastUsed[i] < min) {
                min   = frameLastUsed[i];
                frame = i;
            }
        }

        // invalidate the evicted page in the page table
        for (int i = 0; i < TOTAL_PAGES; i++) {
            if (getPTE(i) == frame) {
                setPTE(i, INVALID_PAGE);
                currentStats.lruEvictions++;
                frameSource[frame] = UNSET;
                break;
            }
        }
    }

    // allocate a clean frame for the new page
    memset(mainMemory[frame], '\0', FRAME_SIZE);

    setPTE(vp, frame);
    frameLastUsed[frame] = ++timeCounter;
}