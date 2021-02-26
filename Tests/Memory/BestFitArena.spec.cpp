// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/Arenas/BestFitArena.h>
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
	describe("Memory", []() {
		describe("Best Fit Arena", []() {
			it("Reserves a block on construction", [&]() {
				BestFitArena arena{1024};
				AssertThat(arena.GetAvailableSpace(), Equals(1024));
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

				Rift::u8* blockPtr = static_cast<Rift::u8*>(*arena.GetBlock());

				void* p = arena.Allocate(4);
				new (p) TypeOfSize<4>();
				void* expectedP = blockPtr + Rift::GetAlignmentPaddingWithHeader(blockPtr, 8, 8);
				AssertThat(p, Is().EqualTo(expectedP));

				void* p2 = arena.Allocate(4);
				new (p2) TypeOfSize<4>();
				void* expectedP2 =
				    static_cast<Rift::u8*>(p) + 8 + Rift::GetAlignmentPaddingWithHeader(p, 8, 8);
				AssertThat(p2, Is().EqualTo(expectedP2));
			});

			it("Detects there is not enough space", [&]() {
				BestFitArena arena{32};

				// 16 bytes
				void* p     = arena.Allocate(8);
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
				BestFitArena arena{1024};

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
				BestFitArena arena{64};

				void* p = arena.Allocate(32);
				new (p) TypeOfSize<32>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(24));

				arena.Free(p);
				AssertThat(arena.GetAvailableSpace(), Equals(64));
			});

			it("Can free multiple", [&]() {
				BestFitArena arena{64};

				void* p = arena.Allocate(16);
				new (p) TypeOfSize<16>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(40));

				void* p2 = arena.Allocate(16);
				new (p2) TypeOfSize<16>();
				AssertThat(p2, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(16));

				arena.Free(p2);
				AssertThat(arena.GetAvailableSpace(), Equals(40));

				arena.Free(p);
				AssertThat(arena.GetAvailableSpace(), Equals(64));
			});

			it("Can free in between allocations", [&]() {
				BestFitArena arena{64};

				void* p = arena.Allocate(16);
				new (p) TypeOfSize<16>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(40));

				void* p2 = arena.Allocate(16);
				new (p2) TypeOfSize<16>();
				AssertThat(p2, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(16));
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				void* p3 = arena.Allocate(8);
				new (p3) TypeOfSize<8>();
				AssertThat(p3, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(0));
				// No space left, no free slots
				AssertThat(arena.GetFreeSlots().Size(), Equals(0));

				arena.Free(p2);
				AssertThat(arena.GetAvailableSpace(), Equals(24));
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));
				AssertThat(arena.GetFreeSlots()[0].start, Equals(static_cast<Rift::u8*>(p2) - 8));
				AssertThat(arena.GetFreeSlots()[0].end, Equals(static_cast<Rift::u8*>(p3) - 8));
			});

			it("Can merge previous and next slots on free", [&]() {
				BestFitArena arena{64};

				void* p = arena.Allocate(16);
				new (p) TypeOfSize<16>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(40));

				void* p2 = arena.Allocate(16);
				new (p2) TypeOfSize<16>();
				AssertThat(p2, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(16));
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				void* p3 = arena.Allocate(8);
				new (p3) TypeOfSize<8>();
				AssertThat(p3, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(0));

				// No space left, no free slots
				AssertThat(arena.GetFreeSlots().Size(), Equals(0));

				arena.Free(p);
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				arena.Free(p3);
				AssertThat(arena.GetFreeSlots().Size(), Equals(2));

				arena.Free(p2);    // Slots previous and next are merged
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				// Slot contains the entire memory block
				AssertThat(arena.GetFreeSlots()[0].start,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
				AssertThat(arena.GetFreeSlots()[0].end,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
			});

			it("Can merge previous slot on free", [&]() {
				BestFitArena arena{48};

				void* p = arena.Allocate(16);
				new (p) TypeOfSize<16>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(24));

				void* p2 = arena.Allocate(16);
				new (p2) TypeOfSize<16>();
				AssertThat(p2, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(0));
				AssertThat(arena.GetFreeSlots().Size(), Equals(0));

				arena.Free(p);
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				arena.Free(p2);    // Slot is expanded from the front
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				// Slot contains the entire memory block
				AssertThat(arena.GetFreeSlots()[0].start,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
				AssertThat(arena.GetFreeSlots()[0].end,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
			});

			it("Can merge next slot on free", [&]() {
				BestFitArena arena{48};

				void* p = arena.Allocate(16);
				new (p) TypeOfSize<16>();
				AssertThat(p, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(24));

				void* p2 = arena.Allocate(16);
				new (p2) TypeOfSize<16>();
				AssertThat(p2, Is().Not().Null());
				AssertThat(arena.GetAvailableSpace(), Equals(0));
				AssertThat(arena.GetFreeSlots().Size(), Equals(0));

				arena.Free(p2);
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				arena.Free(p);    // Slot is expanded from the back
				AssertThat(arena.GetFreeSlots().Size(), Equals(1));

				// Slot contains the entire memory block
				AssertThat(arena.GetFreeSlots()[0].start,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetData())));
				AssertThat(arena.GetFreeSlots()[0].end,
				    Equals(static_cast<const Rift::u8*>(arena.GetBlock().GetEnd())));
			});
		});
	});
});
