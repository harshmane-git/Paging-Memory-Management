#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ─────────────────────────────────────────
//  MEMORY SPECIFICATIONS
// ─────────────────────────────────────────
#define FRAME_SIZE        4
#define MAIN_MEM_FRAMES   10
#define SEC_MEM_SIZE      100
#define TOTAL_PAGES       20
#define MAX_DATA_LINES    50    // max data lines per job
#define MAX_LINE_LEN      100   // max characters per line

// ─────────────────────────────────────────
//  MEMORY DECLARATIONS
// ─────────────────────────────────────────
char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
char secMemory[SEC_MEM_SIZE][FRAME_SIZE];

// pageTable[virtual_page] = frame number in main memory
// -1 means page NOT loaded yet → triggers page fault
int pageTable[TOTAL_PAGES];

// ─────────────────────────────────────────
//  LRU TRACKING
// ─────────────────────────────────────────
int timeCounter = 0;
int frameLastUsed[MAIN_MEM_FRAMES];
int frameOwnerPage[MAIN_MEM_FRAMES];

// ─────────────────────────────────────────
//  PCB
// ─────────────────────────────────────────
struct PCB {
    int TTL;
    int TLL;
    int jobId;
};
struct PCB pcb;

// ─────────────────────────────────────────
//  CPU REGISTERS & FLAGS
//  FIX 3,4,5: Added IC, register, toggle
// ─────────────────────────────────────────
int  IC     = 0;        // Instruction Counter — which instruction to execute next
char reg[FRAME_SIZE];   // CPU general purpose register (4 bytes)
int  toggle = 0;        // toggle flag — set by CR, used by BT

// ─────────────────────────────────────────
//  DATA BUFFER
//  FIX 2: Store data lines between $DTA and $END
// ─────────────────────────────────────────
char dataBuffer[MAX_DATA_LINES][MAX_LINE_LEN];
int  dataCount   = 0;   // how many data lines stored
int  dataPointer = 0;   // which data line to read next (for GD)

// ─────────────────────────────────────────
//  TRACKING VARIABLES
// ─────────────────────────────────────────
int secMemPointer   = 0;
int mainMemPointer  = 2;   // frames 0-1 = page table, user pages start at 2
int PTR             = 0;
int currentSecStart = 0;


