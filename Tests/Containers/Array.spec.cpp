// Copyright 2015-2021 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Containers/Array.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


go_bandit([]() {
	describe("Containers.Array", []() {
		it("Can initialize", [&]() {
			TArray<i32> data1{};
			TArray<i32> data2(3);
			TArray<i32> data3(3, 0);
			TArray<i32> data4{3, 3, 3, 3};

			AssertThat(data1.Size(), Equals(0));
			AssertThat(data2.Size(), Equals(3));
			AssertThat(data3.Size(), Equals(3));
			AssertThat(data4.Size(), Equals(4));


			AssertThat(data2[0], Equals(0));
			AssertThat(data2[2], Equals(0));
			AssertThat(data3[0], Equals(0));
			AssertThat(data3[2], Equals(0));
			AssertThat(data4[0], Equals(3));
			AssertThat(data4[3], Equals(3));
		});

		it("Can copy", [&]() {
			TArray<i32> data1{6, 6};
			TArray<i32> data2{data1};
			AssertThat(data1.Size(), Equals(2));
			AssertThat(data2.Size(), Equals(2));

			TArray<i32> data3{6, 6};
			TArray<i32> data4;
			data4 = data3;
			AssertThat(data3.Size(), Equals(2));
			AssertThat(data4.Size(), Equals(2));
		});

		it("Can move", [&]() {
			TArray<i32> data1{4, 4};
			AssertThat(data1.Size(), Equals(2));

			TArray<i32> data2{Move(data1)};
			AssertThat(data1.Size(), Equals(0));
			AssertThat(data2.Size(), Equals(2));

			TArray<i32> data3{4, 4};
			TArray<i32> data4;
			AssertThat(data3.Size(), Equals(2));
			AssertThat(data4.Size(), Equals(0));

			data4 = Move(data3);
			AssertThat(data3.Size(), Equals(0));
			AssertThat(data4.Size(), Equals(2));
		});

		it("Can access data", [&]() {
			TArray<i32> data1;
			TArray<i32> data2{1};

			AssertThat(data1.Data(), Equals(nullptr));
			AssertThat(data2.Data(), !Equals(nullptr));
		});

		it("Can RemoveIf", [&]() {
			TArray<i32> data{1, 4, 5, 6};

			AssertThat(data.Size(), Equals(4));

			data.RemoveIf([](i32 v) {
				return v == 1 || v == 6;
			});
			AssertThat(data.Size(), Equals(2));
			AssertThat(data[0], Equals(4));
			AssertThat(data[1], Equals(5));
		});

		it("Can RemoveIfSwap", [&]() {
			TArray<i32> data{1, 4, 5, 6};

			AssertThat(data.Size(), Equals(4));

			data.RemoveIfSwap([](i32 v) {
				return v == 1 || v == 6;
			});
			AssertThat(data.Size(), Equals(2));
			AssertThat(data[0], Equals(5));
			AssertThat(data[1], Equals(4));
		});
	});
});
