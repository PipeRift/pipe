// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/Arenas/LinearArena.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;


go_bandit([]() {
	describe("Memory.LinearArena", []() {
		it("Reserves a block on construction", [&]() {
			LinearArena arena{1024};

			AssertThat(*arena.GetBlock(), Is().Not().Null());
			AssertThat(arena.GetBlockSize(), Is().EqualTo(1024));
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(0));
		});

		it("Can grow a new block", [&]() {
			LinearArena arena{256};

			AssertThat(arena.GetBlockSize(), Is().EqualTo(256));

			arena.Grow(512);
			AssertThat(arena.GetDiscardedBlocks().Size(), Is().EqualTo(1));
			AssertThat(arena.GetBlockSize(), Is().EqualTo(512));
		});

		it("Can free active block", [&]() {
			LinearArena arena{1024};
			arena.Reset();
			AssertThat(*arena.GetBlock(), Is().Null());
		});

		it("Can allocate", [&]() {
			LinearArena arena{1024};
			void* p = arena.Allocate(sizeof(float));
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(4));
			AssertThat(arena.GetBlockSize(), Is().EqualTo(1024));
		});

		it("Can allocate after reset", [&]() {
			LinearArena arena{1024};
			arena.Reset();
			void* p = arena.Allocate(sizeof(float));
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(4));
			// Buffer size will be as small as the type (4 bytes)
			AssertThat(arena.GetBlockSize(), Is().EqualTo(4));
		});

		it("Has correct allocation size after new block", [&]() {
			LinearArena arena{256};

			void* p = arena.Allocate(sizeof(float));
			AssertThat(p, Is().Not().Null());
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(4));

			arena.Grow(512);
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(0));
		});

		it("Can free full blocks", [&]() {
			LinearArena arena{1024};
			arena.Grow(256);
			AssertThat(arena.GetDiscardedBlocks().Size(), Is().EqualTo(1));
			arena.Reset();
			AssertThat(arena.GetDiscardedBlocks().Size(), Is().EqualTo(0));
		});

		it("Allocates at correct addresses", [&]() {
			LinearArena arena{1024};

			void* blockPtr = *arena.GetBlock();
			void* p        = arena.Allocate(sizeof(float));
			AssertThat(p, Is().EqualTo(blockPtr));

			void* p2 = arena.Allocate(sizeof(float));
			AssertThat(p2, Is().EqualTo((Rift::u8*) blockPtr + 4));
		});

		it("Grows when there's not enough space", [&]() {
			LinearArena arena{16};

			void* block = *arena.GetBlock();
			void* p     = arena.Allocate(sizeof(float*));    // 8 bytes
			void* p2    = arena.Allocate(sizeof(float));     // 4 bytes
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(12));
			AssertThat(arena.GetBlockSize(), Is().EqualTo(16));

			void* p3       = arena.Allocate(sizeof(float*));    // 8 bytes
			void* newBlock = *arena.GetBlock();
			AssertThat(block, Is().Not().EqualTo(newBlock));
			AssertThat(p3, Is().EqualTo(newBlock));
			AssertThat(arena.GetUsedBlockSize(), Is().EqualTo(8));
			AssertThat(arena.GetBlockSize(), Is().EqualTo(16));
		});

		it("Allocates with alignment", [&]() {
			LinearArena arena{1024};

			arena.Allocate(sizeof(bool));

			// When padding is not 0 (last ptr is not aligned)
			void* p = arena.Allocate(sizeof(float), 8);
			AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

			// When padding is 0 (last ptr is aligned)
			void* p2 = arena.Allocate(sizeof(float), 16);
			AssertThat(Rift::GetAlignmentPadding(p2, 16), Is().EqualTo(0));
		});

		it("Grow and allocate with alignment", [&]() {
			LinearArena arena{1024};

			void* firstBlock = *arena.GetBlock();

			arena.Allocate(1019);    // 5 bytes available

			// Enough memory available, but not if we align it.
			// Must grow a new block
			void* p = arena.Allocate(sizeof(float), 8);
			AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

			void* secondBlock = *arena.GetBlock();
			AssertThat(firstBlock, Is().Not().EqualTo(secondBlock));
		});
	});
});
