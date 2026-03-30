#include "paging.h"
#include <stdio.h>
#include <string.h>

// Memory
char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
char secMemory[SEC_MEM_SIZE][FRAME_SIZE];
int pageTable[TOTAL_PAGES];

// LRU
int frameLastUsed[MAIN_MEM_FRAMES];
int frameOwnerPage[MAIN_MEM_FRAMES];
int timeCounter = 0;

// Pointers
int mainMemPointer = 2;
int secMemPointer = 0;
int currentSecStart = 0;

void initMemory() {
    memset(mainMemory, '\0', sizeof(mainMemory));
    memset(secMemory, '\0', sizeof(secMemory));

    for (int i = 0; i < TOTAL_PAGES; i++)
        pageTable[i] = -1;

    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        frameLastUsed[i] = -1;
        frameOwnerPage[i] = -1;
    }

    printf("Memory Initialized\n");
}

void resetForNextJob() {
    secMemPointer = 0;
    mainMemPointer = 2;
    currentSecStart = 0;
    timeCounter = 0;

    memset(mainMemory, '\0', sizeof(mainMemory));

    for (int i = 0; i < TOTAL_PAGES; i++)
        pageTable[i] = -1;

    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        frameOwnerPage[i] = -1;
        frameLastUsed[i] = -1;
    }
}
