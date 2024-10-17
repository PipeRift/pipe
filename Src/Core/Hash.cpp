// Copyright 2015-2024 Piperift - All rights reserved
// Based on martinus/robin-hood-hashing

#include "Pipe/Core/Hash.h"

#include "Pipe/Memory/Memory.h"


namespace p
{
#ifndef P_FALLTHROUGH
	#if __has_cpp_attribute(clang::fallthrough)
		#define P_FALLTHROUGH [[clang::fallthrough]]
	#elif __has_cpp_attribute(gnu::fallthrough)
		#define P_FALLTHROUGH [[gnu::fallthrough]]
	#else
		#define P_FALLTHROUGH
	#endif
#endif


	template<typename T>
	inline T UnalignedLoad(void const* ptr) noexcept
	{
		// using memcpy so we don't get into unaligned load problems.
		// compiler should optimize this very well anyways.
		T t;
		CopyMem(&t, ptr, sizeof(T));
		return t;
	}

	sizet HashBytes(const void* ptr, const sizet len)
	{
		static constexpr u64 m          = UINT64_C(0xc6a4a7935bd1e995);
		static constexpr u64 seed       = UINT64_C(0xe17a1465);
		static constexpr unsigned int r = 47;

		auto const* const data64 = static_cast<const u64*>(ptr);
		u64 h                    = seed ^ (len * m);

		sizet const n_blocks = len / 8;
		for (sizet i = 0; i < n_blocks; ++i)
		{
			auto k = UnalignedLoad<u64>(data64 + i);

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		const auto* const data8 = reinterpret_cast<const u8*>(data64 + n_blocks);
		switch (len & 7U)
		{
			case 7: h ^= static_cast<u64>(data8[6]) << 48U; P_FALLTHROUGH;
			case 6: h ^= static_cast<u64>(data8[5]) << 40U; P_FALLTHROUGH;
			case 5: h ^= static_cast<u64>(data8[4]) << 32U; P_FALLTHROUGH;
			case 4: h ^= static_cast<u64>(data8[3]) << 24U; P_FALLTHROUGH;
			case 3: h ^= static_cast<u64>(data8[2]) << 16U; P_FALLTHROUGH;
			case 2: h ^= static_cast<u64>(data8[1]) << 8U; P_FALLTHROUGH;
			case 1:
				h ^= static_cast<u64>(data8[0]);
				h *= m;
				P_FALLTHROUGH;
			default: break;
		}

		h ^= h >> r;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// h *= m;
		// h ^= h >> r;
		return static_cast<sizet>(h);
	}

	sizet HashInt(u64 x)
	{
		// tried lots of different hashes, let's stick with murmurhash3. It's simple, fast, well
		// tested, and doesn't need any special 128bit operations.
		x ^= x >> 33U;
		x *= UINT64_C(0xff51afd7ed558ccd);
		x ^= x >> 33U;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// x *= UINT64_C(0xc4ceb9fe1a85ec53);
		// x ^= x >> 33U;
		return static_cast<sizet>(x);
	}
}    // namespace p