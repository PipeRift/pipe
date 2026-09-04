// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeMemoryArenas.h>
#include <PipeTest.h>


using namespace p;


Spec("Memory.MonoLinearArena", []()
{
	It("Reserves a block on construction", []()
	{
		MonoLinearArena arena{1024};

		Expect(arena.GetAvailableMemory()).ToEqual(1024);
		arena.GetStats()->CollectStats();
		Expect(arena.GetStats()->used).ToEqual(0);
	});

	It("Can allocate outside the block", []()
	{
		MonoLinearArena arena{256};

		Expect(arena.GetAvailableMemory()).ToEqual(256);
		void* p = arena.Alloc(512);
		Expect(arena.GetAvailableMemory()).ToEqual(256);
		arena.Free(p, 512);
	});

	It("Can free from outside the block", []()
	{
		MonoLinearArena arena{256};

		void* p = arena.Alloc(512);
		Expect(arena.GetAvailableMemory()).ToEqual(256);
		arena.Free(p, 512);
		Expect(arena.GetAvailableMemory()).ToEqual(256);
	});

	It("Can free active block", []()
	{
		MonoLinearArena arena{1024};
		arena.Release();

		TArray<ArenaBlock> blocks;
		arena.GetBlocks(blocks);
		Expect(blocks.Size()).ToEqual(1);
	});

	It("Can allocate", []()
	{
		MonoLinearArena arena{1024};
		void* p = arena.Alloc(sizeof(float));
		Expect(p).ToNotEqual(nullptr);
		arena.GetStats()->CollectStats();
		Expect(arena.GetStats()->used).ToEqual(4);
		Expect(arena.GetAvailableMemory()).ToEqual(1024);
		arena.Free(p, sizeof(float));
	});

	It("Can allocate with alignment", []()
	{
		MonoLinearArena arena{1024};

		void* p0 = arena.Alloc(sizeof(bool));

		// When padding is not 0 (last ptr is not aligned)
		void* p1 = arena.Alloc(sizeof(float), 8);
		Expect(p::GetAlignmentPadding(p1, 8)).ToEqual(0);

		// When padding is 0 (last ptr is aligned)
		void* p2 = arena.Alloc(sizeof(float), 16);
		Expect(p::GetAlignmentPadding(p2, 16)).ToEqual(0);

		arena.Free(p0, sizeof(bool));
		arena.Free(p1, sizeof(float));
		arena.Free(p2, sizeof(float));
	});

	It("Can allocate after release", []()
	{
		MonoLinearArena arena{1024};
		arena.Release();
		void* p = arena.Alloc(sizeof(float));
		Expect(p).ToNotEqual(nullptr);
		arena.GetStats()->CollectStats();
		Expect(arena.GetStats()->used).ToEqual(4);
		// Buffer size will be as small as the type (4 bytes)
		Expect(arena.GetAvailableMemory()).ToEqual(1024);

		arena.Free(p, sizeof(float));
	});

	It("Can free block after Free", []()
	{
		MonoLinearArena arena{1024};
		void* p = arena.Alloc(256);
		arena.GetStats()->CollectStats();
		Expect(arena.GetStats()->used).ToEqual(256);
		arena.Free(p, 256);
		arena.GetStats()->CollectStats();
		Expect(arena.GetStats()->used).ToEqual(0);
	});

	It("Allocates at correct addresses", []()
	{
		MonoLinearArena arena{1024};

		TArray<ArenaBlock> blocks;
		arena.GetBlocks(blocks);

		void* p1 = arena.Alloc(sizeof(float));
		Expect(p1).ToEqual(blocks[0].data);
		void* p2 = arena.Alloc(sizeof(float), alignof(float));
		Expect(p2).ToEqual((u8*)blocks[0].data + 4);

		arena.Free(p1, sizeof(float));
		arena.Free(p2, sizeof(float));
	});

	// Move test to Multi linear
	/*It("Allocated new blocks when previous is filled", []() {
	    MonoLinearArena arena{16};

	    void* p = arena.Alloc(sizeof(float*));    // 8 bytes
	    arena.Alloc(sizeof(float));               // 4 bytes
	    Expect(arena.GetStats()->used).ToEqual(12);
	    Expect(arena.GetAvailableMemory()).ToEqual(16);

	    void* p3 = arena.Alloc(sizeof(float*));    // 8 bytes
	    TArray<ArenaBlock> blocks;
	    arena.GetBlocks(blocks);
	    Expect(blocks.Size()).ToEqual(2);
	    Expect(blocks[0]).ToNotEqual(blocks[1]);
	    Expect(p).ToEqual(blocks[0].data);
	    Expect(p3).ToEqual(blocks[1].data);

	    Expect(arena.GetStats()->used).ToEqual(8);
	    Expect(arena.GetAvailableMemory()).ToEqual(16);
	});*/
});
