# Week 1: Lock-Free Queue - Performance Results

## System Specs
- CPU: i5 12th gen
- RAM: 16gb
- OS: Windows 11
- Compiler: MSVC 
- Build: Release (x64)

## Implementation
Single Producer Single Consumer (SPSC)
1024 items ib the qyeye
Ring buffer with atomic head/tail
Heap-allocated via unique_ptr at start
Lock-free atomics with acquire/release

## Performance Results

### Throughput Benchmark
- **Messages processed:** 100,000,000
- **Duration:** 1.73 seconds
- **Throughput:** 57.9M msgs/sec
- **Average latency:** 17ns/msg

###  Optimizations
1. Cache-line alignment (alignas(64)) to get both into different cache lines so one cpu doesnt wait for a release on the line
2. Power-of-2 capacity enables bitmask (x & mask vs x % size)
3. Memory ordering: relaxed for own variables, acquire/release for sync
4. Single allocation at construction (no runtime malloc)

- Timer resolution (100ns) limits latency measurement  correctly because it looks like the counter cant measure anything below 100ns
- unique_ptr adds one pointer indirection (~1-2ns overhead) each iter 


will do nextr Remove unique_ptr indirection idk how