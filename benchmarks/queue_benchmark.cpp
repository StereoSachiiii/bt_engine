#include "../src/core/queue.hpp"
#include "../src/core/timer.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstddef>

using TestQueue = SPSCQueue<int, 1024>;

struct STATS {
	uint64_t min, max, p50, p99, p999;
	double avg;
};

STATS calculate_stats(std::vector<uint64_t>& data) {
	STATS s;

	std::sort(data.begin(),data.end());

	s.min = data[0];
	s.p50 = data[data.size() / 2];
	s.p99 = data[data.size() * 99 / 100];
	s.p999 = data[data.size() * 999 / 1000];
	s.max = data[data.size() - 1];

	uint64_t sum = 0;
	for (auto i : data) sum += i;
	s.avg = static_cast<double>(sum) / data.size();
	return s;

}

///take the queue push something pop something then just store the latancies in the vector

void benchmark_latency() {
	std::cout << "=== Latency Benchmark ===\n";


	TestQueue queue;
	uint64_t ITERATIONS = 10000000;

	std::vector<uint64_t> latencies;
	Timer timer;
	
	latencies.reserve(ITERATIONS);


	for (int i = 0; i < ITERATIONS; ++i) {
		uint64_t pushed = i;
		timer.start();

		
		queue.try_push(pushed);

		int popped;
		queue.try_pop(popped);

		uint64_t latency = timer.elapsed_ns();
		latencies.push_back(latency);
	}

	STATS s = calculate_stats(latencies);

	std::cout << "Messages: " << ITERATIONS << "\n";
	std::cout << "Min:      " << s.min << "ns\n";
	std::cout << "p50:      " << s.p50 << "ns\n";
	std::cout << "p99:      " << s.p99 << "ns\n";
	std::cout << "p999:     " << s.p999 << "ns\n";
	std::cout << "Max:      " << s.max << "ns\n";
	std::cout << "Average:  " << s.avg << "ns\n";

}


void benchmark_throughput() {
	std::cout << "\n=== Throughput Benchmark ===\n";

	TestQueue queue;
	uint64_t MESSAGES = 100000000;
	Timer timer;

	std::atomic<bool> producer_done{ false };
	std::atomic<size_t> messages_consumed{ 0 };

	std::thread producer([&]() {

		for (size_t i = 0; i < MESSAGES; ++i) {
			while (!queue.try_push(static_cast<int>(i))) {
				std::this_thread::yield();
			}
		}
		producer_done.store(false);
		});

	timer.start();

	std::thread consumer([&]() {
		int popped;
		//while messages are not still over
		while (messages_consumed.load() < MESSAGES) {
			//if the queue is not empty
			if (!queue.try_pop(popped)) {
				messages_consumed.fetch_add(1);
			}
			else if (producer_done.load()) {
				while (queue.try_pop(popped)) {
					messages_consumed.fetch_add(1);
				}
				break;
			}


		}
	});
	producer.join();
	consumer.join();

	uint64_t elapsed_ns = timer.elapsed_ns();
	double elapsed_s = elapsed_ns / 1e9;
	double throughput = MESSAGES / elapsed_s;

	std::cout << "Messages:   " << MESSAGES << "\n";
	std::cout << "Duration:   " << elapsed_s << "s\n";
	std::cout << "Throughput: " << (throughput / 1e6) << "M msgs/sec\n";
	std::cout << "Avg latency:" << (elapsed_ns / MESSAGES) << "ns/msg\n";
		
}

int main() {
	benchmark_latency();
	benchmark_throughput();

}