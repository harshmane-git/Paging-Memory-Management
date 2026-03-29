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