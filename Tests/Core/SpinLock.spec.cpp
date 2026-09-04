// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Core/SpinLock.h>

#include <atomic>
#include <thread>
#include <vector>


using namespace p;


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Core.SpinLock", []()
{
	Describe("SpinLock", []()
	{
		It("Acquires and releases exclusively", []()
		{
			SpinLock lock;
			ScopedLock guard(lock);

			Expect(lock.Locked()).ToBeTrue();
			Expect(lock.TryLock()).ToBeFalse();
		});

		It("Allows serialized writers to increment a counter", []()
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

			Expect(counter).ToEqual(kThreads * kPerThread);
		});
	});

	Describe("SharedSpinLock", []()
	{
		It("Exclusive lock excludes a second exclusive lock", []()
		{
			SharedSpinLock lock;
			ExclusiveScopedLock writer(lock);

			Expect(lock.TryLockExclusive()).ToBeFalse();
		});

		It("Exclusive lock excludes shared locks", []()
		{
			SharedSpinLock lock;
			ExclusiveScopedLock writer(lock);

			Expect(lock.TryLockShared()).ToBeFalse();
		});

		It("Shared lock excludes an exclusive lock", []()
		{
			SharedSpinLock lock;
			SharedScopedLock reader(lock);

			Expect(lock.TryLockExclusive()).ToBeFalse();
		});

		It("Allows multiple overlapping shared locks", []()
		{
			SharedSpinLock lock;

			SharedScopedLock r1(lock);
			SharedScopedLock r2(lock);
			SharedScopedLock r3(lock);

			// Readers coexist: shared still acquirable.
			Expect(lock.TryLockShared()).ToBeTrue();
			lock.UnlockShared();

			Expect(lock.TryLockExclusive()).ToBeFalse();
		});

		It("Writers exclude each other", []()
		{
			SharedSpinLock lock;

			ExclusiveScopedLock w1(lock);
			Expect(lock.TryLockExclusive()).ToBeFalse();
		});

		It("Writes under exclusive lock are mutually excluded", []()
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

			Expect(counter).ToEqual(kThreads * kPerThread);
		});

		It("Shared readers run concurrently without tearing shared state", []()
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

			Expect(reads.load()).ToEqual(kThreads * kIterations);
		});
	});
});
return true;
}();
}    // namespace
