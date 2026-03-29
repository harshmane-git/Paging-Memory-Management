#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ─────────────────────────────────────────
//  MEMORY SPECIFICATIONS (STRESSED LRU)
// ─────────────────────────────────────────
#define FRAME_SIZE       4
#define MAIN_MEM_FRAMES  100  // 95 available for user pages (0-4 are reserved)
#define SEC_MEM_SIZE     200  // Increased to hold more dummy data
#define TOTAL_PAGES      120  // Increased so Virtual > Physical (Forces LRU)

// ─────────────────────────────────────────
//  MEMORY DECLARATIONS
// ─────────────────────────────────────────
char mainMemory[MAIN_MEM_FRAMES][FRAME_SIZE];
char secMemory[SEC_MEM_SIZE][FRAME_SIZE];

// pageTable[virtual_page] = frame number in main memory
// -1 means that page is NOT loaded in main memory yet
int pageTable[TOTAL_PAGES];

// ─────────────────────────────────────────
//  LRU TRACKING VARIABLES
// ─────────────────────────────────────────
int timeCounter = 0;                  // Acts as our logical clock
int frameLastUsed[MAIN_MEM_FRAMES];   // Tracks the last access time of each frame
int frameOwnerPage[MAIN_MEM_FRAMES];  // Maps a frame back to its virtual page

// ─────────────────────────────────────────
//  PCB - Process Control Block
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
int secMemPointer  = 0;   
int mainMemPointer = 5;   // 0-4 are reserved for OS/Page Table
int PTR            = 0;   
int currentSecStart = 0;  // Tracks where current job starts in sec memory


