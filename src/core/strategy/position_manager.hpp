#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../types/strategy_types.hpp"

class PositionManager {
private:
   
    std::unordered_map<uint16_t, Position> positions_;

public:
    void on_fill(uint16_t locate, int64_t qty, double price, bool is_buy) {
        positions_[locate].update_on_fill(qty, price, is_buy);
    }

    const Position& get_position(uint16_t locate) const {
        static const Position empty{};
        auto it = positions_.find(locate);
        return (it != positions_.end()) ? it->second : empty;
    }

    const std::unordered_map<uint16_t, Position>& all_positions() const {
        return positions_;
    }
};
