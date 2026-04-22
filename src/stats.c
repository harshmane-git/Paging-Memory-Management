#include <stdio.h>
#include "stats.h"

JobStats currentStats;

// ── RESET STATS ───────────────────────────────────────────

void resetStats(int jobId) {
    currentStats.jobId             = jobId;
    currentStats.pageFaults        = 0;
    currentStats.pageHits          = 0;
    currentStats.lruEvictions      = 0;
    currentStats.totalInstructions = 0;
    currentStats.ttlViolated       = 0;
    currentStats.tllViolated       = 0;
}

// ── PRINT STATS ───────────────────────────────────────────

void printStats() {
    printf("--- Job %04d Statistics ---\n",  currentStats.jobId);
    printf("Total Instructions Executed : %d\n", currentStats.totalInstructions);
    printf("Page Faults                 : %d\n", currentStats.pageFaults);
    printf("Page Hits                   : %d\n", currentStats.pageHits);
    printf("LRU Evictions               : %d\n", currentStats.lruEvictions);
    printf("TTL Violated                : %s\n", currentStats.ttlViolated ? "YES" : "NO");
    printf("TLL Violated                : %s\n", currentStats.tllViolated ? "YES" : "NO");
    printf("--------------------------\n\n");
}