// ═══════════════════════════════════════════════════════
//  FUNCTION: initMemory
// ═══════════════════════════════════════════════════════
void initMemory() {
    memset(mainMemory, '\0', sizeof(mainMemory));
    memset(secMemory,  '\0', sizeof(secMemory));
    
    for (int i = 0; i < TOTAL_PAGES; i++) {
        pageTable[i] = -1;  
    }
    
    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        frameLastUsed[i] = -1;
        frameOwnerPage[i] = -1;
    }

    printf("✔ Memory initialized.\n");
    printf("  Main Memory : %d frames × %d bytes\n", MAIN_MEM_FRAMES, FRAME_SIZE);
    printf("  Sec  Memory : %d frames × %d bytes\n", SEC_MEM_SIZE, FRAME_SIZE);
    printf("  Virtual Pgs : %d\n\n", TOTAL_PAGES);
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: loadIntoSecMemory
// ═══════════════════════════════════════════════════════
void loadIntoSecMemory(char *line) {
    int len = strlen(line);
    int i = 0;
    while (i < len) {
        for (int col = 0; col < FRAME_SIZE && i < len; col++) {
            secMemory[secMemPointer][col] = line[i++];
        }
        secMemPointer++;
    }
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: handlePageFault (Demand Paging & LRU Logic)
// ═══════════════════════════════════════════════════════
void handlePageFault(int virtualPage) {
    int targetFrame = -1;

    // 1. Look for an empty frame first (starting from index 5)
    for (int i = 5; i < MAIN_MEM_FRAMES; i++) { 
        if (frameOwnerPage[i] == -1) {
            targetFrame = i;
            break;
        }
    }

    // 2. If memory is full, execute LRU Eviction
    if (targetFrame == -1) {
        int lruTime = timeCounter + 1; 
        
        for (int i = 5; i < MAIN_MEM_FRAMES; i++) {
            if (frameLastUsed[i] < lruTime) {
                lruTime = frameLastUsed[i];
                targetFrame = i;
            }
        }
        
        int evictedPage = frameOwnerPage[targetFrame];
        printf("    [LRU ALERT] Memory Full! Evicting Virtual Page %d from Frame %d\n", evictedPage, targetFrame);
        pageTable[evictedPage] = -1; // Remove evicted page from page table
    }

    // 3. Load the new page from Secondary Memory into the target frame
    int secFrame = currentSecStart + virtualPage;
    // Safety check so we don't read past secondary memory
    if (secFrame < SEC_MEM_SIZE) { 
        for (int col = 0; col < FRAME_SIZE; col++) {
            mainMemory[targetFrame][col] = secMemory[secFrame][col];
        }
    }

    // 4. Update the Page Table and LRU trackers
    pageTable[virtualPage] = targetFrame;
    frameOwnerPage[targetFrame] = virtualPage;
    frameLastUsed[targetFrame] = ++timeCounter; 

    printf("    [PAGING] Loaded Virtual Page %d into Frame %d\n", virtualPage, targetFrame);
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: getPhysicalAddress
// ═══════════════════════════════════════════════════════
int getPhysicalAddress(int virtualAddress) {
    int virtualPage = virtualAddress / FRAME_SIZE;
    int offset      = virtualAddress % FRAME_SIZE;

    if (virtualPage >= TOTAL_PAGES) {
        printf("  ERROR: Invalid virtual page %d.\n", virtualPage);
        return -1;
    }

    printf("\n  Accessing Virtual Address %d (Page %d, Offset %d)...\n", virtualAddress, virtualPage, offset);

    if (pageTable[virtualPage] == -1) {
        printf("    -> PAGE FAULT! Page not in main memory.\n");
        handlePageFault(virtualPage);
    } else {
        int frame = pageTable[virtualPage];
        frameLastUsed[frame] = ++timeCounter; // Update LRU timestamp
        printf("    -> PAGE HIT! Page is already in Frame %d.\n", frame);
    }

    int physicalFrame   = pageTable[virtualPage];
    int physicalAddress = physicalFrame * FRAME_SIZE + offset;

    printf("    -> Translated to Physical Address %d\n", physicalAddress);
    return physicalAddress;
}


// ═══════════════════════════════════════════════════════
//  FUNCTION: readJobFile
// ═══════════════════════════════════════════════════════
void readJobFile() {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("WARNING: input.txt not found. Simulating data load into Sec Mem...\n");
        currentSecStart = 0;
        char dummyData[] = "ABCD1234EFGH5678IJKL9012MNOP"; 
        loadIntoSecMemory(dummyData);
        return;
    }

    char line[100];
    int  inProgram  = 0;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "$AMJ", 4) == 0) {
            char jobStr[5]; strncpy(jobStr, line + 4, 4); jobStr[4] = '\0';
            pcb.jobId = atoi(jobStr);
            inProgram = 1;
            currentSecStart = secMemPointer; 
            printf("✔ JOB START (ID: %d)\n", pcb.jobId);
        }
        else if (strncmp(line, "$DTA", 4) == 0) {
            inProgram = 0;
            printf("✔ Program loaded to Secondary Memory. Ready for Execution.\n");
        }
        else if (strncmp(line, "$END", 4) == 0) {
            printf("✔ JOB END\n");
            secMemPointer  = 0;
            currentSecStart = 0;
            timeCounter = 0;
            for (int i = 0; i < TOTAL_PAGES; i++) pageTable[i] = -1;
            for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
                frameOwnerPage[i] = -1;
                frameLastUsed[i] = -1;
            }
        }
        else if (inProgram) {
            loadIntoSecMemory(line);
        }
    }
    fclose(fp);
}


// ═══════════════════════════════════════════════════════
//  MAIN (Automated Stress Test)
// ═══════════════════════════════════════════════════════
int main() {
    initMemory();
    readJobFile();

    printf("\n═══════════════════════════════════════\n");
    printf("  Automated LRU Stress Test\n");
    printf("═══════════════════════════════════════\n");

    // 1. Fill up all 95 available frames (Frames 5 through 99)
    printf("\n--- Filling Memory (Loading Pages 0 to 94) ---\n");
    for (int i = 0; i < 95; i++) {
        // Multiplied by 4 because FRAME_SIZE is 4. 
        getPhysicalAddress(i * 4); 
    }

    // 2. Access Page 0 again to make it the MOST recently used (Page Hit)
    printf("\n--- Triggering a Page Hit on Page 0 ---\n");
    getPhysicalAddress(0); 

    // 3. Request Page 95. Memory is now full! 
    // It should evict Page 1 (from Frame 6) because Page 0 just got a fresh timestamp.
    printf("\n--- Forcing LRU Eviction (Requesting Page 95) ---\n");
    getPhysicalAddress(95 * 4); 

    return 0;
}