#ifndef MEMORY_H
#define MEMORY_H

// ── Memory constants ──────────────────────────────────────
#define FRAME_SIZE          5
#define INSTR_SIZE          4
#define MAIN_MEM_FRAMES     20
#define SEC_MEM_SIZE        100
#define TOTAL_PAGES         30

#define PT_FRAMES           6
#define DATA_FRAME_START    6
#define INVALID_PAGE        0xFF

#define MAX_DATA_LINES      50
#define MAX_LINE_LEN        100

// ── Frame source tracking ─────────────────────────────────
typedef enum { UNSET, GD_WRITTEN, SR_WRITTEN } FrameSource;

// ── Global variable externs ───────────────────────────────
extern char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
extern char secMemory[SEC_MEM_SIZE][INSTR_SIZE + 1];
extern FrameSource frameSource[MAIN_MEM_FRAMES];
extern int frameLastUsed[MAIN_MEM_FRAMES];
extern int timeCounter;
extern int countStats;

extern int IC;
extern int toggle;
extern char reg[FRAME_SIZE];

extern char outputFileName[50];
extern char dataBuffer[MAX_DATA_LINES][MAX_LINE_LEN];
extern int dataCount;
extern int dataPointer;
extern int secMemPointer;

// ── Memory functions ──────────────────────────────────────
void initMemory();
void resetForNextJob();

#endif