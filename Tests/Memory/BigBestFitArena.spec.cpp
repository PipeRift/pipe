// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/Arenas/BigBestFitArena.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;

template <Rift::u32 Size>
struct TypeOfSize
{
	static constexpr Rift::u32 size = Size;
	Rift::u8 data[size]{0};    // Fill data for debugging
};


go_bandit([]() {
	describe("Memory.BigBestFitArena", []() {
		it("Reserves a block on construction", [&]() {
			BigBestFitArena arena{1024};
			AssertThat(arena.GetFreeSize(), Equals(1024));
			AssertThat(*arena.GetBlock(), Is().Not().Null());
			AssertThat(arena.GetBlock().GetSize(), Is().EqualTo(1024));
		});

		it("Can allocate", [&]() {
			BigBestFitArena arena{1024};

			void* p = arena.Allocate(4);
			new (p) TypeOfSize<4>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.Contains(p), Is().True());
		});

		it("Allocates at correct addresses", [&]() {
			BigBestFitArena arena{1024};

			const auto* blockPtr = static_cast<const Rift::u8*>(*arena.GetBlock());

			void* p = arena.Allocate(4);
			new (p) TypeOfSize<4>();
			const void* expectedP = blockPtr + Rift::GetAlignmentPaddingWithHeader(blockPtr, 8, 8);
			AssertThat(p, Is().EqualTo(expectedP));

			void* p2 = arena.Allocate(4);
			new (p2) TypeOfSize<4>();
			void* expectedP2 =
			    static_cast<Rift::u8*>(p) + 8 + Rift::GetAlignmentPaddingWithHeader(p, 8, 8);
			AssertThat(p2, Is().EqualTo(expectedP2));
		});

		it("Detects there is not enough space", [&]() {
			BigBestFitArena arena{32};

			// 16 bytes
			void* p = arena.Allocate(8);
			new (p) TypeOfSize<8>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.Contains(p), Is().True());

			// Another 16 bytes
			void* p2 = arena.Allocate(4);
			new (p2) TypeOfSize<4>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.Contains(p2), Is().True());

			// No more space, return null
			void* p3 = arena.Allocate(8);    // 8 bytes
			AssertThat(p3, Is().Null());
		});

		it("Allocates with alignment", [&]() {
			BigBestFitArena arena{1024};

			void* b = arena.Allocate(1);
			new (b) TypeOfSize<1>();

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Allocate(4, 8);
			new (p) TypeOfSize<4>();
			AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Allocate(4, 16);
			new (p2) TypeOfSize<4>();
			AssertThat(Rift::GetAlignmentPadding(p2, 16), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p3 = arena.Allocate(8, 32);
			new (p3) TypeOfSize<8>();
			AssertThat(Rift::GetAlignmentPadding(p3, 32), Is().EqualTo(0));
		});

		it("Can free", [&]() {
			BigBestFitArena arena{64};

			void* p = arena.Allocate(32);
			new (p) TypeOfSize<32>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(24));

			arena.Free(p, 32);
			AssertThat(arena.GetFreeSize(), Equals(64));
		});

		it("Can free multiple", [&]() {
			BigBestFitArena arena{64};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(40));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(16));

			arena.Free(p2, 16);
			AssertThat(arena.GetFreeSize(), Equals(40));

			arena.Free(p, 16);
			AssertThat(arena.GetFreeSize(), Equals(64));
		});

		it("Can free in between allocations", [&]() {
			BigBestFitArena arena{64};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(40));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(16));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			void* p3 = arena.Allocate(8);
			new (p3) TypeOfSize<8>();
			AssertThat(p3, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			// No space left, no free slots
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p2, 16);
			AssertThat(arena.GetFreeSize(), Equals(24));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));
			AssertThat(arena.GetFreeSlots()[0].start, Equals(static_cast<Rift::u8*>(p2) - 8));
			AssertThat(arena.GetFreeSlots()[0].end, Equals(static_cast<Rift::u8*>(p3) - 8));
		});

		it("Can merge previous and next slots on free", [&]() {
			BigBestFitArena arena{64};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(40));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(16));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			void* p3 = arena.Allocate(8);
			new (p3) TypeOfSize<8>();
			AssertThat(p3, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));

			// No space left, no free slots
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p, 16);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p3, 8);
			AssertThat(arena.GetFreeSlots().Size(), Equals(2));

			arena.Free(p2, 16);    // Slots previous and next are merged
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
			AssertThat(arena.GetFreeSlots()[0].end,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
		});

		it("Can merge previous slot on free", [&]() {
			BigBestFitArena arena{48};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(24));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p, 16);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p2, 16);    // Slot is expanded from the front
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
			AssertThat(arena.GetFreeSlots()[0].end,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
		});

		it("Can merge next slot on free", [&]() {
			BigBestFitArena arena{48};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(24));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p2, 16);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p, 16);    // Slot is expanded from the back
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
			AssertThat(arena.GetFreeSlots()[0].end,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
		});

		it("Ensures a big alignment leaves a gap", [&]() {
			BigBestFitArena arena{128};

			// We ensure first allocation aligns the block (just for the test)
			void* p = arena.Allocate(8);
			new (p) TypeOfSize<8>();
			AssertThat(arena.GetFreeSize(), Equals(112));

			void* p2 = arena.Allocate(8, 64);
			new (p2) TypeOfSize<8>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(96));
			AssertThat(arena.GetFreeSlots().Size(), Equals(2));

			// Slot contains the rest if the block
			AssertThat(arena.GetFreeSlots()[0].start, Equals(arena.GetAllocationEnd(p2)));
			AssertThat(arena.GetFreeSlots()[0].end,
			    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));

			// Slot contains the alignment gap
			AssertThat(arena.GetFreeSlots()[1].start, Equals(arena.GetAllocationEnd(p)));
			AssertThat(arena.GetFreeSlots()[1].end, Equals(arena.GetAllocationStart(p2)));
		});
	});
});
