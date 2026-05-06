#pragma once
#include "order_book_manager.hpp"
#include "types/order.hpp"
#include "data/itch_file_parser.hpp"
#include "data/itch_file_reader.hpp"
#include "strategy/strategy.hpp"
#include <iostream>
#include <vector>
#include "utils/compiler.hpp"

template<typename StrategyType>
class HFTEngine {
private:
    OrderBookManager manager_;
    Order current_order_;
    StrategyType& strategy_;

public:
    HFTEngine(StrategyType& strategy) : strategy_(strategy) {
        ITCHParser::init();
    }

    void set_raw_mode(bool raw) { raw_mode_ = raw; }

   
    FORCE_INLINE void process_message(const uint8_t* data) {
        if (!ITCHParser::parse(data, current_order_)) return;

        strategy_.on_event(current_order_, manager_);

        manager_.process_order(current_order_);

        OrderBook* book = manager_.get_or_create_book(current_order_.stock_locate, ' ');
        if (book) {
            strategy_.on_order_book_update(current_order_.stock_locate, *book, manager_);
            
            if (current_order_.msg_type == 'P' || current_order_.msg_type == 'E') {
                strategy_.on_trade(current_order_.stock_locate, static_cast<double>(current_order_.price), current_order_.shares);
            }
        }
    }



    void run_file(const std::string& path) {
        ITCHReader reader;
        reader.set_raw_mode(raw_mode_);
        if (!reader.open(path.c_str())) {
            std::cerr << "Failed to open ITCH file: " << path << "\n";
            return;
        }

        std::vector<uint8_t> buffer(8192); 
        size_t msg_len = 0;
        
        while (reader.read_next(buffer.data(), msg_len)) {
            process_message(buffer.data());
        }
    }

    OrderBookManager& get_manager() { return manager_; }

    bool raw_mode_ = false;
};
