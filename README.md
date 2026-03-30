# Matching engine

A  backtesting engine for NASDAQ ITCH market data, built from scratch in C++ to explore systems programming, lock-free data structures, and high-performance computing.

## Project Goals

 Implement low-level systems components without relying on high-level libraries
 Optimize for nanosecond-level latency using lock-free algorithms and cache-aware design
 Learn systems programming skills for HFT/quantitative trading roles


**Upcoming:**
- Order book implementation
- Strategy backtesting framework
- Performance profiling and optimization

## Architecture
```
ITCH File → File Reader → Parser → Object Pool → Queue → Order Book → Strategy
              ↓             ↓          ↓           ↓         ↓
          [Binary]      [Bytes]   [Allocate]  [Lock-free] [Update]
```

## Components

### 1. Lock-Free SPSC Queue (`src/core/spsc_queue.hpp`)

Single-producer, single-consumer lock-free queue using atomic operations.

**Features:**
- Ring buffer design (power-of-2 capacity)
- Atomic head/tail pointers with acquire/release semantics
- Cache-line aligned to prevent false sharing
- Stack-allocated for zero heap allocations

**Performance:**
- Average latency: 14ns per operation
- Throughput: 57M messages/sec
- faster than mutex-based queue

**Key learnings:**
- Memory ordering (acquire/release barriers)
- Cache coherency (MESI protocol)
- False sharing prevention

### 2. Lock-Free Object Pool (`src/core/object_pool.hpp`)

Pre-allocated pool of Order objects with lock-free allocation/deallocation.

**stuff i learned:**
- Intrusive free list using `std::aligned_storage`
- Tagged pointers (128-bit CAS) for ABA prevention
- Placement new + explicit destructor for object lifetime management
- Zero heap allocations in hot path
-  ABA problem and solutions
- Manual memory management
- C++ object lifetime rules
- 128-bit atomic operations

**Performance:**
- ~20ns per allocation (estimated)
- Deterministic latency
- Lock-free concurrent access



### 3. ITCH File Reader (`src/data/itch_file_reader.hpp`)

Binary parser for NASDAQ ITCH 5.0 format files.

- Reads 2-byte length prefix (big-endian)
- Validates message boundaries
- Handles endianness conversion
- Binary file I/O
- Network byte order (big-endian) conversion
- Buffer management

### 4. ITCH Message Parser (`src/data/itch_parser.hpp`)

Parses raw ITCH bytes into typed Order structures.
**Features:**
- Supports Add Order ('A') and Delete Order ('D') messages
- Big-endian to little-endian conversion
- Price denormalization (ITCH stores prices in 1/10000 dollar)

**Key learnings:**
- Binary protocol parsing
- Struct layout and alignment
- Fixed-point arithmetic

  ## There is more to come 

## Building

**Requirements:**
- C++17 or later
- CMake 3.15+
- Compiler with 128-bit atomic support (GCC with `-mcx16`, Clang, or MSVC on x64)

**Build instructions:**
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run tests
./build/Release/spsc_queue_test
./build/Release/object_pool_test
./build/Release/itch_parser_test
```

**Windows-specific:**
```powershell
cmake --build build --config Release
.\build\Release\spsc_queue_test.exe
```

## Testing

**Test data:**

Sample ITCH file can be created with `create_test_itch.cpp` or downloaded from:
```
ftp://emi.nasdaq.com/ITCH/01302019.NASDAQ_ITCH50
```

**Run all tests:**
```bash
cmake --build build --config Release --target test
```

## Design Decisions

### Why lock-free?

- to learn, Deep dive into atomics, memory ordering, and concurrent data structures
- faster than mutex-based alternatives

## Learning Resources

**Lock-free programming:**
- "C++ Concurrency in Action" by Anthony Williams
- Herb Sutter's "atomic<> Weapons" talks
- Jeff Preshing's blog on memory ordering
- cpp con

**Market microstructure:**
- NASDAQ ITCH specification v5.0

**Performance optimization:**
- godbolt
- a bit of intel dev manual for stuff like 16 byte atomic compare exchange


This is a learning project. 

**Note:** This is a student project focused on learning systems programming and low-latency techniques.
