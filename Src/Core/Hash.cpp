// Copyright 2015-2024 Piperift - All rights reserved
// Based on martinus/robin-hood-hashing

#include "Pipe/Core/Hash.h"

#include "PipeMemory.h"


namespace p
{
	namespace detail
	{
		template<typename T>
		T UnalignedLoad(void const* ptr) noexcept
		{
			// using memcpy so we don't get into unaligned load problems.
			// compiler should optimize this very well anyways.
			T t;
			CopyMem(&t, ptr, sizeof(T));
			return t;
		}

		// hash_mix for 64 bit size_t
		//
		// The general "xmxmx" form of state of the art 64 bit mixers originates
		// from Murmur3 by Austin Appleby, which uses the following function as
		// its "final mix":
		//
		//	k ^= k >> 33;
		//	k *= 0xff51afd7ed558ccd;
		//	k ^= k >> 33;
		//	k *= 0xc4ceb9fe1a85ec53;
		//	k ^= k >> 33;
		//
		// (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp)
		//
		// It has subsequently been improved multiple times by different authors
		// by changing the constants. The most well known improvement is the
		// so-called "variant 13" function by David Stafford:
		//
		//	k ^= k >> 30;
		//	k *= 0xbf58476d1ce4e5b9;
		//	k ^= k >> 27;
		//	k *= 0x94d049bb133111eb;
		//	k ^= k >> 31;
		//
		// (https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html)
		//
		// This mixing function is used in the splitmix64 RNG:
		// http://xorshift.di.unimi.it/splitmix64.c
		//
		// We use Jon Maiga's implementation from
		// http://jonkagstrom.com/mx3/mx3_rev2.html
		//
		// 	x ^= x >> 32;
		//	x *= 0xe9846af9b1a615d;
		//	x ^= x >> 32;
		//	x *= 0xe9846af9b1a615d;
		//	x ^= x >> 28;
		//
		// An equally good alternative is Pelle Evensen's Moremur:
		//
		//	x ^= x >> 27;
		//	x *= 0x3C79AC492BA7B653;
		//	x ^= x >> 33;
		//	x *= 0x1C69B3F74AC4AE35;
		//	x ^= x >> 27;
		//
		// (https://mostlymangling.blogspot.com/2019/12/stronger-better-morer-moremur-better.html)
		constexpr sizet HashMix(sizet x)
		{
			if constexpr (Limits<sizet>::digits == 64)
			{    // 64 bit mixing
				constexpr u64 m = 0xe9846af9b1a615d;
				x ^= x >> 32;
				x *= m;
				x ^= x >> 32;
				x *= m;
				x ^= x >> 28;
			}
			else
			{    // 32 bit mixing
				constexpr u32 m1 = 0x21f0aaad;
				constexpr u32 m2 = 0x735a2d97;
				x ^= x >> 16;
				x *= m1;
				x ^= x >> 15;
				x *= m2;
				x ^= x >> 15;
			}
			return x;
		}
	}    // namespace detail

#ifndef P_FALLTHROUGH
	#if __has_cpp_attribute(clang::fallthrough)
		#define P_FALLTHROUGH [[clang::fallthrough]]
	#elif __has_cpp_attribute(gnu::fallthrough)
		#define P_FALLTHROUGH [[gnu::fallthrough]]
	#else
		#define P_FALLTHROUGH
	#endif
#endif


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
			auto k = detail::UnalignedLoad<u64>(data64 + i);

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

	sizet HashCombine(sizet a, sizet b)
	{
		return detail::HashMix(a + 0x9e3779b9 + b);
	}
}    // namespace p
