hft-engine
-> high-performance c++23 nasdaq itch 5.0 processor

features
-> zero-allocation order book
-> constant-time price discovery
-> cache-optimized (64b alignment)
-> fast itch parsing

benchmarks
-> parsing: 2-4ns
-> full system: 30-50ns
-> throughput: 20m+ msg/s

build
-> cmake -B build
-> cmake --build build --config Release

test
-> ./build/Release/orderbook_test
-> ./build/Release/itch_parser_test
-> ./build/Release/full_system_benchmark

run
-> ./build/Release/hft_engine <file>
