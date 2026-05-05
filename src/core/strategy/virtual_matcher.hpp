#pragma once
#include "../types/order.hpp"
#include "../orderbook.hpp"
#include "../types/strategy_types.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <array>

class VirtualMatcher {
private:
    //  unique_ptr to keep it off the stack but cache problem
    std::unique_ptr<std::array<StockOrders, 65536>> orders_;

public:
    VirtualMatcher() : orders_(std::make_unique<std::array<StockOrders, 65536>>()) {
        //  all orders are initialized to inactive
        for (auto& stock : *orders_) {
            stock.bid.active = false;
            stock.ask.active = false;
        }
    }

    void place_order(uint16_t locate, uint8_t side, uint64_t price, uint32_t qty, uint32_t total_ahead) {
        auto& stock = (*orders_)[locate];
        if (side == 'B') {
            stock.bid = {price, qty, total_ahead, true};
        } else {
            stock.ask = {price, qty, total_ahead, true};
        }
    }

    void on_message(const Order& event, PositionManager& pos_mgr, const OrderBookManager& manager) {
        //dont care
        if (event.msg_type != 'E' && event.msg_type != 'C' && event.msg_type != 'X' && event.msg_type != 'D') [[likely]] return;

        auto& stock = (*orders_)[event.stock_locate];
        if (!stock.bid.active && !stock.ask.active) [[likely]] return;

        const Order* original = const_cast<OrderBookManager&>(manager).get_order(event.stock_locate, event.order_ref);
        if (!original) [[unlikely]] return;

        uint64_t price = original->price;
        uint8_t side = original->side;

        VirtualOrder& vo = (side == 'B') ? stock.bid : stock.ask;
        
        if (!vo.active || vo.price != price) [[likely]] return;

        if (event.msg_type == 'E' || event.msg_type == 'C') {
            if (vo.volume_ahead >= event.shares) [[likely]] {
                
                vo.volume_ahead -= event.shares;
            } else {
                uint32_t fill_qty = std::min(vo.qty, event.shares - vo.volume_ahead);
                vo.volume_ahead = 0;
                
                if (fill_qty > 0) {
                    bool is_buy = (side == 'B');
                    pos_mgr.on_fill(event.stock_locate, (int64_t)fill_qty, (double)vo.price, is_buy);
                    std::cout << "[Matcher] FILL " << (is_buy ? "BUY" : "SELL") << ": " << fill_qty << " @ " << (vo.price / 10000.0) << "\n";
                    vo.qty -= fill_qty;
                    if (vo.qty == 0) vo.active = false;
                }
            }
        } else if (event.msg_type == 'X' || event.msg_type == 'D') {
            if (vo.volume_ahead > 0) {
                vo.volume_ahead = (event.shares >= vo.volume_ahead) ? 0 : vo.volume_ahead - event.shares;
            }
        }
    }
};
