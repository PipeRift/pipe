// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeMemoryArenas.h>


using namespace p;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


void RegisterMemoryBestFitArenaTests()
{
	Spec("Memory.BestFitArena", []()
	{
		It("Reserves a block on construction", []()
		{
			BestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;
			Expect(arena.GetFreeSize()).ToEqual(1024);
			Expect(*arena.GetBlock()).ToNotEqual(nullptr);
			Expect(arena.GetBlock().size).ToEqual(1024);
		});

		It("Can allocate", []()
		{
			BestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(4);
			new (p) TypeOfSize<4>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.Contains(p)).ToBeTrue();
		});

		It("Allocates at correct addresses", []()
		{
			BestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			const auto* blockPtr = static_cast<const u8*>(*arena.GetBlock());

			void* p = arena.Alloc(4);
			new (p) TypeOfSize<4>();
			Expect(p).ToEqual(blockPtr);

			void* p2 = arena.Alloc(4);
			new (p2) TypeOfSize<4>();
			Expect(p2).ToEqual(blockPtr + 4);
		});

		It("Detects there is not enough space", []()
		{
			BestFitArena arena{32};
			arena.GetStats()->detectLeaks = false;

			// 16 bytes
			void* p = arena.Alloc(20);
			new (p) TypeOfSize<20>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.Contains(p)).ToBeTrue();

			// Another 16 bytes
			void* p2 = arena.Alloc(6);
			new (p2) TypeOfSize<6>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.Contains(p2)).ToBeTrue();

			// No more space, return null
			void* p3 = arena.Alloc(8);    // 8 bytes
			Expect(p3).ToEqual(nullptr);
		});

		It("Allocates with alignment", []()
		{
			BestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			void* b = arena.Alloc(1);
			new (b) TypeOfSize<1>();

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Alloc(4, 8);
			new (p) TypeOfSize<4>();
			Expect(GetAlignmentPadding(p, 8)).ToEqual(0);

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Alloc(4, 16);
			new (p2) TypeOfSize<4>();
			Expect(GetAlignmentPadding(p2, 16)).ToEqual(0);

			// When padding is 0 (last ptr is aligned)
			void* p3 = arena.Alloc(8, 32);
			new (p3) TypeOfSize<8>();
			Expect(GetAlignmentPadding(p3, 32)).ToEqual(0);
		});

		It("Can free", []()
		{
			BestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(32);
			new (p) TypeOfSize<32>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(32);

			arena.Free(p, 32);
			Expect(arena.GetFreeSize()).ToEqual(64);
		});

		It("Can free multiple", []()
		{
			BestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(48);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(32);

			arena.Free(p2, 16);
			Expect(arena.GetFreeSize()).ToEqual(48);

			arena.Free(p, 16);
			Expect(arena.GetFreeSize()).ToEqual(64);
		});

		It("Can free in between allocations", []()
		{
			BestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(32);
			new (p) TypeOfSize<32>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(32);

			void* p2 = arena.Alloc(30);
			new (p2) TypeOfSize<30>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(2);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			void* p3 = arena.Alloc(2);
			new (p3) TypeOfSize<2>();
			Expect(p3).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			// No space left, no free slots
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p2, 30);
			Expect(arena.GetFreeSize()).ToEqual(30);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);
			Expect(arena.GetFreeSlots()[0].start).ToEqual(p2);
			Expect(arena.GetFreeSlots()[0].End()).ToEqual(p3);
		});

		It("Can merge previous and next slots on free", []()
		{
			BestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(9);
			new (p) TypeOfSize<9>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(55);

			void* p2 = arena.Alloc(50);
			new (p2) TypeOfSize<50>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(5);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			void* p3 = arena.Alloc(5);
			new (p3) TypeOfSize<5>();
			Expect(p3).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);

			// No space left, no free slots
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p, 9);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p3, 5);
			Expect(arena.GetFreeSlots().Size()).ToEqual(2);

			arena.Free(p2, 50);    // Slots previous and next are merged
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);
			Expect(arena.GetFreeSlots()[0].size).ToEqual(64);

			// Slot contains the entire memory block
			Expect(arena.GetFreeSlots()[0].start).ToEqual(arena.GetBlock().data);
			Expect(arena.GetFreeSlots()[0].End()).ToEqual(arena.GetBlock().End());
		});

		It("Can merge previous slot on free", []()
		{
			BestFitArena arena{48};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(39);
			new (p) TypeOfSize<39>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(9);

			void* p2 = arena.Alloc(9);
			new (p2) TypeOfSize<9>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p, 39);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p2, 9);    // Slot is expanded from the front
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);
			Expect(arena.GetFreeSlots()[0].size).ToEqual(48);

			// Slot contains the entire memory block
			Expect(arena.GetFreeSlots()[0].start).ToEqual(arena.GetBlock().data);
			Expect(arena.GetFreeSlots()[0].End()).ToEqual(arena.GetBlock().End());
		});

		It("Can merge next slot on free", []()
		{
			BestFitArena arena{48};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(24);
			new (p) TypeOfSize<24>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(24);

			void* p2 = arena.Alloc(24);
			new (p2) TypeOfSize<24>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p2, 24);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p, 24);    // Slot is expanded from the back
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);
			Expect(arena.GetFreeSlots()[0].size).ToEqual(48);

			// Slot contains the entire memory block
			Expect(arena.GetFreeSlots()[0].start).ToEqual(arena.GetBlock().data);
			Expect(arena.GetFreeSlots()[0].End()).ToEqual(arena.GetBlock().End());
		});

		It("Ensures a big alignment leaves a gap", []()
		{
			BestFitArena arena{128};
			arena.GetStats()->detectLeaks = false;

			// We ensure first allocation aligns the block (just for the test)
			void* p = arena.Alloc(8);
			new (p) TypeOfSize<8>();
			Expect(arena.GetFreeSize()).ToEqual(120);

			void* p2 = arena.Alloc(8, 64);
			new (p2) TypeOfSize<8>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(112);

			// Alignment is absolute, so the gap between p and p2 is zero
			// when the block base lands on a matching 64B boundary.
			const bool hasGap = p2 > (u8*)p + 8;
			Expect(arena.GetFreeSlots().Size()).ToEqual(hasGap ? 2 : 1);

			// Slot contains the rest if the block
			Expect(arena.GetFreeSlots()[0].start).ToEqual((u8*)p2 + 8);
			Expect(arena.GetFreeSlots()[0].End()).ToEqual(arena.GetBlock().End());

			// Slot contains the alignment gap
			if (hasGap)
			{
				Expect(arena.GetFreeSlots()[1].start).ToEqual((u8*)p + 8);
				Expect(arena.GetFreeSlots()[1].End()).ToEqual(p2);
			}
		});
	});
}
