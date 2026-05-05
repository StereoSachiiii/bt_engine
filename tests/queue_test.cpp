#include "core/queue.hpp"
#include <iostream>
#include <cassert>

void test_push_pop() {
    SPSCQueue<int, 8> q;
    assert(q.try_push(std::move(42)) && "push to empty queue should succeed");

    [[maybe_unused]] int out = 0;
    assert(q.try_pop(out) && "pop from non-empty queue should succeed");
    assert(out == 42 && "popped value should match pushed value");

    std::cout << "test_push_pop OK\n";
}

void test_empty_pop() {
    SPSCQueue<int, 8> q;
    int dummy = 0;
    assert(!q.try_pop(dummy) && "pop from empty queue should fail");

    std::cout << "test_empty_pop OK\n";
}

void test_full_queue() {
    // capacity=4, but one slot is always reserved (ring buffer), so max items = 3
    SPSCQueue<int, 4> q;
    assert(q.try_push(std::move(1)));
    assert(q.try_push(std::move(2)));
    assert(q.try_push(std::move(3)));
    assert(!q.try_push(std::move(4)) && "push to full queue should fail");

    std::cout << "test_full_queue OK\n";
}

void test_wrap_around() {
    SPSCQueue<int, 4> q;

    // fill and drain multiple times to exercise wrap-around
    for (int round = 0; round < 10; ++round) {
        for (int i = 0; i < 3; ++i) {
            [[maybe_unused]] int v = round * 10 + i;
            assert(q.try_push(std::move(v)));
        }
        for (int i = 0; i < 3; ++i) {
            [[maybe_unused]] int out = 0;
            assert(q.try_pop(out));
            assert(out == round * 10 + i);
        }
    }

    std::cout << "test_wrap_around OK\n";
}

int main() {
    test_push_pop();
    test_empty_pop();
    test_full_queue();
    test_wrap_around();
    std::cout << "All queue tests passed!\n";
    return 0;
}