// ═══════════════════════════════════════════════════════
//  FUNCTION: initMemory
// ═══════════════════════════════════════════════════════
void initMemory() {
    memset(mainMemory, '\0', sizeof(mainMemory));
    memset(secMemory,  '\0', sizeof(secMemory));
    memset(reg,        '\0', sizeof(reg));
    memset(dataBuffer, '\0', sizeof(dataBuffer));

    for (int i = 0; i < TOTAL_PAGES; i++)
        pageTable[i] = -1;

    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        frameLastUsed[i]  = -1;
        frameOwnerPage[i] = -1;
    }

    IC          = 0;
    toggle      = 0;
    dataCount   = 0;
    dataPointer = 0;

    printf("Memory initialized.\n");
    printf("  Main Memory  : %d frames x %d bytes = %d bytes\n",
           MAIN_MEM_FRAMES, FRAME_SIZE, MAIN_MEM_FRAMES * FRAME_SIZE);
    printf("  Sec  Memory  : %d frames x %d bytes\n", SEC_MEM_SIZE, FRAME_SIZE);
    printf("  Virtual Pages: %d\n", TOTAL_PAGES);
    printf("  Free Frames  : %d (frames %d to %d)\n\n",
           MAIN_MEM_FRAMES - mainMemPointer, mainMemPointer, MAIN_MEM_FRAMES - 1);
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: loadIntoSecMemory
//  FIX 1: H instruction now gets its own frame correctly
// ═══════════════════════════════════════════════════════
void loadIntoSecMemory(char *line) {
    int len = strlen(line);
    int i   = 0;

    printf("  Loading into SecMem at location %d: \"%s\"\n", secMemPointer, line);

    while (i < len) {
        // ── FIX 1: H gets its own frame ──────────────────
        if (line[i] == 'H') {
            // clear the frame first
            memset(secMemory[secMemPointer], '\0', FRAME_SIZE);
            // store H alone in this frame
            secMemory[secMemPointer][0] = 'H';
            secMemPointer++;
            i++;   // move past H
        }
        // ── Normal 4-byte instruction ─────────────────────
        else {
            for (int col = 0; col < FRAME_SIZE && i < len; col++) {
                // stop filling if next char is H
                if (line[i] == 'H') break;
                secMemory[secMemPointer][col] = line[i++];
            }
            secMemPointer++;
        }
    }
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printPageTable
// ═══════════════════════════════════════════════════════
void printPageTable() {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║              PAGE TABLE                  ║\n");
    printf("╠═════════════════════╦════════════════════╣\n");
    printf("║   Virtual Page      ║   Physical Frame   ║\n");
    printf("╠═════════════════════╬════════════════════╣\n");
    for (int i = 0; i < TOTAL_PAGES; i++) {
        if (pageTable[i] != -1)
            printf("║        %2d           ║        %2d          ║\n", i, pageTable[i]);
        else
            printf("║        %2d           ║       ---          ║\n", i);
    }
    printf("╚═════════════════════╩════════════════════╝\n\n");
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printSecMemory
// ═══════════════════════════════════════════════════════
void printSecMemory() {
    printf("\n--- Secondary Memory Contents ---\n");
    for (int i = 0; i < secMemPointer; i++) {
        printf("  SecMem[%2d]: [%c%c%c%c]\n", i,
            secMemory[i][0] ? secMemory[i][0] : '.',
            secMemory[i][1] ? secMemory[i][1] : '.',
            secMemory[i][2] ? secMemory[i][2] : '.',
            secMemory[i][3] ? secMemory[i][3] : '.');
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printMainMemory
// ═══════════════════════════════════════════════════════
void printMainMemory() {
    printf("\n--- Main Memory Contents ---\n");
    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        printf("  Frame[%2d]: [%c%c%c%c]", i,
            mainMemory[i][0] ? mainMemory[i][0] : '.',
            mainMemory[i][1] ? mainMemory[i][1] : '.',
            mainMemory[i][2] ? mainMemory[i][2] : '.',
            mainMemory[i][3] ? mainMemory[i][3] : '.');

        if (i < 2)
            printf("  <- Page Table");
        else if (frameOwnerPage[i] != -1)
            printf("  <- Virtual Page %d (last used: tick %d)",
                   frameOwnerPage[i], frameLastUsed[i]);
        printf("\n");
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printDataBuffer
// ═══════════════════════════════════════════════════════
void printDataBuffer() {
    printf("\n--- Data Buffer Contents ---\n");
    for (int i = 0; i < dataCount; i++) {
        printf("  Data[%d]: \"%s\"%s\n", i, dataBuffer[i],
               i < dataPointer ? " (already read)" : "");
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: handlePageFault
//  Demand Paging + LRU
// ═══════════════════════════════════════════════════════
void handlePageFault(int virtualPage) {
    int targetFrame = -1;

    // STEP 1: Find free frame
    for (int i = mainMemPointer; i < MAIN_MEM_FRAMES; i++) {
        if (frameOwnerPage[i] == -1) {
            targetFrame = i;
            break;
        }
    }

    // STEP 2: No free frame → LRU eviction
    if (targetFrame == -1) {
        printf("    [LRU] Memory full! Finding least recently used frame...\n");

        int lruTime = timeCounter + 1;
        for (int i = mainMemPointer; i < MAIN_MEM_FRAMES; i++) {
            if (frameLastUsed[i] < lruTime) {
                lruTime     = frameLastUsed[i];
                targetFrame = i;
            }
        }

        int evictedPage = frameOwnerPage[targetFrame];
        printf("    [LRU] Evicting Virtual Page %d from Frame %d (last used: tick %d)\n",
               evictedPage, targetFrame, lruTime);
        pageTable[evictedPage] = -1;
    }

    // STEP 3: Load page from secondary memory
    int secFrame = currentSecStart + virtualPage;
    if (secFrame < SEC_MEM_SIZE) {
        for (int col = 0; col < FRAME_SIZE; col++)
            mainMemory[targetFrame][col] = secMemory[secFrame][col];
    }

    // STEP 4: Update page table + LRU trackers
    pageTable[virtualPage]      = targetFrame;
    frameOwnerPage[targetFrame] = virtualPage;
    frameLastUsed[targetFrame]  = ++timeCounter;

    printf("    [DEMAND PAGING] Loaded Virtual Page %d -> Frame %d (tick %d)\n",
           virtualPage, targetFrame, timeCounter);
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: getPhysicalAddress
//  Virtual → Physical address translation
// ═══════════════════════════════════════════════════════
int getPhysicalAddress(int virtualAddress) {
    int virtualPage = virtualAddress / FRAME_SIZE;
    int offset      = virtualAddress % FRAME_SIZE;

    if (virtualPage >= TOTAL_PAGES) {
        printf("  ERROR: Invalid virtual address %d\n", virtualAddress);
        return -1;
    }

    if (pageTable[virtualPage] == -1) {
        printf("    -> PAGE FAULT! Page %d not in main memory.\n", virtualPage);
        handlePageFault(virtualPage);
    } else {
        int frame = pageTable[virtualPage];
        frameLastUsed[frame] = ++timeCounter;
        printf("    -> PAGE HIT! Page %d in Frame %d (tick %d)\n",
               virtualPage, frame, timeCounter);
    }

    int physicalFrame   = pageTable[virtualPage];
    int physicalAddress = physicalFrame * FRAME_SIZE + offset;

    printf("    -> Physical Address = %d\n", physicalAddress);
    return physicalAddress;
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: resetForNextJob
// ═══════════════════════════════════════════════════════
void resetForNextJob() {
    secMemPointer   = 0;
    mainMemPointer  = 2;
    currentSecStart = 0;
    timeCounter     = 0;
    IC              = 0;
    toggle          = 0;
    dataCount       = 0;
    dataPointer     = 0;

    memset(reg,        '\0', sizeof(reg));
    memset(mainMemory, '\0', sizeof(mainMemory));
    memset(dataBuffer, '\0', sizeof(dataBuffer));

    for (int i = 0; i < TOTAL_PAGES; i++)
        pageTable[i] = -1;
    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        frameOwnerPage[i] = -1;
        frameLastUsed[i]  = -1;
    }
}



// ═══════════════════════════════════════════════════════
//  FUNCTION: readJobFile
//  FIX 2: Data lines now stored in dataBuffer
// ═══════════════════════════════════════════════════════
void readJobFile() {
    FILE *fp = fopen("job_2.txt", "r");
    if (!fp) {
        printf("ERROR: Cannot open input.txt\n");
        return;
    }

    char line[MAX_LINE_LEN];
    int  inProgram = 0;
    int  inData    = 0;   // FIX 2: flag for data section

    printf("=======================================\n");
    printf("  Reading Job File\n");
    printf("=======================================\n\n");

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        printf("Read: \"%s\"\n", line);

        // ── $AMJ ───────────────────────────────────────
        if (strncmp(line, "$AMJ", 4) == 0) {
            char jobStr[5], ttlStr[5], tllStr[5];
            strncpy(jobStr, line + 4,  4); jobStr[4] = '\0';
            strncpy(ttlStr, line + 8,  4); ttlStr[4] = '\0';
            strncpy(tllStr, line + 12, 4); tllStr[4] = '\0';

            pcb.jobId = atoi(jobStr);
            pcb.TTL   = atoi(ttlStr);
            pcb.TLL   = atoi(tllStr);

            printf("\n JOB START\n");
            printf("  Job ID    : %d\n", pcb.jobId);
            printf("  Time Limit: %d\n", pcb.TTL);
            printf("  Line Limit: %d\n\n", pcb.TLL);

            inProgram       = 1;
            inData          = 0;
            currentSecStart = secMemPointer;
        }

        // ── $DTA ───────────────────────────────────────
        else if (strncmp(line, "$DTA", 4) == 0) {
            printf("\n $DTA - Program loaded into Secondary Memory.\n");
            printf(" Pages will load ON DEMAND during execution.\n\n");
            printSecMemory();

            inProgram = 0;
            inData    = 1;   // FIX 2: now collecting data lines
        }

        // ── $END ───────────────────────────────────────
        else if (strncmp(line, "$END", 4) == 0) {
            printf("\n Data Buffer collected:\n");
            printDataBuffer();

            printf(" Final Page Table:\n");
            printPageTable();
            printf(" Final Main Memory:\n");
            printMainMemory();

            printf(" $END - Job finished.\n");
            printf("=======================================\n\n");

            inData = 0;
            resetForNextJob();
        }

        // ── Program lines (between $AMJ and $DTA) ──────
        else if (inProgram) {
            loadIntoSecMemory(line);
        }

        // ── FIX 2: Data lines stored in dataBuffer ──────
        else if (inData) {
            if (dataCount < MAX_DATA_LINES) {
                strncpy(dataBuffer[dataCount], line, MAX_LINE_LEN - 1);
                dataBuffer[dataCount][MAX_LINE_LEN - 1] = '\0';
                printf("  [Data stored in buffer[%d]]: %s\n", dataCount, line);
                dataCount++;
            }
        }
    }

    fclose(fp);
}


// ═══════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════
int main() {
    initMemory();
    readJobFile();
    return 0;
}