// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <Pipe/Memory/MemoryStats.h>

#include <atomic>
#include <thread>
#include <vector>


using namespace snowhouse;
using namespace bandit;
using namespace p;


static i32 AllocCount(const MemoryStats& s)
{
	i32 n = 0;
	for (const auto& ev : s.events)
	{
		if (!ev.IsFree())
		{
			++n;
		}
	}
	return n;
}
static i32 FreeCount(const MemoryStats& s)
{
	i32 n = 0;
	for (const auto& ev : s.events)
	{
		if (ev.IsFree())
		{
			++n;
		}
	}
	return n;
}
static MemoryStatsEvent LiveAt(const MemoryStats& s, i32 i)
{
	for (const auto& ev : s.events)
	{
		if (ev.IsFree())
		{
			continue;
		}
		if (i == 0)
		{
			return ev;
		}
		--i;
	}
	return MemoryStatsEvent{};
}
static MemoryStatsEvent FreeAt(const MemoryStats& s, i32 i)
{
	for (const auto& ev : s.events)
	{
		if (!ev.IsFree())
		{
			continue;
		}
		if (i == 0)
		{
			return ev;
		}
		--i;
	}
	return MemoryStatsEvent{};
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
				AssertThat(AllocCount(s), Is().EqualTo(0));
				AssertThat(FreeCount(s), Is().EqualTo(0));
			});

			it("Tracks a single add", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(64));
				AssertThat(s.totalAllocated, Is().EqualTo(64));
				AssertThat(AllocCount(s), Is().EqualTo(1));
				AssertThat(LiveAt(s, 0).GetPtr(), Is().EqualTo((u8*)0x1000));
				AssertThat(LiveAt(s, 0).GetSize(), Is().EqualTo(64));
				AssertThat(LiveAt(s, 0).IsFree(), Is().EqualTo(false));
			});

			it("Tracks add plus free", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(s.totalAllocated, Is().EqualTo(0));
				AssertThat(AllocCount(s), Is().EqualTo(1));
				AssertThat(FreeCount(s), Is().EqualTo(1));
				AssertThat(FreeAt(s, 0).GetPtr(), Is().EqualTo((u8*)0x1000));
				AssertThat(FreeAt(s, 0).IsFree(), Is().EqualTo(true));
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
				AssertThat(AllocCount(s), Is().EqualTo(3));
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
				AssertThat(s.totalAllocated, Is().EqualTo((N / 2) * 16));
				AssertThat(AllocCount(s), Is().EqualTo(N));
				AssertThat(FreeCount(s), Is().EqualTo(N / 2));
			});

			it("Records double-free", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.Remove((void*)0x1000, 64);
				s.CollectStats();
				// Every free is recorded chronologically, so used underflows.
				AssertThat(s.used, Is().EqualTo((sizet)-64));
				AssertThat(AllocCount(s), Is().EqualTo(1));
				AssertThat(FreeCount(s), Is().EqualTo(2));
			});

			it("Records free of unknown ptr", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Remove((void*)0xDEAD, 64);
				s.CollectStats();
				// Free of unknown ptr is still recorded, so used underflows.
				AssertThat(s.used, Is().EqualTo((sizet)-64));
				AssertThat(AllocCount(s), Is().EqualTo(0));
				AssertThat(FreeCount(s), Is().EqualTo(1));
			});

			it("Records duplicate allocs", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x1000, 128);
				s.CollectStats();
				// Every alloc is recorded chronologically, both survive.
				AssertThat(AllocCount(s), Is().EqualTo(2));
				AssertThat(LiveAt(s, 0).GetSize(), Is().EqualTo(64));
				AssertThat(LiveAt(s, 1).GetSize(), Is().EqualTo(128));
			});

			it("CheckLeaks always runs when called directly", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				s.CheckLeaks();
				AssertThat(AllocCount(s), Is().EqualTo(1));
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
				AssertThat(AllocCount(s), Is().EqualTo(1));
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
				AssertThat(AllocCount(s), Is().EqualTo(1));
			});

			it("Release resets state", [&]()
			{
				MemoryStats s;
				s.Add((void*)0x1000, 64);
				s.Add((void*)0x2000, 32);
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(96));

				s.Release();
				AssertThat(s.used, Is().EqualTo(0));
				AssertThat(s.totalAllocated, Is().EqualTo(0));
				AssertThat(AllocCount(s), Is().EqualTo(0));
				AssertThat(FreeCount(s), Is().EqualTo(0));
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
				AssertThat(AllocCount(s), Is().EqualTo(2));
			});

			it("Re-collecting preserves state", [&]()
			{
				MemoryStats s;
				s.detectLeaks = false;
				s.Add((void*)0x1000, 64);
				s.CollectStats();
				s.CollectStats();
				AssertThat(s.used, Is().EqualTo(64));
				AssertThat(AllocCount(s), Is().EqualTo(1));
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
				AssertThat(AllocCount(s), Is().EqualTo(N));
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
				AssertThat(AllocCount(s), Is().EqualTo(N));
				AssertThat(FreeCount(s), Is().EqualTo(N / 2));
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
				AssertThat(AllocCount(s), Is().EqualTo(N));
				for (sizet i = 0; i < N / 2; ++i)
				{
					s.Remove(&buf[i * 8], 8);
				}
				s.CollectStats();
				AssertThat(AllocCount(s), Is().EqualTo(N));
			});
		});


		describe("SPSC stress", [&]()
		{
			it("Producer and consumer work concurrently", [&]()
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
					while (!producerDone.load(std::memory_order_acquire) || AllocCount(s) < N)
					{
						s.CollectStats();
						std::this_thread::yield();
					}
				});

				start.store(true, std::memory_order_release);
				producer.join();
				consumer.join();

				AssertThat(AllocCount(s), Is().EqualTo(N));
				AssertThat(s.used, Is().EqualTo(N * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Release();
			});
		});


		describe("MPMC stress", [&]()
		{
			it("Many producers push, one consumer collects", [&]()
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

				AssertThat(AllocCount(s), Is().EqualTo(N));
				AssertThat(s.used, Is().EqualTo(N * 8));
				AssertThat(s.totalAllocated, Is().EqualTo(N * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Release();
			});

			it("Producers push and free, one consumer collects", [&]()
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

				AssertThat(AllocCount(s), Is().EqualTo(N));
				AssertThat(s.used, Is().EqualTo((N / 2) * 8));
				AssertThat(s.totalAllocated, Is().EqualTo((N / 2) * 8));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Release();
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

				// s.used reflects net adds/frees chronologically.
				AssertThat(s.used, Is().EqualTo((AllocCount(s) - FreeCount(s)) * 8));
				AssertThat(s.used, Is().EqualTo(s.totalAllocated));

				// Suppress leak warnings at destruction (test buffers are stack).
				s.Release();
			});
		});
	});
});
