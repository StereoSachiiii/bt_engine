#pragma once
#include "utils/taggedPtr.hpp"
#include "order.hpp"
#include<atomic>



template<typename T, size_t MaxObjects>
class ObjectPool {
    static_assert(sizeof(T) >= sizeof(void*),
        "T must be large enough to store a pointer");

private:

    alignas(64) std::array<T, MaxObjects> storage_;

    // Lock-free stack head (128-bit atomic)
    alignas(16) std::atomic<TaggedPtr> free_list_head_;

    // Debug: track allocations
    std::atomic<size_t> allocated_count_{ 0 };

public:
    //pimpl later idiot
    ObjectPool() {
        // 1. Link all objects except the last one
        for (size_t i = 0; i < MaxObjects - 1; i++) {
            set_next(&storage_[i], &storage_[i + 1]);
        }

        // 2. The last object marks the end of the list
        set_next(&storage_[MaxObjects - 1], nullptr);

        // 3. Point the head to the first object in the array

        free_list_head_.store(TaggedPtr(&storage_[0], 0));
    };

    
    ~ObjectPool() = default;

    T* allocate() {
        TaggedPtr old_head = free_list_head_.load(_Atomic_memory_order_acquire);


        while (old_head.ptr != nullptr) {
           
            TaggedPtr new_head(old_head.ptr, old_head.tag);

            if (free_list_head_.compare_exchange_weak(
                old_head,
                new_head,
                std::memory_order_release,
                std::memory_order_acquire
            )) {

                allocated_count_.fetch_add(1, std::memory_order_relaxed);
                return reinterpret_cast<T*>(old_head.ptr);
            }
        }
        return nullptr;
  
    };
//  void deallocate(T* obj);
//
//    size_t available() const {
//        return MaxObjects - allocated_count_.load(std::memory_order_relaxed);
//    }
//
//    size_t capacity() const { return MaxObjects; }
//
//private:
//    // Helper: get next pointer from free object
//    static void* get_next(void* node) {
//        return *reinterpret_cast<void**>(node);
//    }
//
//    // Helper: set next pointer in free object
//    static void set_next(void* node, void* next) {
//        *reinterpret_cast<void**>(node) = next;
//    }
};





















}