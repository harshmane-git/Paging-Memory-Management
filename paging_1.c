#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FRAME_SIZE       4
#define MAIN_MEM_FRAMES  100
#define SEC_MEM_SIZE     100
#define TOTAL_PAGES      20

// ─────────────────────────────────────────
//  MEMORY DECLARATIONS
// ─────────────────────────────────────────
char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
char secMemory[SEC_MEM_SIZE][FRAME_SIZE];

int pageTable[TOTAL_PAGES];

// ─────────────────────────────────────────
//  PCB - Process Control Block
//  Stores job info read from $AMJ card
// ─────────────────────────────────────────
struct PCB {
    int TTL;        // Time Limit
    int TLL;        // Line Limit
    int jobId;      // Job ID from $AMJ
};
struct PCB pcb;

// ─────────────────────────────────────────
//  TRACKING VARIABLES
// ─────────────────────────────────────────
int secMemPointer  = 0;   // next free location in secondary memory
int mainMemPointer = 9;   // next free frame in main memory (0 is reserved for page table)
int PTR            = 0;   // Page Table Register - page table lives at frame 0


// ═══════════════════════════════════════════════════════
//  FUNCTION: initMemory
// ═══════════════════════════════════════════════════════
void initMemory() {
    memset(mainMemory, '\0', sizeof(mainMemory));
    memset(secMemory,  '\0', sizeof(secMemory));
    for (int i = 0; i < TOTAL_PAGES; i++) {
        pageTable[i] = -1;  // -1 = page not in main memory
    }
    printf(" Memory initialized.\n");
    printf("  Main Memory : %d frames × %d bytes\n", MAIN_MEM_FRAMES, FRAME_SIZE);
    printf("  Sec  Memory : %d frames × %d bytes\n", SEC_MEM_SIZE,    FRAME_SIZE);
    printf("  Virtual Pages: %d\n\n", TOTAL_PAGES);
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: loadIntoSecMemory
//  Loads one line of program text into secondary memory
//  frame by frame (4 bytes at a time)
// ═══════════════════════════════════════════════════════
void loadIntoSecMemory(char *line) {
    int len = strlen(line);
    int i = 0;

    printf("  Loading into SecMem starting at location %d: \"%s\"\n", secMemPointer, line);

    while (i < len) {
        for (int col = 0; col < FRAME_SIZE && i < len; col++) {
            secMemory[secMemPointer][col] = line[i++];
        }
        secMemPointer++;
    }
}


void buildPageTable(int secStart, int secEnd) {
    // secStart and secEnd are secondary memory frame indices
    // for the current job's program

    printf("\n--- Building Page Table ---\n");

    int virtualPage = 0;

    for (int sec = secStart; sec < secEnd; sec++) {
        // Assign a free frame in main memory
        int frame = mainMemPointer++;

        if (frame >= MAIN_MEM_FRAMES) {
            printf("  ERROR: Main memory is full!\n");
            return;
        }

        // Copy 4 bytes from secondary → main memory
        for (int col = 0; col < FRAME_SIZE; col++) {
            mainMemory[frame][col] = secMemory[sec][col];
        }

        // Record in page table
        pageTable[virtualPage] = frame;

        printf("  Virtual Page %2d  →  Physical Frame %2d  |  Content: [%c%c%c%c]\n",
            virtualPage, frame,
            mainMemory[frame][0] ? mainMemory[frame][0] : '.',
            mainMemory[frame][1] ? mainMemory[frame][1] : '.',
            mainMemory[frame][2] ? mainMemory[frame][2] : '.',
            mainMemory[frame][3] ? mainMemory[frame][3] : '.');

        virtualPage++;

        if (virtualPage >= TOTAL_PAGES) {
            printf("  WARNING: Exceeded max virtual pages (%d)!\n", TOTAL_PAGES);
            break;
        }
    }
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printPageTable
//  Prints the full page table nicely
// ═══════════════════════════════════════════════════════
void printPageTable() {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║           PAGE TABLE                 ║\n");
    printf("╠══════════════════════╦═══════════════╣\n");
    printf("║   Virtual Page       ║ Physical Frame║\n");
    printf("╠══════════════════════╬═══════════════╣\n");
    for (int i = 0; i < TOTAL_PAGES; i++) {
        if (pageTable[i] != -1) {
            printf("║        %2d            ║      %2d       ║\n", i, pageTable[i]);
        }
    }
    printf("╚══════════════════════╩═══════════════╝\n\n");
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: printSecMemory
//  Prints secondary memory contents
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
//  FUNCTION: getPhysicalAddress
//  Translates virtual address → physical address
//  This is for your OWN understanding — shows the concept
// ═══════════════════════════════════════════════════════
int getPhysicalAddress(int virtualAddress) {
    int virtualPage = virtualAddress / FRAME_SIZE;
    int offset      = virtualAddress % FRAME_SIZE;

    if (virtualPage >= TOTAL_PAGES || pageTable[virtualPage] == -1) {
        printf("  PAGE FAULT! Virtual page %d not in main memory.\n", virtualPage);
        return -1;
    }

    int physicalFrame   = pageTable[virtualPage];
    int physicalAddress = physicalFrame * FRAME_SIZE + offset;

    printf("  Virtual Address %d → Page %d, Offset %d → Frame %d → Physical Address %d\n",
           virtualAddress, virtualPage, offset, physicalFrame, physicalAddress);

    return physicalAddress;
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: readJobFile
//  Reads input.txt line by line
//  Handles $AMJ, $DTA, $END markers
// ═══════════════════════════════════════════════════════
void readJobFile() {
    FILE *fp = fopen("../Phase_3/job_3.txt", "r");
    if (!fp) {
        printf("ERROR: Cannot open input.txt\n");
        return;
    }

    char line[100];
    int  inProgram  = 0;   // 1 = currently reading program lines
    int  secStart   = 0;   // where this job's program starts in sec memory

    printf("═══════════════════════════════════════\n");
    printf("  Reading Job File (input.txt)\n");
    printf("═══════════════════════════════════════\n\n");

    while (fgets(line, sizeof(line), fp)) {
        // Remove newline character at end
        line[strcspn(line, "\n")] = '\0';

        printf("Read line: \"%s\"\n", line);

        // ── $AMJ card ──────────────────────────────
        if (strncmp(line, "$AMJ", 4) == 0) {
            // Format: $AMJjjjjttttllll
            // jjjj = job id, tttt = time limit, llll = line limit
            char jobStr[5], ttlStr[5], tllStr[5];
            strncpy(jobStr, line + 4, 4); jobStr[4] = '\0';
            strncpy(ttlStr, line + 8, 4); ttlStr[4] = '\0';
            strncpy(tllStr, line + 12, 4); tllStr[4] = '\0';

            pcb.jobId = atoi(jobStr);
            pcb.TTL   = atoi(ttlStr);
            pcb.TLL   = atoi(tllStr);

            printf("\n JOB START\n");
            printf("  Job ID    : %d\n", pcb.jobId);
            printf("  Time Limit: %d\n", pcb.TTL);
            printf("  Line Limit: %d\n\n", pcb.TLL);

            inProgram = 1;
            secStart  = secMemPointer;  // mark where this job starts in sec memory
        }

        // ── $DTA card ──────────────────────────────
        else if (strncmp(line, "$DTA", 4) == 0) {
            printf("\n $DTA found — Program loading done. Data section starts.\n");

            // Program is fully in secondary memory now
            // Build the page table from secStart to current secMemPointer
            int secEnd = secMemPointer;
            printSecMemory();
            buildPageTable(secStart, secEnd);
            printPageTable();

            inProgram = 0;
        }

        // ── $END card ──────────────────────────────
        else if (strncmp(line, "$END", 4) == 0) {
            printf(" $END found — Job finished.\n\n");
            printf("═══════════════════════════════════════\n\n");

            // Reset for next job
            secMemPointer  = 0;
            mainMemPointer = 1;
            for (int i = 0; i < TOTAL_PAGES; i++) pageTable[i] = -1;
        }

        // ── Program lines (between $AMJ and $DTA) ──
        else if (inProgram) {
            loadIntoSecMemory(line);
        }

        // ── Data lines (between $DTA and $END) ─────
        else {
            printf("  [Data line - ignored for now]: %s\n", line);
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

    // Demo: show address translation
    printf("--- Address Translation Demo ---\n");
    getPhysicalAddress(0);   // first byte of virtual memory
    getPhysicalAddress(4);   // second virtual page, first byte
    getPhysicalAddress(8);   // third virtual page

    return 0;
}
void LRU(){
    int arr[100];
    

}