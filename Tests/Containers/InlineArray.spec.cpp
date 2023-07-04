// Copyright 2015-2023 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/PipeContainers.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


go_bandit([]() {
	describe("Containers.InlineArray", []() {
		it("Can initialize", [&]() {
			TInlineArray<i32, 5> data1{};
			TInlineArray<i32, 5> data2(3);
			TInlineArray<i32, 5> data3{3, 3, 3, 3};
			TInlineArray<i32, 5> data4(3, 1);
			TInlineArray<i32, 5> data5(6, 1);

			AssertThat(data1.Size(), Equals(0));
			AssertThat(data1.Capacity(), Equals(0));
			AssertThat(data2.Size(), Equals(3));
			AssertThat(data2.Capacity(), Equals(5));
			AssertThat(data3.Size(), Equals(4));
			AssertThat(data3.Capacity(), Equals(5));
			AssertThat(data4.Size(), Equals(3));
			AssertThat(data4.Capacity(), Equals(5));
			AssertThat(data5.Size(), Equals(6));
			AssertThat(data5.Capacity(), Equals(6));

			AssertThat(data2[0], Equals(0));
			AssertThat(data2[2], Equals(0));
			AssertThat(data3[0], Equals(3));
			AssertThat(data3[3], Equals(3));
			AssertThat(data4[0], Equals(1));
			AssertThat(data4[2], Equals(1));
			AssertThat(data5[0], Equals(1));
			AssertThat(data5[5], Equals(1));
		});


		describe("Copy", []() {
			it("Can copy empty", [&]() {
				TInlineArray<i32, 5> source1{};
				TInlineArray<i32, 5> target1 = source1;    // NOLINT
				AssertThat(target1.Data(), Equals(nullptr));
				AssertThat(target1.Size(), Equals(0));
				AssertThat(target1.Capacity(), Equals(0));

				TInlineArray<i32, 0> source2{};
				TInlineArray<i32, 0> target2 = source2;    // NOLINT
				AssertThat(target2.Data(), Equals(nullptr));
				AssertThat(target2.Size(), Equals(0));
				AssertThat(target2.Capacity(), Equals(0));
			});

			it("Can copy dynamic to dynamic", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6, 7, 8};    // Not inline buffer

				TInlineArray<i32, 5> target = source;
				AssertThat(source.Size(), Equals(6));
				AssertThat(source.Capacity(), Equals(6));
				AssertThat(target.Size(), Equals(6));
				AssertThat(target.Capacity(), Equals(6));

				AssertThat(source[0], Equals(3));
				AssertThat(source[5], Equals(8));
				AssertThat(target[0], Equals(3));
				AssertThat(target[5], Equals(8));

				AssertThat(source.Data(), !Equals(source.GetInlineBuffer()));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
			});

			it("Can copy inline to inline", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6};    // Not inline buffer

				TInlineArray<i32, 5> target;
				target = source;
				AssertThat(source.Size(), Equals(4));
				AssertThat(source.Capacity(), Equals(5));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(5));

				AssertThat(source[0], Equals(3));
				AssertThat(source[3], Equals(6));
				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), Equals(source.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(target.GetInlineBuffer()));

				TInlineArray<i32, 4> target2;    // Copy to a different size
				target2 = source;
				AssertThat(source.Data(), Equals(source.GetInlineBuffer()));
				AssertThat(target2.Data(), Equals(target2.GetInlineBuffer()));
			});

			it("Can copy dynamic to inline", [&]() {
				TInlineArray<i32, 0> source{3, 4, 5, 6};    // Not inline buffer

				TInlineArray<i32, 5> target;
				target = source;
				AssertThat(source.Size(), Equals(4));
				AssertThat(source.Capacity(), Equals(4));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(5));

				AssertThat(source[0], Equals(3));
				AssertThat(source[3], Equals(6));
				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), !Equals(source.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(target.GetInlineBuffer()));
			});

			it("Can copy inline to dynamic", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6};    // Not inline buffer

				TInlineArray<i32, 0> target;
				target = source;
				AssertThat(source.Size(), Equals(4));
				AssertThat(source.Capacity(), Equals(5));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(4));

				AssertThat(source[0], Equals(3));
				AssertThat(source[3], Equals(6));
				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), Equals(source.GetInlineBuffer()));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
			});
		});

		describe("Move", []() {
			it("Can move empty", [&]() {
				TInlineArray<i32, 5> source1{};
				TInlineArray<i32, 5> target1 = Move(source1);
				AssertThat(target1.Data(), Equals(nullptr));
				AssertThat(target1.Size(), Equals(0));
				AssertThat(target1.Capacity(), Equals(0));

				TInlineArray<i32, 0> source2{};
				TInlineArray<i32, 0> target2 = Move(source2);
				AssertThat(target2.Data(), Equals(nullptr));
				AssertThat(target2.Size(), Equals(0));
				AssertThat(target2.Capacity(), Equals(0));
			});

			it("Can move dynamic to dynamic", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6, 7, 8};    // Not inline buffer
				i32* sourceData = source.Data();

				TInlineArray<i32, 5> target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(6));
				AssertThat(target.Capacity(), Equals(6));

				AssertThat(target[0], Equals(3));
				AssertThat(target[5], Equals(8));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(sourceData));
			});

			it("Can move inline to inline", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6};     // Not inline buffer
				TInlineArray<i32, 5> source2{3, 4, 5, 6};    // Not inline buffer

				TInlineArray<i32, 5> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(5));

				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), Equals(target.GetInlineBuffer()));

				TInlineArray<i32, 4> target2;    // Copy to a different size
				target2 = Move(source2);
				AssertThat(source2.Data(), Equals(nullptr));
				AssertThat(target2.Data(), Equals(target2.GetInlineBuffer()));
			});

			it("Can move dynamic to inline", [&]() {
				TInlineArray<i32, 0> source{3, 4, 5, 6};    // Not inline buffer
				i32* sourceData = source.Data();

				TInlineArray<i32, 5> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(4));

				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(sourceData));
			});

			it("Can move inline to dynamic", [&]() {
				TInlineArray<i32, 5> source{3, 4, 5, 6};    // Inline buffer

				TInlineArray<i32, 0> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(4));

				AssertThat(target[0], Equals(3));
				AssertThat(target[3], Equals(6));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
			});
		});
	});
});
