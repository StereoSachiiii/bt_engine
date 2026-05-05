#include "core/engine.hpp"
#include "core/strategy/market_maker.hpp"
#include "core/strategy/pairs_strategy.hpp"
#include <iostream>

#include <iomanip>
#include <chrono>

int main(int argc, char* argv[]) {
    bool raw = false;
    std::string path;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--raw") raw = true;
        else path = argv[i];
    }

    if (path.empty()) {
        std::cout << "Usage: hft_engine [--raw] <itch_file_path>\n";
        return 0;
    }

 
    PairsStrategy strategy(1, 2); 
    HFTEngine<PairsStrategy> engine(strategy);

    engine.set_raw_mode(raw);


    std::cout << "Starting HFT Engine...\n";
    std::cout << "Processing file: " << path << " (Raw Mode: " << (raw ? "ON" : "OFF") << ")\n";
    std::cout << "Scanning for instruments...\n";


    auto start = std::chrono::steady_clock::now();
    engine.run_file(path);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "\n--- Engine Statistics ---\n";
    std::cout << "Processing time: " << duration << " ms\n";
    
    auto pnl = strategy.get_pnl(engine.get_manager());
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Realized PnL:   $" << pnl.realized << "\n";
    std::cout << "Unrealized PnL: $" << pnl.unrealized << "\n";
    std::cout << "Total PnL:      $" << pnl.total << "\n";
    std::cout << "----------------------------------------\n";
    
    return 0;
}
