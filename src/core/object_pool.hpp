#pragma once
#include "utils/taggedPtr.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>



/**
 * @brief Thread-safe, lock-free object pool for high-concurrency environments.
 * Uses a TaggedPtr to prevent the ABA problem in the lock-free stack.
 */
template<typename T, size_t MaxObjects>
class ObjectPool {
    // We store the 'next' pointer directly in the raw memory of the object slots.
    // Therefore, T must be large enough to hold at least one pointer.
    static_assert(sizeof(T) >= sizeof(void*),
        "T must be at least pointer-sized: the free-list stores a next-pointer "
        "directly inside each slot.");

    static_assert(alignof(T) >= alignof(void*),
        "T alignment must be at least pointer-aligned for safe next-pointer storage.");

private:
    //rawdog bytes
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Storage* storage_;

    alignas(16) std::atomic<TaggedPtr> free_list_head_;

    std::atomic<size_t> allocated_count_{ 0 };

public:
    ObjectPool() {
        storage_ = static_cast<Storage*>(_aligned_malloc(sizeof(Storage) * MaxObjects, 64));
        if (!storage_) throw std::bad_alloc();

        for (size_t i = 0; i < MaxObjects - 1; i++) {
            set_next(&storage_[i], &storage_[i + 1]);
        }

        set_next(&storage_[MaxObjects - 1], nullptr);

        free_list_head_.store(TaggedPtr(&storage_[0], 0));
    };

    ~ObjectPool() {
        _aligned_free(storage_);
    }

    /**
     * @brief Allocates an object from the pool in a lock-free manner.
     * @return Pointer to the constructed object, or nullptr if pool is empty.
     */
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
                //construct at the addr start lifetime
                return new (old_head.ptr) T();
            }
        }
        return nullptr;
    }

    /**
     * @brief Returns an object to the pool.
     * @param obj Pointer to the object to deallocate.
     */
    void deallocate(T* obj) {
        if (!obj) return;

        // Manually call the destructor since we used placement new.
        obj->~T();

        TaggedPtr old_head = free_list_head_.load(std::memory_order_acquire);
        TaggedPtr new_head;

        // Push the raw memory back onto the lock-free stack.
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
    // Helper to read the hidden 'next' pointer from a memory slot.
    static void* get_next(void* node) {
        return *reinterpret_cast<void**>(node);
    }

    // Helper to write the hidden 'next' pointer into a memory slot.
    static void set_next(void* node, void* next) {
        *reinterpret_cast<void**>(node) = next;
    }
};


/**
 * @brief High-performance object pool for single-threaded use.
 * Removes all atomic overhead for use in partitioned (SPSC-n) architectures.
 */
template<typename T, size_t MaxObjects>
class SingleThreadedObjectPool {
    static_assert(sizeof(T) >= sizeof(void*), "T must be at least pointer-sized");

private:
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Storage* storage_;
    void* free_list_head_;
    size_t allocated_count_{ 0 };

public:
    SingleThreadedObjectPool() {
        storage_ = static_cast<Storage*>(_aligned_malloc(sizeof(Storage) * MaxObjects, 64));
        if (!storage_) throw std::bad_alloc();

        for (size_t i = 0; i < MaxObjects - 1; i++) {
            set_next(&storage_[i], &storage_[i + 1]);
        }
        set_next(&storage_[MaxObjects - 1], nullptr);
        free_list_head_ = &storage_[0];
    }

    ~SingleThreadedObjectPool() {
        _aligned_free(storage_);
    }

    T* allocate() {
        if (!free_list_head_) return nullptr;

        void* ptr = free_list_head_;
        free_list_head_ = get_next(ptr);
        allocated_count_++;

        return new (ptr) T();
    }

    void deallocate(T* obj) {
        if (!obj) return;
        obj->~T();
        set_next(obj, free_list_head_);
        free_list_head_ = obj;
        allocated_count_--;
    }

    size_t available() const { return MaxObjects - allocated_count_; }
    size_t capacity()  const { return MaxObjects; }

private:
    static void* get_next(void* node) {
        return *reinterpret_cast<void**>(node);
    }
    static void set_next(void* node, void* next) {
        *reinterpret_cast<void**>(node) = next;
    }

    SingleThreadedObjectPool(const SingleThreadedObjectPool&) = delete;
    SingleThreadedObjectPool& operator=(const SingleThreadedObjectPool&) = delete;
};

