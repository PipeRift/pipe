// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/LinearAllocator.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;


go_bandit([]() {
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
	});
});
