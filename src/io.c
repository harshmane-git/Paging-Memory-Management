#include "paging.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ─────────────────────────────────────────
// DATA BUFFER (local to IO)
// ─────────────────────────────────────────
char dataBuffer[MAX_DATA_LINES][MAX_LINE_LEN];
int dataCount = 0;
int dataPointer = 0;


// ═══════════════════════════════════════════════════════
// FUNCTION: loadIntoSecMemory
// ═══════════════════════════════════════════════════════
void loadIntoSecMemory(char *line) {
    int len = strlen(line);
    int i   = 0;

    while (i < len) {
        if (line[i] == 'H') {
            memset(secMemory[secMemPointer], '\0', FRAME_SIZE);
            secMemory[secMemPointer][0] = 'H';
            secMemPointer++;
            i++;
        } else {
            for (int col = 0; col < FRAME_SIZE && i < len; col++) {
                if (line[i] == 'H') break;
                secMemory[secMemPointer][col] = line[i++];
            }
            secMemPointer++;
        }
    }
}


// ═══════════════════════════════════════════════════════
// FUNCTION: printPageTable
// ═══════════════════════════════════════════════════════
void printPageTable() {
    printf("\n--- PAGE TABLE ---\n");
    for (int i = 0; i < TOTAL_PAGES; i++) {
        if (pageTable[i] != -1)
            printf("Page %2d -> Frame %2d\n", i, pageTable[i]);
        else
            printf("Page %2d -> Not Loaded\n", i);
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
// FUNCTION: printSecMemory
// ═══════════════════════════════════════════════════════
void printSecMemory() {
    printf("\n--- Secondary Memory ---\n");
    for (int i = 0; i < secMemPointer; i++) {
        printf("SecMem[%2d]: [%c%c%c%c]\n", i,
            secMemory[i][0] ? secMemory[i][0] : '.',
            secMemory[i][1] ? secMemory[i][1] : '.',
            secMemory[i][2] ? secMemory[i][2] : '.',
            secMemory[i][3] ? secMemory[i][3] : '.');
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
// FUNCTION: printMainMemory
// ═══════════════════════════════════════════════════════
void printMainMemory() {
    printf("\n--- Main Memory ---\n");
    for (int i = 0; i < MAIN_MEM_FRAMES; i++) {
        printf("Frame[%2d]: [%c%c%c%c]", i,
            mainMemory[i][0] ? mainMemory[i][0] : '.',
            mainMemory[i][1] ? mainMemory[i][1] : '.',
            mainMemory[i][2] ? mainMemory[i][2] : '.',
            mainMemory[i][3] ? mainMemory[i][3] : '.');

        if (frameOwnerPage[i] != -1)
            printf("  <- Page %d (last used %d)",
                   frameOwnerPage[i], frameLastUsed[i]);

        printf("\n");
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
// FUNCTION: printDataBuffer
// ═══════════════════════════════════════════════════════
void printDataBuffer() {
    printf("\n--- Data Buffer ---\n");
    for (int i = 0; i < dataCount; i++) {
        printf("Data[%d]: \"%s\"%s\n", i, dataBuffer[i],
               i < dataPointer ? " (read)" : "");
    }
    printf("\n");
}


// ═══════════════════════════════════════════════════════
// FUNCTION: readJobFile
// ═══════════════════════════════════════════════════════
void readJobFile() {
    FILE *fp = fopen("input/job_2.txt", "r");
    if (!fp) {
        printf("ERROR: Cannot open job_2.txt\n");
        return;
    }

    char line[MAX_LINE_LEN];
    int  inProgram = 0;
    int  inData    = 0;

    printf("===== Reading Job File =====\n\n");

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        printf("Read: %s\n", line);

        // $AMJ
        if (strncmp(line, "$AMJ", 4) == 0) {
            inProgram = 1;
            inData    = 0;
            currentSecStart = secMemPointer;
        }

        // $DTA
        else if (strncmp(line, "$DTA", 4) == 0) {
            inProgram = 0;
            inData    = 1;

            printf("\n--- Program Loaded ---\n");
            printSecMemory();
        }

        // $END
        else if (strncmp(line, "$END", 4) == 0) {
            printf("\n--- Execution Summary ---\n");
            printDataBuffer();
            printPageTable();
            printMainMemory();

            printf("Job Finished\n\n");

            inData = 0;
            resetForNextJob();
        }

        // Program section
        else if (inProgram) {
            loadIntoSecMemory(line);
        }

        // Data section
        else if (inData) {
            if (dataCount < MAX_DATA_LINES) {
                strncpy(dataBuffer[dataCount], line, MAX_LINE_LEN - 1);
                dataBuffer[dataCount][MAX_LINE_LEN - 1] = '\0';
                dataCount++;
            }
        }
    }

    fclose(fp);
}
