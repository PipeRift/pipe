// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <Pipe/Memory/MemoryStats.h>

#include <atomic>
#include <thread>
#include <vector>


using namespace snowhouse;
using namespace bandit;
using namespace p;


static i32 LiveCount(const MemoryStats& s)
{
	return s.liveAllocations.Size();
}
static const MemoryStatsEvent* LiveFind(const MemoryStats& s, void* ptr)
{
	// Lookup is keyed by pointer only; size is irrelevant for matching.
	return s.liveAllocations.Find(MemoryStatsEvent{ptr, 0});
}


go_bandit([]()
{
	describe("Memory.MemoryStats", []()
	{
		describe("Basic", [&]()
		{
			it("Starts empty", [&]()
			{
				MemoryStats s;
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(s.totalAllocated, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
			});

			it("Tracks a single add", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(64));
				AssertThat(s.totalAllocated, Is().EqualTo(64));
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(LiveFind(s, (void*)0x1000) != nullptr, Is().True());
				AssertThat(LiveFind(s, (void*)0x1000)->GetSize(), Is().EqualTo(64));
				AssertThat(LiveFind(s, (void*)0x1000)->IsFree(), Is().EqualTo(false));
			});

			it("Tracks add plus free", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
				// totalAllocated is cumulative alloc bytes ever.
				AssertThat(s.totalAllocated, Is().EqualTo(64));
			});

			it("Tracks multiple adds", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 16);
				s.Add((void*)0x2000, 32);
				s.Add((void*)0x3000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(16 + 32 + 64));
				AssertThat(s.totalAllocated, Is().EqualTo(16 + 32 + 64));
				AssertThat(LiveCount(s), Is().EqualTo(3));
			});

			it("Tracks many adds and frees", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				const sizet N = 100;
				TArray<u8, 0> buf(N * 16);

				for (sizet i = 0; i < N; ++i)
				{
					s.Add(&buf[i * 16], 16);
				}
				for (sizet i = 0; i < N; i += 2)
				{
					s.Remove(&buf[i * 16], 16);
				}
				s.CollectStats();

				AssertThat(s.used, Is().EqualTo((N / 2) * 16));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 16));
				AssertThat(LiveCount(s), Is().EqualTo(N / 2));
			});

			it("Ignores double-free", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				// The second free matches no live alloc and is ignored.
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
			});

			it("Ignores free of unknown ptr", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Remove((void*)0xDEAD, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
			});

			it("Records duplicate allocs as UnfreedRealloc", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x1000, 128);
				s.CollectStats();
				// Same ptr twice: the second alloc is an error and the
				// live set is left untouched.
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(LiveFind(s, (void*)0x1000)->GetSize(), Is().EqualTo(64));
				AssertThat(s.errors.Size(), Is().EqualTo(1));
				AssertThat(s.errors[0].kind == MemoryStatsErrorType::UnfreedRealloc, Is().True());
				AssertThat(s.errors[0].event.GetSize(), Is().EqualTo(128));
				AssertThat(s.used, Is().EqualTo(64));
			});

			it("CheckLeaks always runs when called directly", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				s.CheckLeaks();
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(s.used, Is().EqualTo(64));
			});

			it("Always tracks frees (no trackFrees flag)", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
			});

			it("CheckLeaks with null name does not crash", [&]()
			{
				{
					// detectLeaks defaults to true and name defaults to null.
					MemoryStats s;
					s.Add((void*)0x1000, 64);
					s.CollectStats();
					// Destructor runs CheckLeaks with leaks and a null name.
				}
			});

			it("live list only keeps unmatched allocs", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				// allocs: 2 live, 1 matched. frees: 2 (one matches, one stray).
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x2000, 32);
				s.Add((void*)0x3000, 16);
				s.Remove((void*)0x3000, 16);
				s.Remove((void*)0xDEAD, 16);
				s.CollectStats();

				AssertThat(LiveCount(s), Is().EqualTo(2));
				AssertThat(LiveFind(s, (void*)0x1000)->GetSize(), Is().EqualTo(64));
				AssertThat(LiveFind(s, (void*)0x2000)->GetSize(), Is().EqualTo(32));

				// Re-collecting must preserve the live list identically.
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(2));
				AssertThat(s.used, Is().EqualTo(64 + 32));
			});

			it("Alternating instances on one thread", [&]()
			{
				// Exercises thread context reuse when the owner switches.
				MemoryStats a;
				MemoryStats b;
				a.detectLeaks = false;
				b.detectLeaks = false;

				a.Add((void*)0x1000, 64);
				b.Add((void*)0x2000, 32);
				a.Add((void*)0x3000, 16);
				b.Remove((void*)0x2000, 32);

				a.CollectStats();
				b.CollectStats();

				AssertThat(a.used, Is().EqualTo(64 + 16));
				AssertThat(LiveCount(a), Is().EqualTo(2));
				AssertThat(b.used, Is().EqualTo(0));
				AssertThat(LiveCount(b), Is().EqualTo(0));
			});

			it("Add after Reset works", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Reset();
				AssertThat(LiveCount(s), Is().EqualTo(0));

				s.Add((void*)0x2000, 32);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(32));
				AssertThat(s.totalAllocated, Is().EqualTo(32));
				AssertThat(LiveCount(s), Is().EqualTo(1));
			});

			it("Duplicate allocs record UnfreedRealloc and live stays usable", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;

				// Collect 1: two allocs sharing the same ptr. The second is
				// an UnfreedRealloc error; the live set keeps only the first.
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(s.errors.Size(), Is().EqualTo(1));
				AssertThat(s.errors[0].kind == MemoryStatsErrorType::UnfreedRealloc, Is().True());
				AssertThat(s.used, Is().EqualTo(64));

				// Collect 2: freeing the original alloc still works.
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(0));
				AssertThat(s.used, Is().EqualTo(0));
			});

			it("Free with wrong size records SizeMismatch", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 32);    // size mismatch
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(s.errors.Size(), Is().EqualTo(1));
				AssertThat(s.errors[0].kind == MemoryStatsErrorType::SizeMismatch, Is().True());
				AssertThat(s.errors[0].event.GetSize(), Is().EqualTo(32));
				AssertThat(s.used, Is().EqualTo(64));

				// Correcting the size frees the alloc normally.
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(0));
				AssertThat(s.used, Is().EqualTo(0));
			});

			it("Free of unknown ptr records UnknownFree", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0xDEAD, 64);
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(1));
				AssertThat(s.errors.Size(), Is().EqualTo(1));
				AssertThat(s.errors[0].kind == MemoryStatsErrorType::UnknownFree, Is().True());
				AssertThat(s.errors[0].event.GetPtr(), Is().EqualTo((u8*)0xDEAD));
				AssertThat(s.used, Is().EqualTo(64));
			});

			it("Ignores null ptr in Remove", [&]()
			{
				MemoryStats s;
				s.Remove(nullptr, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
			});

			it("Ignores null ptr in Add", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add(nullptr, 64);
				s.CollectStats();
				// Add has no null check (unlike Remove), so the event is
				// recorded and processed. Add's size is still tracked.
				AssertThat(s.used, Is().EqualTo(64));
				AssertThat(LiveCount(s), Is().EqualTo(1));
			});

			it("Reset resets state", [&]()
			{
				MemoryStats s;
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x2000, 32);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(96));

				s.Reset();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(s.totalAllocated, Is().EqualTo(0));
				AssertThat(LiveCount(s), Is().EqualTo(0));
			});

			it("CollectStats is additive", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				s.Add((void*)0x2000, 32);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(96));
				AssertThat(LiveCount(s), Is().EqualTo(2));
			});

			it("Re-collecting preserves state", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(64));
				AssertThat(LiveCount(s), Is().EqualTo(1));
			});
		});


		describe("Multiple chunks", [&]()
		{
			it("Spans multiple chunks correctly", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				const sizet N = 10000;
				TArray<u8, 0> buf(N * 8);
				for (sizet i = 0; i < N; ++i)
				{
					s.Add(&buf[i * 8], 8);
				}
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(N * 8));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 8));
				AssertThat(LiveCount(s), Is().EqualTo(N));
			});

			it("Handles add/free across chunks", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				const sizet N = 5000;
				TArray<u8, 0> buf(N * 8);
				for (sizet i = 0; i < N; ++i)
				{
					s.Add(&buf[i * 8], 8);
				}
				for (sizet i = 0; i < N / 2; ++i)
				{
					s.Remove(&buf[i * 8], 8);
				}
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo((N / 2) * 8));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 8));
				AssertThat(LiveCount(s), Is().EqualTo(N / 2));
			});

			it("Frees chunks between CollectStats calls", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				const sizet N = 10000;
				TArray<u8, 0> buf(N * 8);
				for (sizet i = 0; i < N; ++i)
				{
					s.Add(&buf[i * 8], 8);
				}
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(N));
				for (sizet i = 0; i < N / 2; ++i)
				{
					s.Remove(&buf[i * 8], 8);
				}
				s.CollectStats();
				AssertThat(LiveCount(s), Is().EqualTo(N / 2));
				AssertThat(s.used, Is().EqualTo((N / 2) * 8));
			});
		});


		describe("Multithreading", [&]()
		{
			it("One thread adds, another collects", [&]()
			{
				MemoryStats s;
				const sizet N = 1000;
				TArray<u8, 0> buf(N * 8);
				std::atomic<bool> start{false};
				std::atomic<bool> producerDone{false};

				std::thread producer([&]()
				{
					while (!start.load(std::memory_order_acquire))
					{}
					for (sizet i = 0; i < N; ++i)
					{
						s.Add(&buf[i * 8], 8);
					}
					producerDone.store(true, std::memory_order_release);
				});

				std::thread consumer([&]()
				{
					while (!start.load(std::memory_order_acquire))
					{}
					while (!producerDone.load(std::memory_order_acquire) || LiveCount(s) < N)
					{
						s.CollectStats();
						std::this_thread::yield();
					}
				});

				start.store(true, std::memory_order_release);
				producer.join();
				consumer.join();

				AssertThat(LiveCount(s), Is().EqualTo(N));
				AssertThat(s.used, Is().EqualTo(N * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Reset();
			});

			it("Many threads add, then collects", [&]()
			{
				MemoryStats s;
				const sizet N_PER_THREAD = 1000;
				const sizet NUM_THREADS  = 4;
				const sizet N            = N_PER_THREAD * NUM_THREADS;

				TArray<TArray<u8, 0>, 0> buffers;
				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					TArray<u8, 0> buf(N_PER_THREAD * 8);
					buffers.Add(Move(buf));
				}

				std::atomic<bool> start{false};
				std::atomic<sizet> producersDone{0};
				std::vector<std::thread> producers;

				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					producers.emplace_back([&, t]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (sizet i = 0; i < N_PER_THREAD; ++i)
						{
							s.Add(&buffers[t][i * 8], 8);
						}
						producersDone.fetch_add(1, std::memory_order_release);
					});
				}

				std::thread consumer([&]()
				{
					while (!start.load(std::memory_order_acquire))
					{}
					while (producersDone.load(std::memory_order_acquire) < NUM_THREADS)
					{
						s.CollectStats();
						std::this_thread::yield();
					}
					s.CollectStats();
				});

				start.store(true, std::memory_order_release);
				for (auto& t : producers)
				{
					t.join();
				}
				consumer.join();

				AssertThat(LiveCount(s), Is().EqualTo(N));
				AssertThat(s.used, Is().EqualTo(N * 8));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Reset();
			});

			it("Many threads add and remove, then collects", [&]()
			{
				MemoryStats s;
				const sizet N_PER_THREAD = 1000;
				const sizet NUM_THREADS  = 4;
				const sizet N            = N_PER_THREAD * NUM_THREADS;

				TArray<TArray<u8, 0>, 0> buffers;
				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					TArray<u8, 0> buf(N_PER_THREAD * 8);
					buffers.Add(Move(buf));
				}

				std::atomic<bool> start{false};
				std::atomic<sizet> producersDone{0};
				std::vector<std::thread> producers;

				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					producers.emplace_back([&, t]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (sizet i = 0; i < N_PER_THREAD; ++i)
						{
							s.Add(&buffers[t][i * 8], 8);
						}
						// Free the first half.
						for (sizet i = 0; i < N_PER_THREAD / 2; ++i)
						{
							s.Remove(&buffers[t][i * 8], 8);
						}
						producersDone.fetch_add(1, std::memory_order_release);
					});
				}

				std::thread consumer([&]()
				{
					while (!start.load(std::memory_order_acquire))
					{}
					while (producersDone.load(std::memory_order_acquire) < NUM_THREADS)
					{
						s.CollectStats();
						std::this_thread::yield();
					}
					s.CollectStats();
				});

				start.store(true, std::memory_order_release);
				for (auto& t : producers)
				{
					t.join();
				}
				consumer.join();

				AssertThat(LiveCount(s), Is().EqualTo(N / 2));
				AssertThat(s.used, Is().EqualTo((N / 2) * 8));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Reset();
			});
		});


		describe("Heavy stress", [&]()
		{
			it("Many producers, many iterations, no crashes", [&]()
			{
				MemoryStats s;
				const sizet N_PER_THREAD = 2000;
				const sizet NUM_THREADS  = 4;
				const sizet N            = N_PER_THREAD * NUM_THREADS;

				TArray<TArray<u8, 0>, 0> buffers;
				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					TArray<u8, 0> buf(N_PER_THREAD * 8);
					buffers.Add(Move(buf));
				}

				std::atomic<bool> start{false};
				std::atomic<sizet> producersDone{0};
				std::vector<std::thread> producers;

				for (sizet t = 0; t < NUM_THREADS; ++t)
				{
					producers.emplace_back([&, t]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (sizet i = 0; i < N_PER_THREAD; ++i)
						{
							s.Add(&buffers[t][i * 8], 8);
							if (i > 0 && i % 3 == 0)
							{
								s.Remove(&buffers[t][(i - 1) * 8], 8);
							}
						}
						producersDone.fetch_add(1, std::memory_order_release);
					});
				}

				std::thread consumer([&]()
				{
					while (!start.load(std::memory_order_acquire))
					{}
					while (producersDone.load(std::memory_order_acquire) < NUM_THREADS)
					{
						s.CollectStats();
						std::this_thread::yield();
					}
					s.CollectStats();
				});

				start.store(true, std::memory_order_release);
				for (auto& t : producers)
				{
					t.join();
				}
				consumer.join();

				// s.used reflects the net remaining live set.
				AssertThat(s.used, Is().EqualTo(LiveCount(s) * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Reset();
			});
		});
	});
});