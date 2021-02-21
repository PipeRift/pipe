// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/Allocators/LinearAllocator.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;


go_bandit([]() {
	describe("Memory", []() {
		describe("Linear Allocator", []() {
			it("Reserves a block on construction", [&]() {
				LinearAllocator allocator{1024};

				AssertThat(*allocator.GetBlock(), Is().Not().Null());
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(1024));
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(0));
			});

			it("Can grow a new block", [&]() {
				LinearAllocator allocator{256};

				AssertThat(allocator.GetBlockSize(), Is().EqualTo(256));

				allocator.Grow(512);
				AssertThat(allocator.GetDiscardedBlocks().Size(), Is().EqualTo(1));
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(512));
			});

			it("Can free active block", [&]() {
				LinearAllocator allocator{1024};
				allocator.Reset();
				AssertThat(*allocator.GetBlock(), Is().Null());
			});

			it("Can allocate", [&]() {
				LinearAllocator allocator{1024};
				void* p = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(4));
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(1024));
			});

			it("Can allocate after reset", [&]() {
				LinearAllocator allocator{1024};
				allocator.Reset();
				void* p = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(4));
				// Buffer size will be as small as the type (4 bytes)
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(4));
			});

			it("Has correct allocation size after new block", [&]() {
				LinearAllocator allocator{256};

				void* p = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(4));

				allocator.Grow(512);
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(0));
			});

			it("Can free full blocks", [&]() {
				LinearAllocator allocator{1024};
				allocator.Grow(256);
				AssertThat(allocator.GetDiscardedBlocks().Size(), Is().EqualTo(1));
				allocator.Reset();
				AssertThat(allocator.GetDiscardedBlocks().Size(), Is().EqualTo(0));
			});

			it("Allocates at correct addresses", [&]() {
				LinearAllocator allocator{1024};

				void* blockPtr = *allocator.GetBlock();
				void* p        = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().EqualTo(blockPtr));

				void* p2 = allocator.Allocate(sizeof(float));
				AssertThat(p2, Is().EqualTo((Rift::u8*) blockPtr + 4));
			});

			it("Grows when there's not enough space", [&]() {
				LinearAllocator allocator{16};

				void* block = *allocator.GetBlock();
				void* p     = allocator.Allocate(sizeof(float*));    // 8 bytes
				void* p2    = allocator.Allocate(sizeof(float));     // 4 bytes
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(12));
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(16));

				void* p3       = allocator.Allocate(sizeof(float*));    // 8 bytes
				void* newBlock = *allocator.GetBlock();
				AssertThat(block, Is().Not().EqualTo(newBlock));
				AssertThat(p3, Is().EqualTo(newBlock));
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(8));
				AssertThat(allocator.GetBlockSize(), Is().EqualTo(16));
			});

			it("Allocates with alignment", [&]() {
				LinearAllocator allocator{1024};

				allocator.Allocate(sizeof(bool));

				// When padding is not 0 (last ptr is not aligned)
				void* p = allocator.Allocate(sizeof(float), 8);
				AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

				// When padding is 0 (last ptr is aligned)
				void* p2 = allocator.Allocate(sizeof(float), 16);
				AssertThat(Rift::GetAlignmentPadding(p2, 16), Is().EqualTo(0));
			});

			it("Grow and allocate with alignment", [&]() {
				LinearAllocator allocator{1024};

				void* firstBlock = *allocator.GetBlock();

				allocator.Allocate(1019);    // 5 bytes available

				// Enough memory available, but not if we align it.
				// Must grow a new block
				void* p = allocator.Allocate(sizeof(float), 8);
				AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

				void* secondBlock = *allocator.GetBlock();
				AssertThat(firstBlock, Is().Not().EqualTo(secondBlock));
			});
		});
	});
});
