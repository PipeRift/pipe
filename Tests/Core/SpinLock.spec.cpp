// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <Pipe/Core/SpinLock.h>

#include <atomic>
#include <thread>
#include <vector>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]()
{
	describe("Core.SpinLock", []()
	{
		describe("SpinLock", [&]()
		{
			it("Acquires and releases exclusively", [&]()
			{
				SpinLock lock;
				ScopedLock guard(lock);

				AssertThat(lock.Locked(), Is().True());
				AssertThat(lock.TryLock(), Is().False());
			});

			it("Allows serialized writers to increment a counter", [&]()
			{
				SpinLock lock;
				i32 counter = 0;

				constexpr i32 kThreads = 4;
				constexpr i32 kPerThread = 10'000;

				std::vector<std::thread> threads;
				std::atomic<bool> start{false};
				for (i32 t = 0; t < kThreads; ++t)
				{
					threads.emplace_back([&]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (i32 i = 0; i < kPerThread; ++i)
						{
							ScopedLock guard(lock);
							++counter;
						}
					});
				}

				start.store(true, std::memory_order_release);
				for (auto& thread : threads)
				{
					thread.join();
				}

				AssertThat(counter, Is().EqualTo(kThreads * kPerThread));
			});
		});

		describe("SharedSpinLock", [&]()
		{
			it("Exclusive lock excludes a second exclusive lock", [&]()
			{
				SharedSpinLock lock;
				ExclusiveScopedLock writer(lock);

				AssertThat(lock.TryLockExclusive(), Is().False());
			});

			it("Exclusive lock excludes shared locks", [&]()
			{
				SharedSpinLock lock;
				ExclusiveScopedLock writer(lock);

				AssertThat(lock.TryLockShared(), Is().False());
			});

			it("Shared lock excludes an exclusive lock", [&]()
			{
				SharedSpinLock lock;
				SharedScopedLock reader(lock);

				AssertThat(lock.TryLockExclusive(), Is().False());
			});

			it("Allows multiple overlapping shared locks", [&]()
			{
				SharedSpinLock lock;

				SharedScopedLock r1(lock);
				SharedScopedLock r2(lock);
				SharedScopedLock r3(lock);

				// Readers coexist: shared still acquirable.
				AssertThat(lock.TryLockShared(), Is().True());
				lock.UnlockShared();

				AssertThat(lock.TryLockExclusive(), Is().False());
			});

			it("Writers exclude each other", [&]()
			{
				SharedSpinLock lock;

				ExclusiveScopedLock w1(lock);
				AssertThat(lock.TryLockExclusive(), Is().False());
			});

			it("Writes under exclusive lock are mutually excluded", [&]()
			{
				SharedSpinLock lock;
				i32 counter = 0;

				constexpr i32 kThreads = 4;
				constexpr i32 kPerThread = 10'000;

				std::vector<std::thread> threads;
				std::atomic<bool> start{false};
				for (i32 t = 0; t < kThreads; ++t)
				{
					threads.emplace_back([&]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (i32 i = 0; i < kPerThread; ++i)
						{
							ExclusiveScopedLock writer(lock);
							++counter;
						}
					});
				}

				start.store(true, std::memory_order_release);
				for (auto& thread : threads)
				{
					thread.join();
				}

				AssertThat(counter, Is().EqualTo(kThreads * kPerThread));
			});

			it("Shared readers run concurrently without tearing shared state", [&]()
			{
				SharedSpinLock lock;
				i32 value = 0;

				constexpr i32 kThreads = 4;
				constexpr i32 kIterations = 10'000;

				// Shared-side readers are allowed to overlap, so they must only
				// read. This just checks that many threads can take the shared
				// side simultaneously without deadlocking or corrupting the lock.
				std::vector<std::thread> threads;
				std::atomic<bool> start{false};
				std::atomic<i32> reads{0};
				for (i32 t = 0; t < kThreads; ++t)
				{
					threads.emplace_back([&]()
					{
						while (!start.load(std::memory_order_acquire))
						{}
						for (i32 i = 0; i < kIterations; ++i)
						{
							SharedScopedLock reader(lock);
							const i32 v = value;
							(void)v;
							reads.fetch_add(1, std::memory_order_relaxed);
						}
					});
				}

				start.store(true, std::memory_order_release);
				for (auto& thread : threads)
				{
					thread.join();
				}

				AssertThat(reads.load(), Is().EqualTo(kThreads * kIterations));
			});
		});
	});
});
