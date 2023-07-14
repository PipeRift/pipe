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

struct MoveType
{
	i32 value = 0;

	MoveType(i32 value) : value(value) {}
	MoveType(MoveType&& other)
	{
		value = Exchange(other.value, 0);
	}
	MoveType& operator=(MoveType&& other)
	{
		value = Exchange(other.value, 0);
		return *this;
	}
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
				AssertThat(target.Capacity(), IsGreaterThanOrEqualTo(6));
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
				AssertThat(target.Capacity(), IsGreaterThanOrEqualTo(4));
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
				TInlineArray<MoveType, 5> source1{};
				TInlineArray<MoveType, 5> target1 = Move(source1);
				AssertThat(target1.Data(), Equals(nullptr));
				AssertThat(target1.Size(), Equals(0));
				AssertThat(target1.Capacity(), Equals(0));
				TInlineArray<MoveType, 0> source2{};
				TInlineArray<MoveType, 0> target2 = Move(source2);
				AssertThat(target2.Data(), Equals(nullptr));
				AssertThat(target2.Size(), Equals(0));
				AssertThat(target2.Capacity(), Equals(0));
			});

			it("Can move dynamic to dynamic", [&]() {
				TInlineArray<MoveType, 5> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				source.Add(7);
				source.Add(8);
				MoveType* sourceData             = source.Data();
				TInlineArray<MoveType, 5> target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(6));
				AssertThat(target.Capacity(), IsGreaterThanOrEqualTo(6));
				AssertThat(target[0].value, Equals(3));
				AssertThat(target[5].value, Equals(8));
				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(sourceData));
			});

			it("Can move inline to inline", [&]() {
				TInlineArray<MoveType, 5> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				TInlineArray<MoveType, 5> source2{};    // Not inline buffer
				source2.Add(3);
				source2.Add(4);
				source2.Add(5);
				source2.Add(6);
				TInlineArray<MoveType, 5> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(5));
				AssertThat(target[0].value, Equals(3));
				AssertThat(target[3].value, Equals(6));
				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), Equals(target.GetInlineBuffer()));
				TInlineArray<MoveType, 4> target2;    // Copy to a different size
				target2 = Move(source2);
				AssertThat(source2.Data(), Equals(nullptr));
				AssertThat(target2.Data(), Equals(target2.GetInlineBuffer()));
			});

			it("Can move dynamic to inline", [&]() {
				TInlineArray<MoveType, 0> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				MoveType* sourceData = source.Data();
				TInlineArray<MoveType, 5> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), Equals(4));

				AssertThat(target[0].value, Equals(3));
				AssertThat(target[3].value, Equals(6));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
				AssertThat(target.Data(), Equals(sourceData));
			});

			it("Can move inline to dynamic", [&]() {
				TInlineArray<MoveType, 5> source{};    // Inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				TInlineArray<MoveType, 0> target;
				target = Move(source);
				AssertThat(source.Size(), Equals(0));
				AssertThat(source.Capacity(), Equals(0));
				AssertThat(target.Size(), Equals(4));
				AssertThat(target.Capacity(), IsGreaterThanOrEqualTo(4));

				AssertThat(target[0].value, Equals(3));
				AssertThat(target[3].value, Equals(6));

				AssertThat(source.Data(), Equals(nullptr));
				AssertThat(target.Data(), !Equals(target.GetInlineBuffer()));
			});
		});

		describe("Add", []() {
			it("Can add to dynamic", [&]() {
				TInlineArray<i32, 0> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Add(3);
				AssertThat(data.Size(), Equals(1));
				AssertThat(data[0], Equals(3));
				data.Add(4);
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[1], Equals(4));
			});

			it("Can add to inline", [&]() {
				TInlineArray<i32, 2> data;
				data.Add(3);
				AssertThat(data.Size(), Equals(1));
				AssertThat(data[0], Equals(3));
				data.Add(4);
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[1], Equals(4));
			});

			it("Can add to correct buffers", [&]() {
				TInlineArray<i32, 2> data;
				data.Add(3);
				data.Add(4);
				AssertThat(data.Data(), Equals(data.GetInlineBuffer()));
				data.Add(5);    // Grow here to dynamic buffer
				AssertThat(data.Size(), Equals(3));
				AssertThat(data[0], Equals(3));
				AssertThat(data[1], Equals(4));
				AssertThat(data[2], Equals(5));
				AssertThat(data.Data(), !Equals(data.GetInlineBuffer()));
			});

			it("Can add by move", [&]() {
				TInlineArray<MoveType, 0> data;
				MoveType tmp{2};
				data.Add(Move(tmp));
				data.Add(MoveType{3});
				AssertThat(data[0].value, Equals(2));
				AssertThat(data[1].value, Equals(3));
				AssertThat(tmp.value, Equals(0));
			});

			it("Can add by copy", [&]() {
				TInlineArray<i32, 0> data;
				i32 tmp = 2;
				data.Add(tmp);
				data.Add(3);
				AssertThat(tmp, Equals(2));
				AssertThat(data[0], Equals(2));
				AssertThat(data[1], Equals(3));
			});

			it("Can add defaulted", [&]() {
				TInlineArray<i32, 0> data;
				data.Add();
				AssertThat(data[0], Equals(0));
			});
		});
		describe("Append", []() {
			it("Can append defaulted", [&]() {
				TInlineArray<i32, 0> data;
				data.Append(2);
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0], Equals(0));
				AssertThat(data[1], Equals(0));
				data.Append(0);
				AssertThat(data.Size(), Equals(2));
				data.Append(2);
				AssertThat(data.Size(), Equals(4));
				AssertThat(data[2], Equals(0));
				AssertThat(data[3], Equals(0));
			});

			it("Can append value", [&]() {
				TInlineArray<i32, 0> data;
				data.Append(2, 234);
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0], Equals(234));
				AssertThat(data[1], Equals(234));
				data.Append(0, 234);
				AssertThat(data.Size(), Equals(2));
				data.Append(2, 234);
				AssertThat(data.Size(), Equals(4));
				AssertThat(data[2], Equals(234));
				AssertThat(data[3], Equals(234));
			});

			it("Can append to dynamic", [&]() {
				TInlineArray<i32, 0> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Append(2, 33);
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0], Equals(33));
				AssertThat(data[1], Equals(33));
			});
		});
		describe("Assign", []() {
			xit("Can assign", []() {});
		});
		describe("Insert", []() {
			it("Can insert copied value", [&]() {
				TInlineArray<i32, 0> data;
				data.Insert(0, 32);    // Insert at empty
				AssertThat(data.Size(), Equals(1));
				AssertThat(data[0], Equals(32));

				data.Insert(0, 65);    // Insert at start
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0], Equals(65));
				AssertThat(data[1], Equals(32));

				data.Add(85);
				data.Insert(1, 27);    // Insert in the middle
				AssertThat(data.Size(), Equals(4));
				AssertThat(data[1], Equals(27));

				data.Insert(4, 43);    // Insert in the end
				AssertThat(data.Size(), Equals(5));
				AssertThat(data[4], Equals(43));
			});

			it("Can insert many values", [&]() {
				TInlineArray<i32, 0> data;
				data.Insert(0, 2, 32);    // Insert at empty
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0], Equals(32));
				AssertThat(data[1], Equals(32));

				data.Insert(0, 2, 5);    // Insert at start
				AssertThat(data.Size(), Equals(4));
				AssertThat(data[0], Equals(5));
				AssertThat(data[1], Equals(5));
				AssertThat(data[2], Equals(32));
				AssertThat(data[3], Equals(32));

				data.Insert(3, 2, 6);    // Insert in the middle
				AssertThat(data.Size(), Equals(6));
				AssertThat(data[3], Equals(6));
				AssertThat(data[4], Equals(6));

				data.Insert(6, 2, 9);    // Insert in the end
				AssertThat(data.Size(), Equals(8));
				AssertThat(data[6], Equals(9));
				AssertThat(data[7], Equals(9));
			});

			it("Can insert moved value", [&]() {
				TInlineArray<MoveType, 0> data;
				MoveType tmp{34};
				data.Insert(0, Move(tmp));    // Insert at empty
				AssertThat(data.Size(), Equals(1));
				AssertThat(data[0].value, Equals(34));
				AssertThat(tmp.value, Equals(0));

				MoveType tmp2{4};
				data.Insert(0, Move(tmp2));    // Insert at start
				AssertThat(data.Size(), Equals(2));
				AssertThat(data[0].value, Equals(4));
				AssertThat(data[1].value, Equals(34));
				AssertThat(tmp2.value, Equals(0));

				MoveType tmp3{3};
				data.Add(MoveType{85});
				data.Insert(1, Move(tmp3));    // Insert in the middle
				AssertThat(data.Size(), Equals(4));
				AssertThat(data[1].value, Equals(3));
				AssertThat(tmp3.value, Equals(0));

				MoveType tmp4{7};
				data.Insert(4, Move(tmp4));    // Insert in the end
				AssertThat(data.Size(), Equals(5));
				AssertThat(data[4].value, Equals(7));
				AssertThat(tmp4.value, Equals(0));
			});

			it("Can insert buffer", [&]() {
				TInlineArray<i32, 0> data;
				i32 src[]{34, 23, 844};
				data.Insert(0, src, 3);    // Insert at empty
				AssertThat(data.Size(), Equals(3));
				AssertThat(data[0], Equals(34));
				AssertThat(data[1], Equals(23));
				AssertThat(data[2], Equals(844));

				i32 src2[]{2, 71, 21};
				data.Insert(0, src2, 3);    // Insert at start
				AssertThat(data.Size(), Equals(6));
				AssertThat(data[0], Equals(2));
				AssertThat(data[1], Equals(71));
				AssertThat(data[2], Equals(21));
				AssertThat(data[3], Equals(34));
				AssertThat(data[4], Equals(23));
				AssertThat(data[5], Equals(844));

				i32 src3[]{4, 3, 6};
				data.Insert(3, src3, 3);    // Insert in the middle
				AssertThat(data.Size(), Equals(9));
				AssertThat(data[3], Equals(4));
				AssertThat(data[4], Equals(3));
				AssertThat(data[5], Equals(6));

				i32 src4[]{7, 2, 3};
				data.Insert(9, src4, 3);    // Insert in the end
				AssertThat(data.Size(), Equals(12));
				AssertThat(data[9], Equals(7));
				AssertThat(data[10], Equals(2));
				AssertThat(data[11], Equals(3));
			});
		});
		describe("Remove", []() {
			xit("Can remove", []() {});
		});
	});
});
