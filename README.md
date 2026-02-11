# SkipStore 🚀

**SkipStore** is a high-performance, persistent distributed Key-Value store written from scratch in C++. It utilizes a **Log-Structured Merge (LSM) Tree** architecture, a custom **Probabilistic Skip List**, and **Bloom Filters** to provide efficient data storage and retrieval.

## 🛠️ Key Technical Features

- **LSM-Tree Architecture:** Designed for high write throughput by utilizing append-only logs and immutable on-disk storage (SSTables).
- **Custom Skip List:** Implemented a randomized, multi-level Skip List as the primary in-memory data structure (MemTable), replacing standard STL containers for better scalability.
- **Crash Durability (WAL):** Integrated a **Write-Ahead Log (WAL)** to ensure ACID-compliant durability, allowing for full data recovery after unexpected system failures.
- **Read Optimization:** Utilizes **Bloom Filters** to prevent unnecessary disk I/O, allowing the system to skip 99% of disk seeks for non-existent keys.
- **Networking:** Features a custom TCP server/client architecture built on the Windows Sockets (Winsock) API.
- **Automated Compaction:** Background process to merge multiple SSTables and reclaim storage space, mitigating read amplification.

## 📊 Performance

Based on internal benchmarks using the included C++ stress-testing tool:

- **Synchronous Write Throughput:** ~3,550+ requests per second.
- **Latency:** Optimized via in-memory buffering and sequential disk I/O.

## 📁 Project Structure

- `SkipList.h`: Custom implementation of the Probabilistic Skip List.
- `KVStore.h`: The core engine handling LSM logic, WAL, and Bloom Filters.
- `server_win.cpp`: TCP Server implementation for remote database access.
- `client_win.cpp`: Interactive CLI client for manual database operations.
- `benchmark.cpp`: High-speed stress-testing tool to measure throughput.

## 🚀 Getting Started

### Prerequisites

- Windows OS
- MinGW-w64 (g++) or Visual Studio

### Compilation

Use the following commands to compile the system:

```bash
# Compile the Server
g++ server_win.cpp -o server -lws2_32

# Compile the Client
g++ client_win.cpp -o client -lws2_32

# Compile the Benchmark Tool
g++ benchmark.cpp -o benchmark -lws2_32
```
