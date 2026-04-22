#include <ctype.h>
#include "memory.h"
#include "address.h"
#include "stats.h"
#include "lru.h"

// ── PAGE TABLE ACCESS ─────────────────────────────────────

// Write page table entry: virtual page vp → physical frame f
void setPTE(int vp, int f) {
    int ptFrame  = vp / FRAME_SIZE;
    int ptOffset = vp % FRAME_SIZE;
    mainMemory[ptFrame][ptOffset] = (char)f;
}

// Read page table entry for virtual page vp
// Returns -1 if invalid, else frame number
int getPTE(int vp) {
    int ptFrame  = vp / FRAME_SIZE;
    int ptOffset = vp % FRAME_SIZE;
    unsigned char entry = (unsigned char)mainMemory[ptFrame][ptOffset];
    if (entry == INVALID_PAGE) return -1;
    return (int)entry;
}

// ── ADDRESS TRANSLATION ───────────────────────────────────

int getPhysicalAddress(int va) {

    if (va < 0 || va >= TOTAL_PAGES * FRAME_SIZE)
        return -1;

    int vp     = va / FRAME_SIZE;
    int offset = va % FRAME_SIZE;

    if (getPTE(vp) == -1) {
        handlePageFault(vp);
        if (countStats) currentStats.pageFaults++;
    } else {
        if (countStats) currentStats.pageHits++;
    }

    int frame = getPTE(vp);
    frameLastUsed[frame] = ++timeCounter;

    return frame * FRAME_SIZE + offset;
}

// ── INSTRUCTION ADDRESS PARSING ───────────────────────────

int getAddress(char inst[]) {
    if (!isdigit(inst[2]) || !isdigit(inst[3]))
        return -1;
    return (inst[2] - '0') * 10 + (inst[3] - '0');
}