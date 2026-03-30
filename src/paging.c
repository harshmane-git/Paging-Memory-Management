#include "paging.h"
#include <stdio.h>

int getPhysicalAddress(int virtualAddress) {
    int virtualPage = virtualAddress / FRAME_SIZE;
    int offset = virtualAddress % FRAME_SIZE;

    if (virtualPage >= TOTAL_PAGES) {
        printf("Invalid address\n");
        return -1;
    }

    if (pageTable[virtualPage] == -1) {
        printf("PAGE FAULT on page %d\n", virtualPage);
        handlePageFault(virtualPage);
    } else {
        int frame = pageTable[virtualPage];
        frameLastUsed[frame] = ++timeCounter;
    }

    return pageTable[virtualPage] * FRAME_SIZE + offset;
}
