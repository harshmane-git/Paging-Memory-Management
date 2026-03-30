# Paging Memory Management System

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**A modular C implementation of Operating System memory management** featuring **Paging**, **Demand Paging**, **Page Fault Handling**, and the **LRU (Least Recently Used)** page replacement algorithm.

This simulator demonstrates how the OS handles virtual-to-physical address translation, loads pages on demand, and efficiently manages limited main memory using the LRU policy.

---

## 📌 Features

- Fixed-size paging with page table mapping
- Demand Paging (pages loaded only when accessed)
- Page fault detection and handling
- LRU page replacement algorithm (with time-based tracking)
- Secondary memory simulation (backing store)
- Logical to physical address translation
- Clean modular design (easy to extend with FIFO/Optimal later)

---

## 🧠 Concepts Demonstrated

- Virtual Memory Management
- Page Table and Frame Management
- Demand Paging Mechanism
- Page Fault Service Routine
- Page Replacement Strategies (LRU)
- Memory Hierarchy (Main Memory vs Secondary Memory)

---

## 🛠️ Tech Stack

- **Language**: C
- **Compiler**: GCC
- **Build Tool**: Makefile
- **Platform**: Linux (Ubuntu tested)

---

## 📂 Project Structure

```bash
Paging-Memory-Management/
├── src/           # Core source files (main logic, LRU, memory handling)
├── include/       # Header files (constants, declarations)
├── input/         # Sample job/input files for testing
├── output/        # Generated output files (optional)
├── Makefile       # Build instructions
├── .gitignore
└── README.md


▶️ How to Build and Run

1. Clone & Checkout Dev Branch
    Bashgit clone https://github.com/harshmane-git/Paging-Memory-Management.git
    cd Paging-Memory-Management
    git checkout dev

2. Compile
    Bash
    make

3. Run
    Bash
    ./paging_sim input/<your_input_file>.txt
    Replace <your_input_file>.txt with any file from the input/ directory.

4. Clean
    Bash
    make clean

🔄 How It Works

1.  Initializes main memory, secondary memory,and page table
2.  Reads job file and loads data/instructions into secondary memory
3.  Translates virtual addresses to physical addresses
4.  Triggers page fault when a page is not in main memory
5.  Handles page fault by loading the required page from secondary memory
6.  Uses LRU algorithm to evict the least recently used page when main memory is full
7.  Updates page table and displays memory state

📊 Sample Output
    textPAGE FAULT on page 3
    Loaded Page 3 into Frame 7
    LRU Evicted Page 2
    Loaded Page 5 into Frame 2

🔍 Key Highlights

    Efficient memory utilization through Demand Paging
    Realistic page fault handling and LRU replacement
    Modular design – easy to extend with new replacement policies (FIFO, Optimal, etc.)
    Good educational tool for understanding OS memory management
