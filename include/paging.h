#ifndef PAGING_H
#define PAGING_H

// ─────────────────────────────────────────
// CONSTANTS
// ─────────────────────────────────────────
#define FRAME_SIZE        4
#define MAIN_MEM_FRAMES   10
#define SEC_MEM_SIZE      100
#define TOTAL_PAGES       20
#define MAX_DATA_LINES    50
#define MAX_LINE_LEN      100

// ─────────────────────────────────────────
// MEMORY (extern = defined in memory.c)
// ─────────────────────────────────────────
extern char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
extern char secMemory[SEC_MEM_SIZE][FRAME_SIZE];
extern int pageTable[TOTAL_PAGES];

// ─────────────────────────────────────────
// LRU
// ─────────────────────────────────────────
extern int frameLastUsed[MAIN_MEM_FRAMES];
extern int frameOwnerPage[MAIN_MEM_FRAMES];
extern int timeCounter;

// ─────────────────────────────────────────
// POINTERS
// ─────────────────────────────────────────
extern int mainMemPointer;
extern int secMemPointer;
extern int currentSecStart;

// ─────────────────────────────────────────
// FUNCTIONS
// ─────────────────────────────────────────
void initMemory();
void resetForNextJob();

void loadIntoSecMemory(char *line);
void printPageTable();
void printSecMemory();
void printMainMemory();
void printDataBuffer();

void handlePageFault(int virtualPage);
int getPhysicalAddress(int virtualAddress);

void readJobFile();

#endif
