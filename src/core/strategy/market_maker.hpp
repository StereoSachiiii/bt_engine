#include "strategy.hpp"
#include "virtual_matcher.hpp"
#include <iostream>
#include <iomanip>
#include <memory>


class SimpleMarketMaker : public Strategy {
private:
    uint16_t target_locate_ = 0;
    VirtualMatcher matcher_;
    
    // Aggressive Strategy for Verification
    const double base_offset = 0.0;            
    const double skew_factor = 0.01 * 10000;    
    const uint32_t max_pos = 1000;              
    const double min_spread = 0.0;             

    struct Quote {  
        double bid = 0;
        double ask = 1e9;
    };
    std::unique_ptr<std::array<Quote, 65536>> quotes_;

public:
    SimpleMarketMaker(uint16_t locate) : quotes_(std::make_unique<std::array<Quote, 65536>>()) {
        for (auto& q : *quotes_) {
            q.bid = 0;
            q.ask = 1e9;
        }
    }



    void on_event(const Order& event, const OrderBookManager& manager) {
        matcher_.on_message(event, pos_mgr_, manager);
    }

    void on_order_book_update(uint16_t locate, const OrderBook& book, const OrderBookManager& manager) {
        // Trade 
        double bid = static_cast<double>(book.best_bid());
        double ask = static_cast<double>(book.best_ask());
        if (bid == 0 || ask == 0) return;

        // Quote at the Market Touch to ensure we are at a price level with real executions
        double target_bid = bid;
        double target_ask = ask;
        
        int32_t position = (int32_t)pos_mgr_.get_position(locate).net_qty;
        double skew = (position / 100.0) * skew_factor;

        double old_bid = (*quotes_)[locate].bid;
        double old_ask = (*quotes_)[locate].ask;

        (*quotes_)[locate].bid = target_bid - skew;
        (*quotes_)[locate].ask = target_ask - skew;


        if (position >= (int32_t)max_pos) (*quotes_)[locate].bid = 0;
        if (position <= -(int32_t)max_pos) (*quotes_)[locate].ask = 1e9;

        if ((uint64_t)(*quotes_)[locate].bid != (uint64_t)old_bid) {
            uint64_t quote_price = (uint64_t)(*quotes_)[locate].bid;
            std::cout << "[Strategy] New BID for Locate " << locate << ": " << (quote_price / 10000.0) << "\n";
            uint32_t ahead = book.qty_at_price('B', quote_price);
            matcher_.place_order(locate, 'B', quote_price, 100, ahead);
        }
        if ((uint64_t)(*quotes_)[locate].ask != (uint64_t)old_ask) {
            uint64_t quote_price = (uint64_t)(*quotes_)[locate].ask;
            std::cout << "[Strategy] New ASK for Locate " << locate << ": " << (quote_price / 10000.0) << "\n";
            uint32_t ahead = book.qty_at_price('S', quote_price);
            matcher_.place_order(locate, 'S', quote_price, 100, ahead);
        }



    }
    


    void on_trade(uint16_t locate, double price, uint32_t qty) {
    }
};

