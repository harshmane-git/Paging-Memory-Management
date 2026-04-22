#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "executor.h"
#include "memory.h"
#include "address.h"
#include "stats.h"

extern char mainMemory[][FRAME_SIZE];
extern char secMemory[][INSTR_SIZE + 1];
extern char outputFileName[50];

extern int IC, toggle;
extern int countStats;

extern char dataBuffer[][MAX_LINE_LEN];
extern int dataCount, dataPointer;

extern char reg[FRAME_SIZE];
extern FrameSource frameSource[];

FILE *outFile = NULL;

// valid opcodes table
static const char *validOpcodes[] = {"GD","PD","LR","SR","CR","BT", NULL};

// ── TERMINATE ─────────────────────────────────────────────

void terminate(char *msg) {
    fprintf(stderr, "[TERMINATE] %s\n", msg);
    if (outFile) {
        fprintf(outFile, "Error: %s\n", msg);   // write error to output file too
        fclose(outFile);
        outFile = NULL;
    }
}

// ── FETCH INSTRUCTION ─────────────────────────────────────

int fetchInstruction(int ic, char inst[]) {

    if (ic < 0 || ic >= SEC_MEM_SIZE)
        return -1;

    if (secMemory[ic][0] == '\0')
        return -1;

    strncpy(inst, secMemory[ic], INSTR_SIZE);
    inst[INSTR_SIZE] = '\0';

    return 0;
}

// ── EXECUTE USER PROGRAM ──────────────────────────────────

void executeUserProgram() {

    outFile = fopen(outputFileName, "a");

    while (1) {

        char inst[INSTR_SIZE + 1] = {0};

        if (fetchInstruction(IC, inst) == -1) {
            terminate("Invalid instruction fetch");
            break;
        }

        if (inst[0] == 'H')
            break;

        currentStats.totalInstructions++;

        // TTL violation check
        if (currentStats.totalInstructions > TTL_LIMIT) {
            currentStats.ttlViolated = 1;
            terminate("TTL exceeded");
            break;
        }

        //  opcode validation (PI=1) ──────────────
        char opcode[3] = {inst[0], inst[1], '\0'};
        int validOp = 0;
        for (int i = 0; validOpcodes[i] != NULL; i++) {
            if (strcmp(opcode, validOpcodes[i]) == 0) {
                validOp = 1;
                break;
            }
        }
        if (!validOp) {
            fprintf(outFile, "Error: Invalid opcode '%s' (PI=1)\n", opcode);
            terminate("Invalid opcode (PI=1)");
            break;
        }

        // ──  operand validation (PI=2) ─────────────
        int addr = getAddress(inst);
        if (addr == -1) {
            fprintf(outFile, "Error: Invalid operand in '%s' (PI=2)\n", inst);
            terminate("Invalid operand (PI=2)");
            break;
        }

        int pa = getPhysicalAddress(addr);
        if (pa == -1) {
            terminate("Invalid address");
            break;
        }

        int frame = pa / FRAME_SIZE;

        // GD ── read data into memory
        if (strncmp(inst, "GD", 2) == 0) {

            if (dataPointer >= dataCount) {
                terminate("Out of data");
                break;
            }

            char *data    = dataBuffer[dataPointer];
            int   dataLen = (int)strlen(data);

            countStats = 0;
            for (int i = 0; i < dataLen; i++) {
                int curPA = getPhysicalAddress(addr + i);
                if (curPA == -1) break;
                int curFrame = curPA / FRAME_SIZE;
                mainMemory[curFrame][curPA % FRAME_SIZE] = data[i];
                frameSource[curFrame] = GD_WRITTEN;
            }
            countStats = 1;

            dataPointer++;
        }

        // PD ── print data from memory
        else if (strncmp(inst, "PD", 2) == 0) {

            if (frameSource[frame] == SR_WRITTEN) {

                // PATTERN MODE: one frame, each byte = 4 chars wide, null → spaces
                int charsWritten = 0;
                for (int i = 0; i < FRAME_SIZE; i++) {
                    char c = (mainMemory[frame][i] == '\0') ? ' ' : mainMemory[frame][i];
                    if (i < FRAME_SIZE - 1) {
                        fprintf(outFile, "%c   ", c);
                        charsWritten += 4;
                    } else {
                        fprintf(outFile, "%c", c);
                        charsWritten++;
                    }
                }
                while (charsWritten < 20) {
                    fprintf(outFile, " ");
                    charsWritten++;
                }
                fprintf(outFile, "\n");

            } else {

                // ── TEXT MODE — stop at FIRST null ─
                int charsWritten = 0;
                int i            = 0;

                countStats = 0;
                while (1) {
                    int curPA = getPhysicalAddress(addr + i);
                    if (curPA == -1) break;
                    char b = mainMemory[curPA / FRAME_SIZE][curPA % FRAME_SIZE];
                    if (b == '\0') break;        // stop at first null
                    fprintf(outFile, "%c", b);
                    charsWritten++;
                    i++;
                }
                countStats = 1;

                while (charsWritten < 20) {
                    fprintf(outFile, " ");
                    charsWritten++;
                }
                fprintf(outFile, "\n");
            }
        }

        // LR ── load register
        else if (strncmp(inst, "LR", 2) == 0) {
            memcpy(reg, mainMemory[frame], FRAME_SIZE);
        }

        // SR ── store register byte at specific offset
        else if (strncmp(inst, "SR", 2) == 0) {
            int offset = pa % FRAME_SIZE;
            mainMemory[frame][offset] = reg[0];
            frameSource[frame] = SR_WRITTEN;
        }

        // CR ── compare register with memory
        else if (strncmp(inst, "CR", 2) == 0) {
            toggle = (memcmp(reg, mainMemory[frame], FRAME_SIZE) == 0);
        }

        // BT ── branch if toggle set
        else if (strncmp(inst, "BT", 2) == 0) {
            if (toggle)
                IC = addr - 1;
        }

        else {
            terminate("Invalid instruction");
            break;
        }

        IC++;
    }

    if (outFile) {
        fclose(outFile);
        outFile = NULL;
    }
}