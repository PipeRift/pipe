// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/Arenas/BestFitArena.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift::Memory;


go_bandit([]() {
	describe("Memory", []() {
		describe("Best Fit Arena", []() {
			it("Reserves a block on construction", [&]() {
				BestFitArena arena{1024};

				AssertThat(*arena.GetBlock(), Is().Not().Null());
				AssertThat(arena.GetBlock().GetSize(), Is().EqualTo(1024));
			});

			it("Can allocate", [&]() {
				BestFitArena arena{1024};
				void* p = arena.Allocate(sizeof(float));
				AssertThat(p, Is().Not().Null());
			});

			it("Allocates at correct addresses", [&]() {
				BestFitArena arena{1024};

				void* blockPtr = *arena.GetBlock();
				void* p        = arena.Allocate(sizeof(float));
				AssertThat(p, Is().EqualTo(blockPtr));

				void* p2 = arena.Allocate(sizeof(float));
				AssertThat(p2, Is().EqualTo((Rift::u8*) blockPtr + 4));
			});

			it("Detects there is not enough space", [&]() {
				BestFitArena arena{16};

				void* block = *arena.GetBlock();
				void* p     = arena.Allocate(sizeof(float*));    // 8 bytes
				void* p2    = arena.Allocate(sizeof(float));     // 4 bytes
				AssertThat(arena.GetBlock().GetSize(), Is().EqualTo(16));

				void* p3       = arena.Allocate(sizeof(float*));    // 8 bytes
				void* newBlock = *arena.GetBlock();
				AssertThat(block, Is().Not().EqualTo(newBlock));
				AssertThat(p3, Is().EqualTo(newBlock));
				AssertThat(arena.GetBlock().GetSize(), Is().EqualTo(16));
			});

			it("Allocates with alignment", [&]() {
				BestFitArena arena{1024};

				arena.Allocate(sizeof(bool));

				// When padding is not 0 (last ptr is not aligned)
				void* p = arena.Allocate(sizeof(float), 8);
				AssertThat(Rift::GetAlignmentPadding(p, 8), Is().EqualTo(0));

				// When padding is 0 (last ptr is aligned)
				void* p2 = arena.Allocate(sizeof(float), 16);
				AssertThat(Rift::GetAlignmentPadding(p2, 16), Is().EqualTo(0));
			});
		});
	});
});
