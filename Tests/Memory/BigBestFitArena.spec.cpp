// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeMemoryArenas.h>


using namespace p;

template<p::u32 Size>
struct TypeOfSize
{
	static constexpr p::u32 size = Size;
	p::u8 data[size]{0};    // Fill data for debugging
};

void RegisterMemoryBigBestFitArenaTests()
{
	Spec("Memory.BigBestFitArena", []()
	{
		It("Reserves a block on construction", []()
		{
			BigBestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;
			Expect(arena.GetFreeSize()).ToEqual(1024);
			Expect(*arena.GetBlock()).ToNotEqual(nullptr);
			Expect(arena.GetBlock().size).ToEqual(1024);
		});

		It("Can allocate", []()
		{
			BigBestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(4);
			new (p) TypeOfSize<4>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.Contains(p)).ToBeTrue();
		});

		It("Allocates at correct addresses", []()
		{
			BigBestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			const auto* blockPtr = static_cast<const p::u8*>(*arena.GetBlock());

			void* p = arena.Alloc(4);
			new (p) TypeOfSize<4>();
			const void* expectedP = blockPtr + p::GetAlignmentPaddingWithHeader(blockPtr, 8, 8);
			Expect(p).ToEqual(expectedP);

			void* p2 = arena.Alloc(4);
			new (p2) TypeOfSize<4>();
			void* expectedP2 =
			    static_cast<p::u8*>(p) + 8 + p::GetAlignmentPaddingWithHeader(p, 8, 8);
			Expect(p2).ToEqual(expectedP2);
		});

		It("Detects there is not enough space", []()
		{
			BigBestFitArena arena{32};
			arena.GetStats()->detectLeaks = false;

			// 16 bytes
			void* p = arena.Alloc(8);
			new (p) TypeOfSize<8>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.Contains(p)).ToBeTrue();

			// Another 16 bytes
			void* p2 = arena.Alloc(4);
			new (p2) TypeOfSize<4>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.Contains(p2)).ToBeTrue();

			// No more space, return null
			void* p3 = arena.Alloc(8);    // 8 bytes
			Expect(p3).ToEqual(nullptr);
		});

		It("Allocates with alignment", []()
		{
			BigBestFitArena arena{1024};
			arena.GetStats()->detectLeaks = false;

			void* b = arena.Alloc(1);
			new (b) TypeOfSize<1>();

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Alloc(4, 8);
			new (p) TypeOfSize<4>();
			Expect(p::GetAlignmentPadding(p, 8)).ToEqual(0);

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Alloc(4, 16);
			new (p2) TypeOfSize<4>();
			Expect(p::GetAlignmentPadding(p2, 16)).ToEqual(0);

			// When padding is 0 (last ptr is aligned)
			void* p3 = arena.Alloc(8, 32);
			new (p3) TypeOfSize<8>();
			Expect(p::GetAlignmentPadding(p3, 32)).ToEqual(0);
		});

		It("Can free", []()
		{
			BigBestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(32);
			new (p) TypeOfSize<32>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(24);

			arena.Free(p, 32);
			Expect(arena.GetFreeSize()).ToEqual(64);
		});

		It("Can free multiple", []()
		{
			BigBestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(40);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(16);

			arena.Free(p2, 16);
			Expect(arena.GetFreeSize()).ToEqual(40);

			arena.Free(p, 16);
			Expect(arena.GetFreeSize()).ToEqual(64);
		});

		It("Can free in between allocations", []()
		{
			BigBestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(40);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(16);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			void* p3 = arena.Alloc(8);
			new (p3) TypeOfSize<8>();
			Expect(p3).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			// No space left, no free slots
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p2, 16);
			Expect(arena.GetFreeSize()).ToEqual(24);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			auto slot     = arena.GetFreeSlots()[0];
			u8* slotStart = (u8*)arena.GetBlock().data + slot.offset;
			Expect(slotStart).ToEqual(static_cast<p::u8*>(p2) - 8);
			Expect(slotStart + slot.size).ToEqual(static_cast<p::u8*>(p3) - 8);
		});

		It("Can merge previous and next slots on free", []()
		{
			BigBestFitArena arena{64};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(40);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(16);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			void* p3 = arena.Alloc(8);
			new (p3) TypeOfSize<8>();
			Expect(p3).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);

			// No space left, no free slots
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p, 16);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p3, 8);
			Expect(arena.GetFreeSlots().Size()).ToEqual(2);

			arena.Free(p2, 16);    // Slots previous and next are merged
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			// Slot contains the entire memory block
			auto slot     = arena.GetFreeSlots()[0];
			u8* slotStart = (u8*)arena.GetBlock().data + slot.offset;
			Expect(slotStart).ToEqual(static_cast<const p::u8*>(arena.GetBlock().data));
			Expect(slotStart + slot.size).ToEqual(static_cast<const p::u8*>(arena.GetBlock().End()));
		});

		It("Can merge previous slot on free", []()
		{
			BigBestFitArena arena{48};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(24);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p, 16);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p2, 16);    // Slot is expanded from the front
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			// Slot contains the entire memory block
			auto slot     = arena.GetFreeSlots()[0];
			u8* slotStart = (u8*)arena.GetBlock().data + slot.offset;
			Expect(slotStart).ToEqual(static_cast<const p::u8*>(arena.GetBlock().data));
			Expect(slotStart + slot.size).ToEqual(static_cast<const p::u8*>(arena.GetBlock().End()));
		});

		It("Can merge next slot on free", []()
		{
			BigBestFitArena arena{48};
			arena.GetStats()->detectLeaks = false;

			void* p = arena.Alloc(16);
			new (p) TypeOfSize<16>();
			Expect(p).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(24);

			void* p2 = arena.Alloc(16);
			new (p2) TypeOfSize<16>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(0);
			Expect(arena.GetFreeSlots().Size()).ToEqual(0);

			arena.Free(p2, 16);
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			arena.Free(p, 16);    // Slot is expanded from the back
			Expect(arena.GetFreeSlots().Size()).ToEqual(1);

			// Slot contains the entire memory block
			auto slot     = arena.GetFreeSlots()[0];
			u8* slotStart = (u8*)arena.GetBlock().data + slot.offset;
			Expect(slotStart).ToEqual(static_cast<const p::u8*>(arena.GetBlock().data));
			Expect(slotStart + slot.size).ToEqual(static_cast<const p::u8*>(arena.GetBlock().End()));
		});

		It("Ensures a big alignment leaves a gap", []()
		{
			BigBestFitArena arena{128};
			arena.GetStats()->detectLeaks = false;

			// We ensure first allocation aligns the block (just for the test)
			void* p = arena.Alloc(8);
			new (p) TypeOfSize<8>();
			Expect(arena.GetFreeSize()).ToEqual(112);

			void* p2 = arena.Alloc(8, 64);
			new (p2) TypeOfSize<8>();
			Expect(p2).ToNotEqual(nullptr);
			Expect(arena.GetFreeSize()).ToEqual(96);

			// Alignment is absolute, so the gap between p and p2 is zero
			// when the block base lands on a matching 64B boundary.
			const bool hasGap = arena.GetAllocationStart(p2) > arena.GetAllocationEnd(p);
			Expect(arena.GetFreeSlots().Size()).ToEqual(hasGap ? 2 : 1);

			// Slot contains the rest if the block
			auto slot0     = arena.GetFreeSlots()[0];
			u8* slot0Start = (u8*)arena.GetBlock().data + slot0.offset;
			Expect(slot0Start).ToEqual(arena.GetAllocationEnd(p2));
			Expect(slot0Start + slot0.size).ToEqual(static_cast<const p::u8*>(arena.GetBlock().End()));

			// Slot contains the alignment gap
			if (hasGap)
			{
				auto slot1     = arena.GetFreeSlots()[1];
				u8* slot1Start = (u8*)arena.GetBlock().data + slot1.offset;
				Expect(slot1Start).ToEqual(arena.GetAllocationEnd(p));
				Expect(slot1Start + slot1.size).ToEqual(arena.GetAllocationStart(p2));
			}
		});
	});
}
