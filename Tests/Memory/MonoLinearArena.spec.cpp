// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Memory/Memory.h>
#include <Pipe/Memory/MonoLinearArena.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Memory.MonoLinearArena", []() {
		it("Reserves a block on construction", [&]() {
			MonoLinearArena arena{1024};

			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(1024));
			AssertThat(arena.GetUsedMemory(), Is().EqualTo(0));
		});

		it("Can allocate outside the block", [&]() {
			MonoLinearArena arena{256};

			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(256));
			void* p = arena.Alloc(512);
			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(256));
			arena.Free(p, 512);
		});

		it("Can free from outside the block", [&]() {
			MonoLinearArena arena{256};

			void* p = arena.Alloc(512);
			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(256));
			arena.Free(p, 512);
			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(256));
		});

		it("Can free active block", [&]() {
			MonoLinearArena arena{1024};
			arena.Release();

			TArray<Memory::Block> blocks;
			arena.GetBlocks(blocks);
			AssertThat(blocks.Size(), Equals(1));
		});

		it("Can allocate", [&]() {
			MonoLinearArena arena{1024};
			void* p = arena.Alloc(sizeof(float));
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetUsedMemory(), Is().EqualTo(4));
			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(1024));
		});

		it("Can allocate with alignment", [&]() {
			MonoLinearArena arena{1024};

			arena.Alloc(sizeof(bool));

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Alloc(sizeof(float), 8);
			AssertThat(p::GetAlignmentPadding(p, 8), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Alloc(sizeof(float), 16);
			AssertThat(p::GetAlignmentPadding(p2, 16), Is().EqualTo(0));
		});

		it("Can allocate after release", [&]() {
			MonoLinearArena arena{1024};
			arena.Release();
			void* p = arena.Alloc(sizeof(float));
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetUsedMemory(), Is().EqualTo(4));
			// Buffer size will be as small as the type (4 bytes)
			AssertThat(arena.GetAvailableMemory(), Is().EqualTo(1024));
		});

		it("Can free block after Free", [&]() {
			MonoLinearArena arena{1024};
			void* p = arena.Alloc(256);
			AssertThat(arena.GetUsedMemory(), Is().EqualTo(256));
			arena.Free(p, 256);
			AssertThat(arena.GetUsedMemory(), Is().EqualTo(0));
		});

		it("Allocates at correct addresses", [&]() {
			MonoLinearArena arena{1024};

			TArray<Memory::Block> blocks;
			arena.GetBlocks(blocks);

			void* p = arena.Alloc(sizeof(float));
			AssertThat(p, Is().EqualTo(blocks[0].data));
			void* p2 = arena.Alloc(sizeof(float), alignof(float));
			AssertThat(p2, Is().EqualTo((u8*)blocks[0].data + 4));
		});

		// Move test to Multi linear
		/*it("Allocated new blocks when previous is filled", [&]() {
		    MonoLinearArena arena{16};

		    void* p = arena.Alloc(sizeof(float*));    // 8 bytes
		    arena.Alloc(sizeof(float));               // 4 bytes
		    AssertThat(arena.GetUsedMemory(), Is().EqualTo(12));
		    AssertThat(arena.GetAvailableMemory(), Is().EqualTo(16));

		    void* p3 = arena.Alloc(sizeof(float*));    // 8 bytes
		    TArray<Memory::Block> blocks;
		    arena.GetBlocks(blocks);
		    AssertThat(blocks.Size(), Equals(2));
		    AssertThat(blocks[0], Is().Not().EqualTo(blocks[1]));
		    AssertThat(p, Is().EqualTo(blocks[0].data));
		    AssertThat(p3, Is().EqualTo(blocks[1].data));

		    AssertThat(arena.GetUsedMemory(), Is().EqualTo(8));
		    AssertThat(arena.GetAvailableMemory(), Is().EqualTo(16));
		});*/
	});
});
