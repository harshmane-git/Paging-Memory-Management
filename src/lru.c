#include "paging.h"
#include <stdio.h>

void handlePageFault(int virtualPage) {
    int targetFrame = -1;

    // Find free frame
    for (int i = mainMemPointer; i < MAIN_MEM_FRAMES; i++) {
        if (frameOwnerPage[i] == -1) {
            targetFrame = i;
            break;
        }
    }

    // LRU replacement
    if (targetFrame == -1) {
        int lruTime = timeCounter + 1;

        for (int i = mainMemPointer; i < MAIN_MEM_FRAMES; i++) {
            if (frameLastUsed[i] < lruTime) {
                lruTime = frameLastUsed[i];
                targetFrame = i;
            }
        }

        int evictedPage = frameOwnerPage[targetFrame];
        pageTable[evictedPage] = -1;

        printf("LRU Evicted Page %d\n", evictedPage);
    }

    int secFrame = currentSecStart + virtualPage;

    for (int i = 0; i < FRAME_SIZE; i++)
        mainMemory[targetFrame][i] = secMemory[secFrame][i];

    pageTable[virtualPage] = targetFrame;
    frameOwnerPage[targetFrame] = virtualPage;
    frameLastUsed[targetFrame] = ++timeCounter;

    printf("Loaded Page %d into Frame %d\n", virtualPage, targetFrame);
}
