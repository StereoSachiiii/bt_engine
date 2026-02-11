#include<cstddef>
#include<array>
#include<memory>
#include<atomic>


template<typename T,size_t capacity>
class SPSCQueue {
	//(n & (n - 1)) == 0 this is like modulo .. basically base 10 for us humans -> base 2 for computers 

	static_assert((capacity & (capacity - 1)) == 0, "capacity of the queue must be a power of 2");

	private:
		alignas(64) std::atomic<size_t> _tail{ 0 };
		alignas(64) std::atomic<size_t> _head{ 0 };

		std::unique_ptr<std::array<T, capacity>> _buffer;

		static constexpr size_t MASK = capacity - 1 ;

	public:
		SPSCQueue() : _buffer(std::make_unique<std::array<T, capacity>>()){}

		bool try_push(T&& item){
			size_t current_tail = _tail.load(std::memory_order_relaxed);
			size_t next_tail = (current_tail + 1) & MASK;

			size_t current_head = _head.load(std::memory_order_acquire);

			if (current_head == next_tail) {
				return false;
			}

			(*_buffer)[current_tail] = std::move(item); 
			_tail.store(next_tail, std::memory_order_release);
			return true;
		}

		bool try_pop(T& item){
			size_t current_head = _head.load(std::memory_order_relaxed);
			size_t current_tail = _tail.load(std::memory_order_acquire);

			if (current_head == current_tail) {
				return false;
			}

			item = std::move((*_buffer)[current_head]);
			size_t next_head = current_head + 1 & MASK;
			_head.store(next_head, std::memory_order_release);
			return true;
		}

		//copy
		SPSCQueue(const SPSCQueue&) = delete;
		//copy assignment blocked
		SPSCQueue& operator=(const SPSCQueue&) = delete;
		//move
		SPSCQueue(SPSCQueue&&) = delete;
		//move assignment blocked
		SPSCQueue& operator=(SPSCQueue&&) = delete;

		


};
