#pragma once
#include "utils/taggedPtr.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>



template<typename T, size_t MaxObjects>
class ObjectPool {
    static_assert(sizeof(T) >= sizeof(void*),
        "T must be at least pointer-sized: the free-list stores a next-pointer "
        "directly inside each slot. Use a wrapper struct if T is smaller than void*.");

    static_assert(alignof(T) >= alignof(void*),
        "T alignment must be at least pointer-aligned for safe next-pointer storage.");

private:

    // Raw memory — 
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    alignas(64) std::array<Storage, MaxObjects> storage_;

    // Lock-free stack head (128-bit atomic)
    alignas(16) std::atomic<TaggedPtr> free_list_head_;

    // track allocations
    std::atomic<size_t> allocated_count_{ 0 };

public:
    ObjectPool() {
        // Link 
        for (size_t i = 0; i < MaxObjects - 1; i++) {
            set_next(&storage_[i], &storage_[i + 1]);
        }

        set_next(&storage_[MaxObjects - 1], nullptr);

        free_list_head_.store(TaggedPtr(&storage_[0], 0));
    };

    ~ObjectPool() = default;

    //  construct T in-place
    T* allocate() {
        TaggedPtr old_head = free_list_head_.load(std::memory_order_acquire);

        while (old_head.ptr != nullptr) {

            void* next = get_next(old_head.ptr);
           
            TaggedPtr new_head(next, old_head.tag + 1);

            if (free_list_head_.compare_exchange_weak(
                old_head,
                new_head,
                std::memory_order_release,
                std::memory_order_acquire
            )) {

                allocated_count_.fetch_add(1, std::memory_order_relaxed);

                // Construct T in raw memory via placement new
                return new (old_head.ptr) T();
            }
        }
        return nullptr;
    }

    // Destroy T, push raw memory
    void deallocate(T* obj) {
        if (!obj) return;

        // Destroy 
        obj->~T();

        TaggedPtr old_head = free_list_head_.load(std::memory_order_acquire);
        TaggedPtr new_head;

        do {
            set_next(obj, old_head.ptr);
            new_head = TaggedPtr(obj, old_head.tag + 1);
        } while (!free_list_head_.compare_exchange_weak(
            old_head,
            new_head,
            std::memory_order_release,
            std::memory_order_acquire
        ));

        allocated_count_.fetch_sub(1, std::memory_order_relaxed);
    }

    size_t available() const {
        return MaxObjects - allocated_count_.load(std::memory_order_relaxed);
    }

    size_t capacity() const { 
        return MaxObjects; 
    }

private:
    static void* get_next(void* node) {
        return *reinterpret_cast<void**>(node);
    }

    static void set_next(void* node, void* next) {
        *reinterpret_cast<void**>(node) = next;
    }
};
