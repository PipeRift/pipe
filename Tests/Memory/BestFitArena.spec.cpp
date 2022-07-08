// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Memory/Arenas/BestFitArena.h>
#include <Pipe/Memory/Memory.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;
using namespace Memory;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


go_bandit([]() {
	describe("Memory.BestFitArena", []() {
		it("Reserves a block on construction", [&]() {
			BestFitArena arena{1024};
			AssertThat(arena.GetFreeSize(), Equals(1024));
			AssertThat(*arena.GetBlock(), Is().Not().Null());
			AssertThat(arena.GetBlock().GetSize(), Is().EqualTo(1024));
		});

		it("Can allocate", [&]() {
			BestFitArena arena{1024};

			void* p = arena.Allocate(4);
			new (p) TypeOfSize<4>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.Contains(p), Is().True());
		});

		it("Allocates at correct addresses", [&]() {
			BestFitArena arena{1024};

			const auto* blockPtr = static_cast<const u8*>(*arena.GetBlock());

			void* p = arena.Allocate(4);
			new (p) TypeOfSize<4>();
			AssertThat(p, Is().EqualTo(blockPtr));

			void* p2 = arena.Allocate(4);
			new (p2) TypeOfSize<4>();
			AssertThat(p2, Is().EqualTo(blockPtr + 4));
		});

		it("Detects there is not enough space", [&]() {
			BestFitArena arena{32};

			// 16 bytes
			void* p = arena.Allocate(20);
			new (p) TypeOfSize<20>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.Contains(p), Is().True());

			// Another 16 bytes
			void* p2 = arena.Allocate(6);
			new (p2) TypeOfSize<6>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.Contains(p2), Is().True());

			// No more space, return null
			void* p3 = arena.Allocate(8);    // 8 bytes
			AssertThat(p3, Is().Null());
		});

		it("Allocates with alignment", [&]() {
			BestFitArena arena{1024};

			void* b = arena.Allocate(1);
			new (b) TypeOfSize<1>();

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Allocate(4, 8);
			new (p) TypeOfSize<4>();
			AssertThat(GetAlignmentPadding(p, 8), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Allocate(4, 16);
			new (p2) TypeOfSize<4>();
			AssertThat(GetAlignmentPadding(p2, 16), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p3 = arena.Allocate(8, 32);
			new (p3) TypeOfSize<8>();
			AssertThat(GetAlignmentPadding(p3, 32), Is().EqualTo(0));
		});

		it("Can free", [&]() {
			BestFitArena arena{64};

			void* p = arena.Allocate(32);
			new (p) TypeOfSize<32>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(32));

			arena.Free(p, 32);
			AssertThat(arena.GetFreeSize(), Equals(64));
		});

		it("Can free multiple", [&]() {
			BestFitArena arena{64};

			void* p = arena.Allocate(16);
			new (p) TypeOfSize<16>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(48));

			void* p2 = arena.Allocate(16);
			new (p2) TypeOfSize<16>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(32));

			arena.Free(p2, 16);
			AssertThat(arena.GetFreeSize(), Equals(48));

			arena.Free(p, 16);
			AssertThat(arena.GetFreeSize(), Equals(64));
		});

		it("Can free in between allocations", [&]() {
			BestFitArena arena{64};

			void* p = arena.Allocate(32);
			new (p) TypeOfSize<32>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(32));

			void* p2 = arena.Allocate(30);
			new (p2) TypeOfSize<30>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(2));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			void* p3 = arena.Allocate(2);
			new (p3) TypeOfSize<2>();
			AssertThat(p3, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			// No space left, no free slots
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p2, 30);
			AssertThat(arena.GetFreeSize(), Equals(30));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));
			AssertThat(arena.GetFreeSlots()[0].start, Equals(p2));
			AssertThat(arena.GetFreeSlots()[0].GetEnd(), Equals(p3));
		});

		it("Can merge previous and next slots on free", [&]() {
			BestFitArena arena{64};

			void* p = arena.Allocate(9);
			new (p) TypeOfSize<9>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(55));

			void* p2 = arena.Allocate(50);
			new (p2) TypeOfSize<50>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(5));
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			void* p3 = arena.Allocate(5);
			new (p3) TypeOfSize<5>();
			AssertThat(p3, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));

			// No space left, no free slots
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p, 9);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p3, 5);
			AssertThat(arena.GetFreeSlots().Size(), Equals(2));

			arena.Free(p2, 50);    // Slots previous and next are merged
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));
			AssertThat(arena.GetFreeSlots()[0].size, Equals(64));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start, Equals(arena.GetBlock().GetData()));
			AssertThat(arena.GetFreeSlots()[0].GetEnd(), Equals(arena.GetBlock().GetEnd()));
		});

		it("Can merge previous slot on free", [&]() {
			BestFitArena arena{48};

			void* p = arena.Allocate(39);
			new (p) TypeOfSize<39>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(9));

			void* p2 = arena.Allocate(9);
			new (p2) TypeOfSize<9>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p, 39);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p2, 9);    // Slot is expanded from the front
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));
			AssertThat(arena.GetFreeSlots()[0].size, Equals(48));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start, Equals(arena.GetBlock().GetData()));
			AssertThat(arena.GetFreeSlots()[0].GetEnd(), Equals(arena.GetBlock().GetEnd()));
		});

		it("Can merge next slot on free", [&]() {
			BestFitArena arena{48};

			void* p = arena.Allocate(24);
			new (p) TypeOfSize<24>();
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(24));

			void* p2 = arena.Allocate(24);
			new (p2) TypeOfSize<24>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(0));
			AssertThat(arena.GetFreeSlots().Size(), Equals(0));

			arena.Free(p2, 24);
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));

			arena.Free(p, 24);    // Slot is expanded from the back
			AssertThat(arena.GetFreeSlots().Size(), Equals(1));
			AssertThat(arena.GetFreeSlots()[0].size, Equals(48));

			// Slot contains the entire memory block
			AssertThat(arena.GetFreeSlots()[0].start, Equals(arena.GetBlock().GetData()));
			AssertThat(arena.GetFreeSlots()[0].GetEnd(), Equals(arena.GetBlock().GetEnd()));
		});

		it("Ensures a big alignment leaves a gap", [&]() {
			BestFitArena arena{128};

			// We ensure first allocation aligns the block (just for the test)
			void* p = arena.Allocate(8);
			new (p) TypeOfSize<8>();
			AssertThat(arena.GetFreeSize(), Equals(120));

			void* p2 = arena.Allocate(8, 64);
			new (p2) TypeOfSize<8>();
			AssertThat(p2, Is().Not().Null());
			AssertThat(arena.GetFreeSize(), Equals(112));

			AssertThat(arena.GetFreeSlots().Size(), Equals(2));

			// Slot contains the rest if the block
			AssertThat(arena.GetFreeSlots()[0].start, Equals((u8*)p2 + 8));
			AssertThat(arena.GetFreeSlots()[0].GetEnd(), Equals(arena.GetBlock().GetEnd()));

			// Slot contains the alignment gap
			AssertThat(arena.GetFreeSlots()[1].start, Equals((u8*)p + 8));
			AssertThat(arena.GetFreeSlots()[1].GetEnd(), Equals(p2));
		});
	});
});
