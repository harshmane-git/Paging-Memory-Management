#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "address.h"
#include "lru.h"
#include "executor.h"
#include "stats.h"

// ── READ JOB FILE ─────────────────────────────────────────

void readJobFile(char *filename) {

    FILE *fp = fopen(filename, "r");
    if (!fp) exit(1);

    char line[MAX_LINE_LEN];
    int  readingData = 0;

    while (fgets(line, sizeof(line), fp)) {

        line[strcspn(line, "\n")] = '\0';

        // $AMJ — start of new job
        if (strncmp(line, "$AMJ", 4) == 0) {

            resetForNextJob();
            initMemory();
            readingData = 0;

            int jobId;
            sscanf(line + 4, "%4d", &jobId);
            resetStats(jobId);

            FILE *out = fopen(outputFileName, "a");
            fclose(out);
        }

        // $DTA — start of data section
        else if (strncmp(line, "$DTA", 4) == 0) {
            readingData  = 1;
            dataPointer  = 0;
            dataCount    = 0;
        }

        // $END — end of job, execute and print stats
        else if (strncmp(line, "$END", 4) == 0) {
            executeUserProgram();
            printStats();

            FILE *out = fopen(outputFileName, "a");
            fprintf(out, "\n\n");
            fclose(out);
        }

        // DATA lines
        else if (readingData) {
            strncpy(dataBuffer[dataCount], line, MAX_LINE_LEN - 1);
            dataBuffer[dataCount][MAX_LINE_LEN - 1] = '\0';
            dataCount++;
        }

        // PROGRAM lines
        else {
            int i   = 0;
            int len = (int)strlen(line);

            while (i < len) {

                char instr[INSTR_SIZE + 1] = {0};

                if (line[i] == 'H') {
                    memset(instr, '\0', INSTR_SIZE + 1);
                    instr[0] = 'H';
                    i++;
                } else {
                    for (int j = 0; j < INSTR_SIZE && i < len; j++, i++)
                        instr[j] = line[i];
                }

                strncpy(secMemory[secMemPointer], instr, INSTR_SIZE);
                secMemory[secMemPointer][INSTR_SIZE] = '\0';
                secMemPointer++;
            }
        }
    }

    fclose(fp);
}

// ── MAIN ──────────────────────────────────────────────────

int main(int argc, char *argv[]) {

    if (argc < 2) return 1;

    // clear output file once at start
    FILE *out = fopen("job_output.txt", "w");
    fclose(out);

    readJobFile(argv[1]);
    return 0;
}