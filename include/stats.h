#ifndef STATS_H
#define STATS_H

// ── Job statistics struct ─────────────────────────────────
typedef struct {
    int jobId;
    int pageFaults;
    int pageHits;
    int lruEvictions;
    int totalInstructions;
    int ttlViolated;
    int tllViolated;
} JobStats;

extern JobStats currentStats;

// ── Violation limits ──────────────────────────────────────
#define TTL_LIMIT   50
#define TLL_LIMIT   10

// ── Functions ─────────────────────────────────────────────
void resetStats(int jobId);
void printStats();

#endif