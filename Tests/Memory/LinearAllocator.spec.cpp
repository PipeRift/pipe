// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/LinearAllocator.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;


go_bandit([]() {
	describe("Memory", []() {
		describe("Linear Allocator", []() {
			it("Reserves a block on construction", [&]() {
				LinearAllocator allocator{1024};

				AssertThat(allocator.GetActiveBlock(), Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(0));
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(1024));
			});

			it("Can grow a new block", [&]() {
				LinearAllocator allocator{256};

				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(256));

				allocator.Grow(512);
				AssertThat(allocator.GetFullBlocks().Size(), Is().EqualTo(1));
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(512));
			});

			it("Can free active block", [&]() {
				LinearAllocator allocator{1024};
				allocator.Reset();
				AssertThat(allocator.GetActiveBlock(), Is().Null());
			});

			it("Can allocate", [&]() {
				LinearAllocator allocator{1024};
				void* p = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(4));
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(1024));
			});

			it("Can allocate after reset", [&]() {
				LinearAllocator allocator{1024};
				allocator.Reset();
				void* p = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(4));
				// Buffer size will be as small as the type (4 bytes)
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(4));
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
				AssertThat(allocator.GetFullBlocks().Size(), Is().EqualTo(1));
				allocator.Reset();
				AssertThat(allocator.GetFullBlocks().Size(), Is().EqualTo(0));
			});

			it("Allocates at correct addresses", [&]() {
				LinearAllocator allocator{1024};

				void* block = allocator.GetActiveBlock();
				void* p     = allocator.Allocate(sizeof(float));
				AssertThat(p, Is().EqualTo(block));

				void* p2 = allocator.Allocate(sizeof(float));
				AssertThat(p2, Is().EqualTo((Rift::u8*) block + 4));
			});

			it("Grows when there's not enough space", [&]() {
				LinearAllocator allocator{16};

				void* block = allocator.GetActiveBlock();
				void* p     = allocator.Allocate(sizeof(float*));    // 8 bytes
				void* p2    = allocator.Allocate(sizeof(float));     // 4 bytes
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(12));
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(16));

				void* p3       = allocator.Allocate(sizeof(float*));    // 8 bytes
				void* newBlock = allocator.GetActiveBlock();
				AssertThat(block, Is().Not().EqualTo(newBlock));
				AssertThat(p3, Is().EqualTo(newBlock));
				AssertThat(allocator.GetUsedBlockSize(), Is().EqualTo(8));
				AssertThat(allocator.GetMaxBlockSize(), Is().EqualTo(16));
			});
		});
	});
});
