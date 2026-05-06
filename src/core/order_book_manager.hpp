#pragma once
#include "config.hpp"
#include "orderbook.hpp"
#include "data/itch_file_parser.hpp"

#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <iostream>

#include "utils/compiler.hpp"

class OrderBookManager {
private:
    std::array<OrderBook*, 65536> books_;
    std::unordered_set<std::string> whitelisted_symbols_;

    std::vector<std::unique_ptr<OrderBook>> owned_books_;

    ObjectPool<Order, config::MAX_ORDERS> order_pool_;
    ObjectPool<PriceLevel, config::MAX_PRICE_LEVELS> level_pool_;

public:
    OrderBookManager() {
        books_.fill(nullptr);
    }

    void add_to_whitelist(std::string symbol) {
        if (whitelisted_symbols_.empty()) {
            ITCHParser::clear_interest();
        }
        // ITCH symbols are 8 bytes, space-padded
        while (symbol.length() < 8) symbol += ' ';
        whitelisted_symbols_.insert(symbol);
    }
    OrderBook* get_or_create_book(uint16_t locate, char market_category) {
        OrderBook* book = books_[locate];
        if (!book) [[unlikely]] {
            auto new_book = std::make_unique<OrderBook>(order_pool_, level_pool_);
            new_book->set_market_category(market_category);
            book = new_book.get();
            books_[locate] = book;
            owned_books_.push_back(std::move(new_book));
        }
        return book;
    }

    FORCE_INLINE void process_order(const Order& order) {
        if (order.msg_type == 'R') [[unlikely]] {
            std::string sym(order.symbol, 8);
            std::cout << "[Engine] Discovered Ticker: " << sym << " (Locate: " << order.stock_locate << ")\n";
            if (whitelisted_symbols_.empty() || whitelisted_symbols_.count(sym)) {
                ITCHParser::set_interest(order.stock_locate, true);
                OrderBook* book = get_or_create_book(order.stock_locate, order.market_category);
                book->set_market_category(order.market_category);
            }
            return;
        }

        OrderBook* book = get_or_create_book(order.stock_locate, order.market_category);
        if (!book) return;

        switch (order.msg_type) {
            case 'A':
            case 'F':
                book->apply_add(order.order_ref, order.side, order.price, order.shares, order.stock_locate, order.timestamp_ns);
                break;
            case 'E':
                book->apply_execute(order.order_ref, order.shares, order.timestamp_ns);
                break;
            case 'C':
                book->apply_cancel(order.order_ref, order.shares, order.timestamp_ns);
                break;
            case 'X':
                book->apply_cancel(order.order_ref, order.shares, order.timestamp_ns);
                break;
            case 'D':
                book->apply_delete(order.order_ref, order.timestamp_ns);
                break;
            case 'U':
                book->apply_replace(order.order_ref, order.new_order_ref, order.shares, order.price, order.timestamp_ns);
                break;
            case 'R':
                book->set_market_category(order.market_category);
                break;
        }
    }

    Order* get_order(uint16_t locate, uint64_t ref) {
        OrderBook* book = books_[locate];
        return book ? book->get_order(ref) : nullptr;
    }

    const OrderBook* get_book(uint16_t locate) const {
        return books_[locate];
    }
};
