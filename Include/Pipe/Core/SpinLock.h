// Copyright 2015-2026 Piperift. All Rights Reserved.
// Based on https://github.com/fpikus/ConcurrentCpp

#pragma once

#include "PipePlatform.h"

#include <atomic>
#include <chrono>
#include <thread>


namespace p
{
	P_FORCEINLINE static void YieldLockThread() noexcept
	{
		std::this_thread::yield();
	}

	P_FORCEINLINE static void SleepLockBackoff() noexcept
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	/** Spinlock with test-and-test-and-set (TTAS) and two-tier backoff. */
	class SpinLock
	{
	private:
		std::atomic<i32> flag{0};

	public:
		SpinLock()                           = default;
		SpinLock(const SpinLock&)            = delete;
		SpinLock& operator=(const SpinLock&) = delete;

		// Acquire the lock, blocking (spinning, then sleeping) until held.
		void Lock() noexcept
		{
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					if (!(flag.load(std::memory_order_relaxed)
					        || flag.exchange(1, std::memory_order_acquire)))
					{
						return;
					}
				}
				if (spin_count < 8)    // first 8 spin_count: yield, stay hot
				{
					YieldLockThread();
				}
				else    // then escalate to a real sleep and start the yield budget over
				{
					spin_count = 0;
					SleepLockBackoff();
				}
			}    // spin/back-off loop
		}

		/** Bounded acquire: returns true holding the lock, false if it could not be taken within
		 * the short-spin budget.
		 */
		bool TryLock() noexcept
		{
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					if (!(flag.load(std::memory_order_relaxed)
					        || flag.exchange(1, std::memory_order_acquire)))
					{
						return true;
					}
				}
				if (spin_count < 8)    // spin/yield through the short tier...
				{
					YieldLockThread();
				}
				else    // ...then give up instead of long-sleeping
				{
					return false;
				}
			}    // bounded spin loop
			return true;    // for-condition saw the lock free and the exchange won it
		}

		// Release the lock.
		void Unlock() noexcept
		{
			flag.store(0, std::memory_order_release);
		}

		// Advisory, non-synchronizing peek. The answer may be stale the instant
		// it returns, so only use for asserts/diagnostics, never for exclusion.
		bool Locked() const noexcept
		{
			return flag.load(std::memory_order_relaxed) == 1;
		}
	};

	/** Shared (read-write) spinlock.
	 *
	 * Many readers may hold concurrently, but an exclusive writer excludes everyone.
	 * Writers wait for all readers to drain; readers wait for any pending writer.
	 */
	class SharedSpinLock
	{
	private:
		// Top bit of the reader counter is the exclusive (writer) flag. The
		// remaining bits count shared (reader) holders, so at most
		// 2^31 - 1 readers may hold simultaneously.
		static constexpr u32 exclusiveBit = 1u << 31;

		std::atomic<u32> flag{0};

	public:
		SharedSpinLock()                                 = default;
		SharedSpinLock(const SharedSpinLock&)            = delete;
		SharedSpinLock& operator=(const SharedSpinLock&) = delete;

		// Exclusive (writer) acquire. Waits while readers or a writer hold.
		void LockExclusive() noexcept
		{
			u32 current = 0;
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					current = flag.load(std::memory_order_relaxed);
					if (current == 0
					    && flag.compare_exchange_strong(current, exclusiveBit,
					        std::memory_order_acquire, std::memory_order_relaxed))
					{
						return;
					}
				}
				if (spin_count < 8)    // first 8 spin_count: yield, stay hot
				{
					YieldLockThread();
				}
				else    // then escalate to a real sleep and start the yield budget over
				{
					spin_count = 0;
					SleepLockBackoff();
				}
			}    // spin/back-off loop
		}

		// Bounded exclusive acquire, same semantics as SpinLock::TryLock: spins
		// and yields through the short budget before giving up.
		bool TryLockExclusive() noexcept
		{
			u32 current = 0;
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					current = flag.load(std::memory_order_relaxed);
					if (current == 0
					    && flag.compare_exchange_strong(current, exclusiveBit,
					        std::memory_order_acquire, std::memory_order_relaxed))
					{
						return true;
					}
				}
				if (spin_count < 8)    // spin/yield through the short tier...
				{
					YieldLockThread();
				}
				else    // ...then give up instead of long-sleeping
				{
					return false;
				}
			}    // bounded spin loop
		}

		void UnlockExclusive() noexcept
		{
			flag.store(0, std::memory_order_release);
		}

		// Shared (reader) acquire. Waits while a writer holds or is pending.
		void LockShared() noexcept
		{
			u32 current = 0;
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					current = flag.load(std::memory_order_relaxed);
					if ((current & exclusiveBit) == 0
					    && flag.compare_exchange_strong(current, current + 1,
					        std::memory_order_acquire, std::memory_order_relaxed))
					{
						return;
					}
				}
				if (spin_count < 8)    // first 8 spin_count: yield, stay hot
				{
					YieldLockThread();
				}
				else    // then escalate to a real sleep and start the yield budget over
				{
					spin_count = 0;
					SleepLockBackoff();
				}
			}    // spin/back-off loop
		}

		// Bounded shared acquire, same semantics as SpinLock::TryLock.
		bool TryLockShared() noexcept
		{
			u32 current = 0;
			for (int spin_count = 0;; ++spin_count)
			{
				for (i32 i = 0; i < 8; ++i)
				{
					current = flag.load(std::memory_order_relaxed);
					if ((current & exclusiveBit) == 0
					    && flag.compare_exchange_strong(current, current + 1,
					        std::memory_order_acquire, std::memory_order_relaxed))
					{
						return true;
					}
				}
				if (spin_count < 8)    // spin/yield through the short tier...
				{
					YieldLockThread();
				}
				else    // ...then give up instead of long-sleeping
				{
					return false;
				}
			}    // bounded spin loop
			return true;    // for-condition saw the lock free and the exchange won it
		}

		void UnlockShared() noexcept
		{
			flag.fetch_sub(1, std::memory_order_release);
		}
	};

	// RAII guard for SpinLock.
	class ScopedLock
	{
	public:
		explicit ScopedLock(SpinLock& lock) noexcept : lock(lock)
		{
			lock.Lock();
		}
		~ScopedLock()
		{
			lock.Unlock();
		}

		ScopedLock(const ScopedLock&)            = delete;
		ScopedLock& operator=(const ScopedLock&) = delete;

	private:
		SpinLock& lock;
	};

	// RAII guard for the exclusive (writer) side of SharedSpinLock.
	class ExclusiveScopedLock
	{
	public:
		explicit ExclusiveScopedLock(SharedSpinLock& lock) noexcept : lock(lock)
		{
			lock.LockExclusive();
		}
		~ExclusiveScopedLock()
		{
			lock.UnlockExclusive();
		}

		ExclusiveScopedLock(const ExclusiveScopedLock&)            = delete;
		ExclusiveScopedLock& operator=(const ExclusiveScopedLock&) = delete;

	private:
		SharedSpinLock& lock;
	};

	// RAII guard for the shared (reader) side of SharedSpinLock.
	class SharedScopedLock
	{
	public:
		explicit SharedScopedLock(SharedSpinLock& lock) noexcept : lock(lock)
		{
			lock.LockShared();
		}
		~SharedScopedLock()
		{
			lock.UnlockShared();
		}

		SharedScopedLock(const SharedScopedLock&)            = delete;
		SharedScopedLock& operator=(const SharedScopedLock&) = delete;

	private:
		SharedSpinLock& lock;
	};
}    // namespace p
