#pragma once
#include "../order.hpp"
#include "../orderbook.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

struct VirtualOrder {
    uint64_t price;
    uint32_t qty;
    uint8_t side;
    uint32_t volume_ahead; 
    bool active = false;
};

class VirtualMatcher {
private:
    std::vector<VirtualOrder> active_orders_;

public:
    void place_order(uint16_t locate, uint8_t side, uint64_t price, uint32_t qty, uint32_t total_ahead) {
        active_orders_.erase(
            std::remove_if(active_orders_.begin(), active_orders_.end(),
                [side](const VirtualOrder& vo) { return vo.active && vo.side == side; }),
            active_orders_.end()
        );

        active_orders_.push_back({price, qty, side, total_ahead, true});
    }



    void on_message(const Order& event, PositionManager& pos_mgr, const OrderBookManager& manager) {
        if (event.msg_type != 'E' && event.msg_type != 'C' && event.msg_type != 'X' && event.msg_type != 'D') return;

        const Order* original = const_cast<OrderBookManager&>(manager).get_order(event.stock_locate, event.order_ref);
        if (!original) return;


        uint64_t price = original->price;
        uint8_t side = original->side;

        for (auto& vo : active_orders_) {
            if (!vo.active || vo.price != price || vo.side != side) continue;

            if (event.msg_type == 'E' || event.msg_type == 'C') {

                // Execution: Reduces volume ahead
                if (vo.volume_ahead >= event.shares) {
                    vo.volume_ahead -= event.shares;
                } else {
                    // Volume ahead is exhausted! We get filled.
                    uint32_t fill_qty = std::min(vo.qty, event.shares - vo.volume_ahead);
                    vo.volume_ahead = 0;
                    
                    if (fill_qty > 0) {
                        bool is_buy = (vo.side == 'B');
                        pos_mgr.on_fill(event.stock_locate, (int64_t)fill_qty, (double)vo.price, is_buy);
                        std::cout << "[Matcher] FILL " << (is_buy ? "BUY" : "SELL") << ": " << fill_qty << " @ " << (vo.price / 10000.0) << "\n";
                        vo.qty -= fill_qty;
                        if (vo.qty == 0) vo.active = false;
                    }
                }
            } else {
                
            }
        }

        active_orders_.erase(std::remove_if(active_orders_.begin(), active_orders_.end(), 
            [](const VirtualOrder& o) { return !o.active; }), active_orders_.end());
    }
};
