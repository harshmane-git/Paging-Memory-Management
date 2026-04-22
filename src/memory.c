#include <string.h>
#include "memory.h"

// ── Global definitions ────────────────────────────────────
char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
char secMemory[SEC_MEM_SIZE][INSTR_SIZE + 1];
FrameSource frameSource[MAIN_MEM_FRAMES];
int frameLastUsed[MAIN_MEM_FRAMES];
int timeCounter = 0;
int countStats = 1;

int IC = 0;
int toggle = 0;
char reg[FRAME_SIZE];

char outputFileName[50] = "job_output.txt";

char dataBuffer[MAX_DATA_LINES][MAX_LINE_LEN];
int dataCount = 0;
int dataPointer = 0;
int secMemPointer = 0;

// ── INIT ──────────────────────────────────────────────────
void initMemory() {

    memset(mainMemory, '.', sizeof(mainMemory));

    for (int i = 0; i < MAIN_MEM_FRAMES; i++)
        frameSource[i] = UNSET;

    // initialize page table frames (0 to PT_FRAMES-1) to INVALID
    for (int i = 0; i < PT_FRAMES; i++)
        memset(mainMemory[i], INVALID_PAGE, FRAME_SIZE);

    // initialize frameLastUsed — only data frames tracked
    for (int i = DATA_FRAME_START; i < MAIN_MEM_FRAMES; i++)
        frameLastUsed[i] = 0;

    timeCounter = 0;
}

// ── RESET ─────────────────────────────────────────────────
void resetForNextJob() {

    memset(secMemory, '\0', sizeof(secMemory));
    memset(dataBuffer, '\0', sizeof(dataBuffer));

    dataCount = 0;
    dataPointer = 0;
    secMemPointer = 0;

    IC = 0;
    toggle = 0;
    // page table reset handled by initMemory() via mainMemory frames
}