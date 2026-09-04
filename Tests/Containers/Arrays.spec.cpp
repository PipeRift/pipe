// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeContainers.h>


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
	MoveType(MoveType&& other) noexcept : value{Exchange(other.value, 0)} {}
	MoveType& operator=(MoveType&& other) noexcept
	{
		value = Exchange(other.value, 0);
		return *this;
	}
};

struct CopyType
{
	i32 value = 0;

	CopyType() = default;
	CopyType(i32 value) : value(value) {}
	CopyType(const CopyType& other) : value{other.value} {}
	CopyType& operator=(const CopyType& other)
	{
		value = other.value;
		return *this;
	}
};


void RegisterContainersArraysTests()
{
	Spec("Containers.Array", []()
	{
		It("Can initialize", []()
		{
			TArray<i32, 5> data1{};
			TArray<i32, 5> data2(3);
			TArray<i32, 5> data3{3, 3, 3, 3};
			TArray<i32, 5> data4(3, 1);
			TArray<i32, 5> data5(6, 1);

			Expect(data1.Size()).ToEqual(0);
			Expect(data1.Capacity()).ToEqual(0);
			Expect(data2.Size()).ToEqual(3);
			Expect(data2.Capacity()).ToEqual(5);
			Expect(data3.Size()).ToEqual(4);
			Expect(data3.Capacity()).ToEqual(5);
			Expect(data4.Size()).ToEqual(3);
			Expect(data4.Capacity()).ToEqual(5);
			Expect(data5.Size()).ToEqual(6);
			Expect(data5.Capacity()).ToEqual(6);

			Expect(data2[0]).ToEqual(0);
			Expect(data2[2]).ToEqual(0);
			Expect(data3[0]).ToEqual(3);
			Expect(data3[3]).ToEqual(3);
			Expect(data4[0]).ToEqual(1);
			Expect(data4[2]).ToEqual(1);
			Expect(data5[0]).ToEqual(1);
			Expect(data5[5]).ToEqual(1);
		});


		Describe("Copy", []()
		{
			It("Can copy empty", []()
			{
				TArray<i32, 5> source1{};
				TArray<i32, 5> target1 = source1;    // NOLINT
				Expect(target1.Data()).ToEqual(nullptr);
				Expect(target1.Size()).ToEqual(0);
				Expect(target1.Capacity()).ToEqual(0);
				TArray<i32, 0> source2{};
				TArray<i32, 0> target2 = source2;    // NOLINT
				Expect(target2.Data()).ToEqual(nullptr);
				Expect(target2.Size()).ToEqual(0);
				Expect(target2.Capacity()).ToEqual(0);
			});

			It("Can copy dynamic to dynamic", []()
			{
				TArray<i32, 5> source{3, 4, 5, 6, 7, 8};    // Not inline buffer
				TArray<i32, 5> target = source;
				Expect(source.Size()).ToEqual(6);
				Expect(source.Capacity()).ToEqual(6);
				Expect(target.Size()).ToEqual(6);
				Expect(target.Capacity()).ToBeGreaterOrEqual(6);
				Expect(source[0]).ToEqual(3);
				Expect(source[5]).ToEqual(8);
				Expect(target[0]).ToEqual(3);
				Expect(target[5]).ToEqual(8);
				Expect(source.Data()).ToNotEqual(source.GetInlineBuffer());
				Expect(target.Data()).ToNotEqual(target.GetInlineBuffer());
			});

			It("Can copy inline to inline", []()
			{
				TArray<i32, 5> source{3, 4, 5, 6};    // Not inline buffer
				TArray<i32, 5> target;
				target = source;
				Expect(source.Size()).ToEqual(4);
				Expect(source.Capacity()).ToEqual(5);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToEqual(5);
				Expect(source[0]).ToEqual(3);
				Expect(source[3]).ToEqual(6);
				Expect(target[0]).ToEqual(3);
				Expect(target[3]).ToEqual(6);
				Expect(source.Data()).ToEqual(source.GetInlineBuffer());
				Expect(target.Data()).ToEqual(target.GetInlineBuffer());
				TArray<i32, 4> target2;    // Copy to a different size
				target2 = source;
				Expect(source.Data()).ToEqual(source.GetInlineBuffer());
				Expect(target2.Data()).ToEqual(target2.GetInlineBuffer());
			});

			It("Can copy dynamic to inline", []()
			{
				TArray<i32, 0> source{3, 4, 5, 6};    // Not inline buffer
				TArray<i32, 5> target;
				target = source;
				Expect(source.Size()).ToEqual(4);
				Expect(source.Capacity()).ToEqual(4);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToEqual(5);
				Expect(source[0]).ToEqual(3);
				Expect(source[3]).ToEqual(6);
				Expect(target[0]).ToEqual(3);
				Expect(target[3]).ToEqual(6);
				Expect(source.Data()).ToNotEqual(source.GetInlineBuffer());
				Expect(target.Data()).ToEqual(target.GetInlineBuffer());
			});

			It("Can copy inline to dynamic", []()
			{
				TArray<i32, 5> source{3, 4, 5, 6};    // Not inline buffer
				TArray<i32, 0> target;
				target = source;
				Expect(source.Size()).ToEqual(4);
				Expect(source.Capacity()).ToEqual(5);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToBeGreaterOrEqual(4);
				Expect(source[0]).ToEqual(3);
				Expect(source[3]).ToEqual(6);
				Expect(target[0]).ToEqual(3);
				Expect(target[3]).ToEqual(6);
				Expect(source.Data()).ToEqual(source.GetInlineBuffer());
				Expect(target.Data()).ToNotEqual(target.GetInlineBuffer());
			});
		});

		Describe("Move", []()
		{
			It("Can move empty", []()
			{
				TArray<MoveType, 5> source1{};
				TArray<MoveType, 5> target1 = Move(source1);
				Expect(target1.Data()).ToEqual(nullptr);
				Expect(target1.Size()).ToEqual(0);
				Expect(target1.Capacity()).ToEqual(0);
				TArray<MoveType, 0> source2{};
				TArray<MoveType, 0> target2 = Move(source2);
				Expect(target2.Data()).ToEqual(nullptr);
				Expect(target2.Size()).ToEqual(0);
				Expect(target2.Capacity()).ToEqual(0);
			});

			It("Can move dynamic to dynamic", []()
			{
				TArray<MoveType, 5> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				source.Add(7);
				source.Add(8);
				MoveType* sourceData       = source.Data();
				TArray<MoveType, 5> target = Move(source);
				Expect(source.Size()).ToEqual(0);
				Expect(source.Capacity()).ToEqual(0);
				Expect(target.Size()).ToEqual(6);
				Expect(target.Capacity()).ToBeGreaterOrEqual(6);
				Expect(target[0].value).ToEqual(3);
				Expect(target[5].value).ToEqual(8);
				Expect(source.Data()).ToEqual(nullptr);
				Expect(target.Data()).ToNotEqual(target.GetInlineBuffer());
				Expect(target.Data()).ToEqual(sourceData);
			});

			It("Can move inline to inline", []()
			{
				TArray<MoveType, 5> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				TArray<MoveType, 5> source2{};    // Not inline buffer
				source2.Add(3);
				source2.Add(4);
				source2.Add(5);
				source2.Add(6);
				TArray<MoveType, 5> target;
				target = Move(source);
				Expect(source.Size()).ToEqual(0);
				Expect(source.Capacity()).ToEqual(0);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToEqual(5);
				Expect(target[0].value).ToEqual(3);
				Expect(target[3].value).ToEqual(6);
				Expect(source.Data()).ToEqual(nullptr);
				Expect(target.Data()).ToEqual(target.GetInlineBuffer());
				TArray<MoveType, 4> target2;    // Copy to a different size
				target2 = Move(source2);
				Expect(source2.Data()).ToEqual(nullptr);
				Expect(target2.Data()).ToEqual(target2.GetInlineBuffer());
			});

			It("Can move dynamic to inline", []()
			{
				TArray<MoveType, 0> source{};    // Not inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				MoveType* sourceData = source.Data();
				TArray<MoveType, 5> target;
				target = Move(source);
				Expect(source.Size()).ToEqual(0);
				Expect(source.Capacity()).ToEqual(0);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToEqual(4);

				Expect(target[0].value).ToEqual(3);
				Expect(target[3].value).ToEqual(6);

				Expect(source.Data()).ToEqual(nullptr);
				Expect(target.Data()).ToNotEqual(target.GetInlineBuffer());
				Expect(target.Data()).ToEqual(sourceData);
			});

			It("Can move inline to dynamic", []()
			{
				TArray<MoveType, 5> source{};    // Inline buffer
				source.Add(3);
				source.Add(4);
				source.Add(5);
				source.Add(6);
				TArray<MoveType, 0> target;
				target = Move(source);
				Expect(source.Size()).ToEqual(0);
				Expect(source.Capacity()).ToEqual(0);
				Expect(target.Size()).ToEqual(4);
				Expect(target.Capacity()).ToBeGreaterOrEqual(4);

				Expect(target[0].value).ToEqual(3);
				Expect(target[3].value).ToEqual(6);

				Expect(source.Data()).ToEqual(nullptr);
				Expect(target.Data()).ToNotEqual(target.GetInlineBuffer());
			});
		});

		It("Can access data", []()
		{
			TArray<i32, 0> data1;
			TArray<i32, 0> data2{1};
			TArray<i32, 4> data3{1};

			Expect(data1.Data()).ToEqual(nullptr);
			Expect(data2.Data()).ToNotEqual(nullptr);
			Expect(data3.Data()).ToNotEqual(nullptr);
		});

		Describe("Add", []()
		{
			It("Can add to dynamic", []()
			{
				TArray<i32, 0> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Add(3);
				Expect(data.Size()).ToEqual(1);
				Expect(data[0]).ToEqual(3);
				data.Add(4);
				Expect(data.Size()).ToEqual(2);
				Expect(data[1]).ToEqual(4);
			});

			It("Can add to inline", []()
			{
				TArray<i32, 2> data;
				data.Add(3);
				Expect(data.Size()).ToEqual(1);
				Expect(data[0]).ToEqual(3);
				data.Add(4);
				Expect(data.Size()).ToEqual(2);
				Expect(data[1]).ToEqual(4);
			});

			It("Can add to correct buffers", []()
			{
				TArray<i32, 2> data;
				data.Add(3);
				data.Add(4);
				Expect(data.Data()).ToEqual(data.GetInlineBuffer());
				data.Add(5);    // Grow here to dynamic buffer
				Expect(data.Size()).ToEqual(3);
				Expect(data[0]).ToEqual(3);
				Expect(data[1]).ToEqual(4);
				Expect(data[2]).ToEqual(5);
				Expect(data.Data()).ToNotEqual(data.GetInlineBuffer());
			});

			It("Can add value by move", []()
			{
				TArray<MoveType, 0> data;
				MoveType tmp{2};
				data.Add(Move(tmp));
				data.Add(MoveType{3});
				Expect(data[0].value).ToEqual(2);
				Expect(data[1].value).ToEqual(3);
				Expect(tmp.value).ToEqual(0);
			});

			It("Can add value by copy", []()
			{
				TArray<i32, 0> data;
				i32 tmp = 2;
				data.Add(tmp);
				data.Add(3);
				Expect(tmp).ToEqual(2);
				Expect(data[0]).ToEqual(2);
				Expect(data[1]).ToEqual(3);
			});

			It("Can add defaulted", []()
			{
				TArray<i32, 0> data;
				data.Add();
				Expect(data[0]).ToEqual(0);
			});
		});

		Describe("Append", []()
		{
			It("Can append defaulted", []()
			{
				TArray<i32, 0> data;
				data.Append(2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(0);
				Expect(data[1]).ToEqual(0);
				data.Append(0);
				Expect(data.Size()).ToEqual(2);
				data.Append(2);
				Expect(data.Size()).ToEqual(4);
				Expect(data[2]).ToEqual(0);
				Expect(data[3]).ToEqual(0);
			});

			It("Can append value", []()
			{
				TArray<i32, 0> data;
				data.Append(2, 234);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(234);
				Expect(data[1]).ToEqual(234);
				data.Append(0, 234);
				Expect(data.Size()).ToEqual(2);
				data.Append(2, 235);
				Expect(data.Size()).ToEqual(4);
				Expect(data[2]).ToEqual(235);
				Expect(data[3]).ToEqual(235);
			});

			It("Can assign multiple values", []()
			{
				TArray<i32, 0> data;
				i32 buffer[]{24, 53};
				i32 buffer2[]{74, 51};
				data.Append(buffer, 2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(24);
				Expect(data[1]).ToEqual(53);
				data.Append(nullptr, 0);
				Expect(data.Size()).ToEqual(2);
				data.Append(buffer2, 2);
				Expect(data.Size()).ToEqual(4);
				Expect(data[2]).ToEqual(74);
				Expect(data[3]).ToEqual(51);
			});

			It("Can append to dynamic", []()
			{
				TArray<i32, 0> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Append(2, 33);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(33);
				Expect(data[1]).ToEqual(33);
				Expect(data.Data()).ToNotEqual(data.GetInlineBuffer());
			});

			It("Can assign to inline", []()
			{
				TArray<i32, 4> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Append(2, 33);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(33);
				Expect(data[1]).ToEqual(33);
				Expect(data.Data()).ToEqual(data.GetInlineBuffer());
			});
		});

		Describe("Assign", []()
		{
			It("Can assign defaulted", []()
			{
				TArray<i32, 0> data;
				data.Assign(2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(0);
				Expect(data[1]).ToEqual(0);
				data.Assign(0);
				Expect(data.Size()).ToEqual(0);
				data.Assign(2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(0);
				Expect(data[1]).ToEqual(0);
			});

			It("Can assign value", []()
			{
				TArray<i32, 0> data;
				data.Assign(2, 234);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(234);
				Expect(data[1]).ToEqual(234);
				data.Assign(0, 234);
				Expect(data.Size()).ToEqual(0);
				data.Assign(2, 235);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(235);
				Expect(data[1]).ToEqual(235);
			});

			It("Can assign multiple values", []()
			{
				TArray<i32, 0> data;
				i32 buffer[]{24, 53};
				i32 buffer2[]{74, 51};
				data.Assign(buffer, 2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(24);
				Expect(data[1]).ToEqual(53);
				data.Assign(nullptr, 0);
				Expect(data.Size()).ToEqual(0);
				data.Assign(buffer2, 2);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(74);
				Expect(data[1]).ToEqual(51);
			});

			It("Can assign to dynamic", []()
			{
				TArray<i32, 0> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Assign(2, 33);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(33);
				Expect(data[1]).ToEqual(33);
				Expect(data.Data()).ToNotEqual(data.GetInlineBuffer());
			});

			It("Can assign to inline", []()
			{
				TArray<i32, 4> data;
				data.Reserve(2);    // Reserve because we are not testing reallocation here
				data.Assign(2, 33);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(33);
				Expect(data[1]).ToEqual(33);
				Expect(data.Data()).ToEqual(data.GetInlineBuffer());
			});
		});

		Describe("Insert", []()
		{
			It("Can insert at empty", []()
			{
				TArray<i32, 0> data;
				data.Insert(0, 12);
				Expect(data.Size()).ToEqual(1);
				Expect(data[0]).ToEqual(12);

				data.Insert(0, 21);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(21);
			});

			It("Can insert at end", []()
			{
				TArray<i32, 0> data{12, 34};
				data.Insert(2, 12);
				Expect(data.Size()).ToEqual(3);
				Expect(data[2]).ToEqual(12);
			});

			It("Can insert to inline", []()
			{
				TArray<i32, 4> data;
				data.Insert(0, 12);
				data.Insert(0, 21);
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(21);
			});

			It("Can insert copied value", []()
			{
				TArray<i32, 0> data;
				data.Insert(0, 32);    // Insert at empty
				Expect(data.Size()).ToEqual(1);
				Expect(data[0]).ToEqual(32);

				data.Insert(0, 65);    // Insert at start
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(65);
				Expect(data[1]).ToEqual(32);

				data.Add(85);
				data.Insert(1, 27);    // Insert in the middle
				Expect(data.Size()).ToEqual(4);
				Expect(data[1]).ToEqual(27);

				data.Insert(4, 43);    // Insert in the end
				Expect(data.Size()).ToEqual(5);
				Expect(data[4]).ToEqual(43);
			});

			It("Can insert many values", []()
			{
				TArray<i32, 0> data;
				data.Insert(0, 2, 32);    // Insert at empty
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(32);
				Expect(data[1]).ToEqual(32);

				data.Insert(0, 2, 5);    // Insert at start
				Expect(data.Size()).ToEqual(4);
				Expect(data[0]).ToEqual(5);
				Expect(data[1]).ToEqual(5);
				Expect(data[2]).ToEqual(32);
				Expect(data[3]).ToEqual(32);

				data.Insert(3, 2, 6);    // Insert in the middle
				Expect(data.Size()).ToEqual(6);
				Expect(data[3]).ToEqual(6);
				Expect(data[4]).ToEqual(6);

				data.Insert(6, 2, 9);    // Insert in the end
				Expect(data.Size()).ToEqual(8);
				Expect(data[6]).ToEqual(9);
				Expect(data[7]).ToEqual(9);
			});

			It("Can insert many values inline", []()
			{
				TArray<i32, 24> data{1, 2};
				data.Insert(1, 3, 9);    // More values than trailing elements
				Expect(data.Size()).ToEqual(5);
				Expect(data[0]).ToEqual(1);
				Expect(data[1]).ToEqual(9);
				Expect(data[2]).ToEqual(9);
				Expect(data[3]).ToEqual(9);
				Expect(data[4]).ToEqual(2);

				data.Insert(0, 2, 7);    // Fewer values than trailing elements
				Expect(data.Size()).ToEqual(7);
				Expect(data[0]).ToEqual(7);
				Expect(data[1]).ToEqual(7);
				Expect(data[2]).ToEqual(1);
				Expect(data[3]).ToEqual(9);
				Expect(data[4]).ToEqual(9);
				Expect(data[5]).ToEqual(9);
				Expect(data[6]).ToEqual(2);

				data.Insert(5, 3, 4);    // One more value than trailing elements
				Expect(data.Size()).ToEqual(10);
				Expect(data[5]).ToEqual(4);
				Expect(data[6]).ToEqual(4);
				Expect(data[7]).ToEqual(4);
				Expect(data[8]).ToEqual(9);
				Expect(data[9]).ToEqual(2);

				data.Insert(3, 8, 6);    // One more value than trailing elements
				Expect(data.Size()).ToEqual(18);
				Expect(data[3]).ToEqual(6);
				Expect(data[10]).ToEqual(6);
				Expect(data[11]).ToEqual(9);
				Expect(data[17]).ToEqual(2);
			});

			It("Can insert buffer inline", []()
			{
				TArray<i32, 8> data{1, 2, 3};
				i32 src[]{4, 5, 6, 7};
				data.Insert(1, src, 4);    // More values than trailing elements
				Expect(data.Size()).ToEqual(7);
				Expect(data[0]).ToEqual(1);
				Expect(data[1]).ToEqual(4);
				Expect(data[4]).ToEqual(7);
				Expect(data[5]).ToEqual(2);
				Expect(data[6]).ToEqual(3);
			});

			It("Can insert many non trivial values inline", []()
			{
				TArray<CopyType, 8> data;
				data.Add(CopyType{1});
				data.Add(CopyType{2});
				data.Insert(1, 3, CopyType{9});    // More values than trailing elements
				Expect(data.Size()).ToEqual(5);
				Expect(data[0].value).ToEqual(1);
				Expect(data[1].value).ToEqual(9);
				Expect(data[2].value).ToEqual(9);
				Expect(data[3].value).ToEqual(9);
				Expect(data[4].value).ToEqual(2);
			});

			It("Can insert moved value", []()
			{
				TArray<MoveType, 0> data;
				MoveType tmp{34};
				data.Insert(0, Move(tmp));    // Insert at empty
				Expect(data.Size()).ToEqual(1);
				Expect(data[0].value).ToEqual(34);
				Expect(tmp.value).ToEqual(0);

				MoveType tmp2{4};
				data.Insert(0, Move(tmp2));    // Insert at start
				Expect(data.Size()).ToEqual(2);
				Expect(data[0].value).ToEqual(4);
				Expect(data[1].value).ToEqual(34);
				Expect(tmp2.value).ToEqual(0);

				MoveType tmp3{3};
				data.Add(MoveType{85});
				data.Insert(1, Move(tmp3));    // Insert in the middle
				Expect(data.Size()).ToEqual(4);
				Expect(data[1].value).ToEqual(3);
				Expect(tmp3.value).ToEqual(0);

				MoveType tmp4{7};
				data.Insert(4, Move(tmp4));    // Insert in the end
				Expect(data.Size()).ToEqual(5);
				Expect(data[4].value).ToEqual(7);
				Expect(tmp4.value).ToEqual(0);
			});

			It("Can insert buffer", []()
			{
				TArray<i32, 0> data;
				i32 src[]{34, 23, 844};
				data.Insert(0, src, 3);    // Insert at empty
				Expect(data.Size()).ToEqual(3);
				Expect(data[0]).ToEqual(34);
				Expect(data[1]).ToEqual(23);
				Expect(data[2]).ToEqual(844);

				i32 src2[]{2, 71, 21};
				data.Insert(0, src2, 3);    // Insert at start
				Expect(data.Size()).ToEqual(6);
				Expect(data[0]).ToEqual(2);
				Expect(data[1]).ToEqual(71);
				Expect(data[2]).ToEqual(21);
				Expect(data[3]).ToEqual(34);
				Expect(data[4]).ToEqual(23);
				Expect(data[5]).ToEqual(844);

				i32 src3[]{4, 3, 6};
				data.Insert(3, src3, 3);    // Insert in the middle
				Expect(data.Size()).ToEqual(9);
				Expect(data[3]).ToEqual(4);
				Expect(data[4]).ToEqual(3);
				Expect(data[5]).ToEqual(6);

				i32 src4[]{7, 2, 3};
				data.Insert(9, src4, 3);    // Insert in the end
				Expect(data.Size()).ToEqual(12);
				Expect(data[9]).ToEqual(7);
				Expect(data[10]).ToEqual(2);
				Expect(data[11]).ToEqual(3);
			});
		});

		Describe("Remove", []()
		{
			It("Can remove at index", []()
			{
				TArray<i32, 0> data{1, 2, 3, 4};

				// Check invalid inputs
				Expect(data.RemoveAt(-1)).ToEqual(false);
				Expect(data.RemoveAt(4)).ToEqual(false);

				Expect(data.RemoveAt(3)).ToEqual(true);    // Remove last
				Expect(data).ToEqual(TArray<i32, 0>{1, 2, 3});

				Expect(data.RemoveAt(1)).ToEqual(true);    // Remove in the middle
				Expect(data).ToEqual(TArray<i32, 0>{1, 3});

				Expect(data.RemoveAt(0)).ToEqual(true);    // remove first
				Expect(data).ToEqual(TArray<i32, 0>{3});
			});

			It("Can remove many at index", []()
			{
				TArray<i32, 0> data{1, 2, 3, 4, 5, 6, 7, 8};

				// Check invalid inputs
				Expect(data.RemoveAt(-1, 2)).ToEqual(false);
				Expect(data.RemoveAt(8, 2)).ToEqual(false);
				Expect(data.RemoveAt(7, 2)).ToEqual(false);

				Expect(data.RemoveAt(6, 2)).ToEqual(true);    // Remove last
				Expect(data).ToEqual(TArray<i32, 0>{1, 2, 3, 4, 5, 6});

				Expect(data.RemoveAt(2, 2)).ToEqual(true);    // Remove in the middle
				Expect(data).ToEqual(TArray<i32, 0>{1, 2, 5, 6});

				Expect(data.RemoveAt(0, 2)).ToEqual(true);    // Remove first
				Expect(data).ToEqual(TArray<i32, 0>{5, 6});
			});

			It("Can remove swap at index", []()
			{
				TArray<i32, 0> data{1, 2, 3, 4, 5};

				// Check invalid inputs
				Expect(data.RemoveAtSwap(-1)).ToEqual(false);
				Expect(data.RemoveAtSwap(5)).ToEqual(false);

				Expect(data.RemoveAtSwap(3)).ToEqual(true);    // Remove last
				Expect(data).ToEqual(TArray<i32, 0>{1, 2, 3, 5});

				Expect(data.RemoveAtSwap(1)).ToEqual(true);    // Remove swapping
				Expect(data).ToEqual(TArray<i32, 0>{1, 5, 3});

				Expect(data.RemoveAtSwap(0)).ToEqual(true);    // Remove first
				Expect(data).ToEqual(TArray<i32, 0>{3, 5});
			});

			It("Can remove swap many at index", []()
			{
				TArray<i32, 0> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

				// Check invalid inputs
				Expect(data.RemoveAtSwap(-1, 2)).ToEqual(false);
				Expect(data.RemoveAtSwap(10, 2)).ToEqual(false);
				Expect(data.RemoveAtSwap(9, 2)).ToEqual(false);

				Expect(data.RemoveAtSwap(8, 2)).ToEqual(true);    // Remove last
				Expect(data).ToEqual(TArray<i32, 0>{1, 2, 3, 4, 5, 6, 7, 8});

				Expect(data.RemoveAtSwap(1, 2)).ToEqual(true);    // Removes swapping
				Expect(data).ToEqual(TArray<i32, 0>{1, 7, 8, 4, 5, 6});

				Expect(data.RemoveAtSwap(1, 3)).ToEqual(true);    // Removes swapping with less left
				Expect(data).ToEqual(TArray<i32, 0>{1, 5, 6});

				Expect(data.RemoveAtSwap(0, 2)).ToEqual(true);    // Remove first
				Expect(data).ToEqual(TArray<i32, 0>{6});
			});

			It("Can RemoveLast", []()
			{
				TArray<i32> data{1, 4, 6};
				data.RemoveLast();
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(1);
				Expect(data[1]).ToEqual(4);
				Expect(data.Capacity()).ToEqual(2);
			});

			It("Can RemoveLast N", []()
			{
				TArray<i32> dataA{1, 4, 6};
				dataA.RemoveLast(2);
				Expect(dataA.Size()).ToEqual(1);
				Expect(dataA[0]).ToEqual(1);
				Expect(dataA.Capacity()).ToEqual(1);

				TArray<i32> dataB{1, 4, 6};
				dataB.RemoveLast(3);
				Expect(dataB.Size()).ToEqual(0);
				Expect(dataB.Capacity()).ToEqual(0);
			});

			It("Can RemoveIf", []()
			{
				TArray<i32> data{1, 4, 5, 6};

				Expect(data.Size()).ToEqual(4);

				data.RemoveIf([](i32 v)
				{
					return v == 1 || v == 6;
				});
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(4);
				Expect(data[1]).ToEqual(5);
			});

			It("Can RemoveIfSwap", []()
			{
				TArray<i32> data{1, 4, 5, 6};

				Expect(data.Size()).ToEqual(4);

				data.RemoveIfSwap([](i32 v)
				{
					return v == 1 || v == 6;
				});
				Expect(data.Size()).ToEqual(2);
				Expect(data[0]).ToEqual(5);
				Expect(data[1]).ToEqual(4);
			});
		});

		It("Can Sort", []()
		{
			TArray<i32> data0{34, 1, 5};
			data0.Sort();    // Default sort is less
			Expect(data0[0]).ToEqual(1);
			Expect(data0[1]).ToEqual(5);
			Expect(data0[2]).ToEqual(34);

			TArray<i32> data1{34, 1, 5};
			data1.Sort(TGreater<i32>{});
			Expect(data1[0]).ToEqual(34);
			Expect(data1[1]).ToEqual(5);
			Expect(data1[2]).ToEqual(1);
		});

		It("Can find in AddUniqueSorted", []()
		{
			TArray<i32> data{1, 5, 5, 34};

			Expect(data.AddUniqueSorted(1)).ToEqual(0);
			Expect(data.AddUniqueSorted(5)).ToEqual(1);
			Expect(data.AddUniqueSorted(34)).ToEqual(3);
			Expect(data.Size()).ToEqual(4);
		});

		It("Can add in AddUniqueSorted", []()
		{
			TArray<i32> data{1, 5, 5, 34};

			Expect(data.AddUniqueSorted(2)).ToEqual(1);
			Expect(data.Size()).ToEqual(5);

			Expect(data.AddUniqueSorted(6)).ToEqual(4);
			Expect(data.Size()).ToEqual(6);

			Expect(data.AddUniqueSorted(36)).ToEqual(6);
			Expect(data.Size()).ToEqual(7);
		});

		It("Can slice", []()
		{
			TArray<i32> data{1, 2, 3, 4, 5};

			auto mid = data.Slice(1, 2);    // Elements 1 to 3
			Expect(mid.Size()).ToEqual(2);
			Expect(mid[0]).ToEqual(2);
			Expect(mid[1]).ToEqual(3);

			auto tail = data.Slice(3, 100);    // Clamped to available elements
			Expect(tail.Size()).ToEqual(2);
			Expect(tail[0]).ToEqual(4);
			Expect(tail[1]).ToEqual(5);

			auto none = data.Slice(2, 0);    // Zero length
			Expect(none.IsEmpty()).ToBeTrue();

			auto end = data.Slice(5, 2);    // Offset clamped to size
			Expect(end.IsEmpty()).ToBeTrue();
		});

		It("Can slice views", []()
		{
			TArray<i32> data{1, 2, 3, 4, 5};
			TView<const i32> view = data;

			auto mid = view.Slice(2, 2);    // Elements 2 to 4
			Expect(mid.Size()).ToEqual(2);
			Expect(mid[0]).ToEqual(3);
			Expect(mid[1]).ToEqual(4);

			auto head = view.Slice(0, 3);
			Expect(head.Size()).ToEqual(3);
			Expect(head[0]).ToEqual(1);
			Expect(head[2]).ToEqual(3);
		});

		Describe("Iterate", []()
		{
			It("Can iterate empty", []()
			{
				TArray<i32, 5> data1{};
				i32 counter = 0;
				for (i32 v : data1)
				{
					++counter;
				}

				Expect(counter).ToEqual(0);
				TArray<i32, 0> data2{};    // Without inline capacity
				counter = 0;
				for (i32 v : data2)
				{
					++counter;
				}
				Expect(counter).ToEqual(0);
			});

			It("Can iterate non empty", []()
			{
				TArray<i32, 5> data1{1, 3, 4};
				const i32 mirror[]{1, 3, 4};
				i32 counter = 0;
				for (i32 v : data1)
				{
					Expect(v).ToEqual(mirror[counter]);
					++counter;
				}
				Expect(counter).ToEqual(3);

				TArray<i32, 0> data2{1, 3, 4};    // Without inline capacity
				counter = 0;
				for (i32 v : data2)
				{
					Expect(v).ToEqual(mirror[counter]);
					++counter;
				}
				Expect(counter).ToEqual(3);
			});
		});
	});

	Describe("Containers.BitArray", []()
	{
		It("Can initialize", []()
		{
			BitArray data1{};
			BitArray data2(3);
			BitArray data3(3, true);
			BitArray data4(91, true);
			BitArray data5{false, true, false, true, false, true};

			Expect(data1.Size()).ToEqual(0);
			Expect(data1.Capacity()).ToEqual(0);
			Expect(data2.Size()).ToEqual(3);
			Expect(data2.Capacity()).ToEqual(32);
			Expect(data3.Size()).ToEqual(3);
			Expect(data3.Capacity()).ToEqual(32);
			Expect(data4.Size()).ToEqual(91);
			Expect(data4.Capacity()).ToEqual(96);
			Expect(data5.Size()).ToEqual(6);
			Expect(data5.Capacity()).ToEqual(32);

			Expect(data2[0]).ToEqual(false);
			Expect(data2[2]).ToEqual(false);
			Expect(data3[0]).ToEqual(true);
			Expect(data3[2]).ToEqual(true);
			Expect(data4[0]).ToEqual(true);
			Expect(data4[90]).ToEqual(true);
			Expect(data5[0]).ToEqual(false);
			Expect(data5[1]).ToEqual(true);
			Expect(data5[2]).ToEqual(false);
			Expect(data5[3]).ToEqual(true);
			Expect(data5[4]).ToEqual(false);
			Expect(data5[5]).ToEqual(true);
		});

		Describe("Copy", []()
		{
			It("Can copy empty", []()
			{
				BitArray source1{};
				BitArray target1 = source1;    // NOLINT
				Expect(target1.Data()).ToEqual(nullptr);
				Expect(target1.Size()).ToEqual(0);
				Expect(target1.Capacity()).ToEqual(0);
				BitArray source2{};
				BitArray target2 = source2;    // NOLINT
				Expect(target2.Data()).ToEqual(nullptr);
				Expect(target2.Size()).ToEqual(0);
				Expect(target2.Capacity()).ToEqual(0);
			});

			It("Can copy", []()
			{
				BitArray source{false, true, false, true, false, true};
				BitArray target = source;
				Expect(source.Size()).ToEqual(6);
				Expect(source.Capacity()).ToBeGreaterOrEqual(6);
				Expect(target.Size()).ToEqual(6);
				Expect(target.Capacity()).ToBeGreaterOrEqual(6);
				Expect(target[1]).ToEqual(true);
				Expect(target[2]).ToEqual(false);
				Expect(target[3]).ToEqual(true);
				Expect(source.Data()).ToNotEqual(nullptr);
				Expect(target.Data()).ToNotEqual(nullptr);
			});
		});

		Describe("Move", []()
		{
			It("Can move empty", []()
			{
				BitArray source1{};
				BitArray target1 = Move(source1);
				Expect(target1.Data()).ToEqual(nullptr);
				Expect(target1.Size()).ToEqual(0);
				Expect(target1.Capacity()).ToEqual(0);
				BitArray source2{};
				BitArray target2 = Move(source2);
				Expect(target2.Data()).ToEqual(nullptr);
				Expect(target2.Size()).ToEqual(0);
				Expect(target2.Capacity()).ToEqual(0);
			});

			It("Can move", []()
			{
				BitArray source{false, true, false, true, false, true};
				u32* sourceData = source.Data();
				BitArray target = Move(source);
				Expect(source.Size()).ToEqual(0);
				Expect(source.Capacity()).ToEqual(0);
				Expect(target.Size()).ToEqual(6);
				Expect(target.Capacity()).ToBeGreaterOrEqual(6);
				Expect(target[1]).ToEqual(true);
				Expect(target[2]).ToEqual(false);
				Expect(target[3]).ToEqual(true);
				Expect(source.Data()).ToEqual(nullptr);
				Expect(target.Data()).ToEqual(sourceData);
			});

			It("Can bitwise operate", []()
			{
				BitArray a{true, true, false, false};
				BitArray b{true, false, true, false};

				const BitArray anded  = a & b;
				const BitArray ored   = a | b;
				const BitArray xored  = a ^ b;
				const BitArray negged = ~a;

				// a & b: only bit 0 is set in both
				Expect(anded.IsSet(0)).ToBeTrue();
				Expect(anded.IsSet(1)).ToBeFalse();
				Expect(anded.IsSet(2)).ToBeFalse();
				Expect(anded.IsSet(3)).ToBeFalse();

				// a | b: all bits set
				Expect(ored.IsSet(0)).ToBeTrue();
				Expect(ored.IsSet(1)).ToBeTrue();
				Expect(ored.IsSet(2)).ToBeTrue();
				Expect(ored.IsSet(3)).ToBeFalse();

				// a ^ b: bits 1 and 2 differ
				Expect(xored.IsSet(0)).ToBeFalse();
				Expect(xored.IsSet(1)).ToBeTrue();
				Expect(xored.IsSet(2)).ToBeTrue();
				Expect(xored.IsSet(3)).ToBeFalse();

				// ~a: all bits flipped
				Expect(negged.IsSet(0)).ToBeFalse();
				Expect(negged.IsSet(1)).ToBeFalse();
				Expect(negged.IsSet(2)).ToBeTrue();
				Expect(negged.IsSet(3)).ToBeTrue();

				// Compound operations
				BitArray compound = a;
				compound &= b;
				Expect(compound.IsSet(0)).ToBeTrue();
				Expect(compound.IsSet(1)).ToBeFalse();
				compound |= b;
				Expect(compound.IsSet(2)).ToBeTrue();
				compound ^= b;
				Expect(compound.IsSet(0)).ToBeFalse();
				Expect(compound.IsSet(2)).ToBeFalse();
			});

			It("Can bitwise operate with different sizes", []()
			{
				BitArray small{false};
				BitArray big{true, true, true};

				const BitArray anded = big & small;
				Expect(anded.Size()).ToEqual(1);
				Expect(anded.IsSet(0)).ToBeFalse();

				const BitArray ored = big | small;
				Expect(ored.Size()).ToEqual(1);    // Sized to the smallest operand
				Expect(ored.IsSet(0)).ToBeTrue();

				// Only whole words are operated on. Bits past the smallest word count
				// keep their value. Bits within a cleared word are cleared with it.
				BitArray large{false};
				large.Resize(40, true);
				large &= small;    // small has a single (zeroed) word
				Expect(large.Size()).ToEqual(40);
				Expect(large.IsSet(0)).ToBeFalse();
				Expect(large.IsSet(31)).ToBeFalse();    // Same word as bit 0
				Expect(large.IsSet(32)).ToBeTrue();     // Next word, unaffected
				Expect(large.IsSet(39)).ToBeTrue();
			});
		});
	});
}
