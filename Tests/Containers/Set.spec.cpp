// Copyright 2015-2021 Piperift - All rights reserved

#include <Containers/Set.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;

template <u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


go_bandit([]() {
	describe("Containers.Set", []() {
		it("Can initialize", [&]() {
			TSet<i32> data1{};
			TSet<i32> data2(u32(3));
			TSet<i32> data3{5, 4, 3, 2};

			AssertThat(data1.Size(), Equals(0));
			AssertThat(data2.Size(), Equals(0));
			AssertThat(data3.Size(), Equals(4));

			AssertThat(data3[2], Equals(2));
			AssertThat(data3[3], Equals(3));
			AssertThat(data3[4], Equals(4));
			AssertThat(data3[5], Equals(5));
		});

		it("Can copy", [&]() {
			TSet<i32> data1{6, 5};
			TSet<i32> data2{data1};
			AssertThat(data1.Size(), Equals(2));
			AssertThat(data2.Size(), Equals(2));
			AssertThat(data2[5], Equals(5));
			AssertThat(data2[6], Equals(6));

			TSet<i32> data3{6, 5};
			TSet<i32> data4;
			data4 = data3;
			AssertThat(data3.Size(), Equals(2));
			AssertThat(data4.Size(), Equals(2));
			AssertThat(data4[5], Equals(5));
			AssertThat(data4[6], Equals(6));
		});

		it("Can move", [&]() {
			TSet<i32> data1{4, 3};
			AssertThat(data1.Size(), Equals(2));

			TSet<i32> data2{Move(data1)};
			AssertThat(data1.Size(), Equals(0));
			AssertThat(data2.Size(), Equals(2));

			TSet<i32> data3{4, 3};
			TSet<i32> data4;
			AssertThat(data3.Size(), Equals(2));
			AssertThat(data4.Size(), Equals(0));

			data4 = Move(data3);
			AssertThat(data3.Size(), Equals(0));
			AssertThat(data4.Size(), Equals(2));
			AssertThat(data4[3], Equals(3));
			AssertThat(data4[4], Equals(4));
		});

		it("Can access data", [&]() {
			TSet<i32> data1;
			TSet<i32> data2{1, 5};

			AssertThat(data1.Size(), Equals(0));
			AssertThat(data2.Size(), IsGreaterThanOrEqualTo(2));

			AssertThat(data1.Contains(3), Equals(false));
			AssertThat(data2.Contains(1), Equals(true));
			AssertThat(data2.Contains(5), Equals(true));
			AssertThat(data2.Contains(34), Equals(false));
			AssertThat(data2[1], Equals(1));
			AssertThat(data2[5], Equals(5));
		});
	});
});
