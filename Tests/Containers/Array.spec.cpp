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
	describe("Containers.Array", []() {
		it("Can initialize", [&]() {
			TArray2<i32> data1{};
			TArray2<i32> data2(3);
			TArray2<i32> data3(3, 0);
			TArray2<i32> data4{3, 3, 3, 3};

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
	});
});
