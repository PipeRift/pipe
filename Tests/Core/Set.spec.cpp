// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <Pipe/Core/Set.h>


using namespace p;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{0};    // Fill data for debugging
};


void RegisterCoreSetTests()
{
	Spec("Core.Set", []()
	{
		It("Can initialize", []()
		{
			TSet<i32> data1{};
			TSet<i32> data2(u32(3));
			TSet<i32> data3{5, 4, 3, 2};

			Expect(data1.Size()).ToEqual(0);
			Expect(data2.Size()).ToEqual(0);
			Expect(data3.Size()).ToEqual(4);

			Expect(data3[2]).ToEqual(2);
			Expect(data3[3]).ToEqual(3);
			Expect(data3[4]).ToEqual(4);
			Expect(data3[5]).ToEqual(5);
		});

		It("Can copy", []()
		{
			TSet<i32> data1{6, 5};
			TSet<i32> data2{data1};
			Expect(data1.Size()).ToEqual(2);
			Expect(data2.Size()).ToEqual(2);
			Expect(data2[5]).ToEqual(5);
			Expect(data2[6]).ToEqual(6);

			TSet<i32> data3{6, 5};
			TSet<i32> data4;
			data4 = data3;
			Expect(data3.Size()).ToEqual(2);
			Expect(data4.Size()).ToEqual(2);
			Expect(data4[5]).ToEqual(5);
			Expect(data4[6]).ToEqual(6);
		});

		It("Can move", []()
		{
			TSet<i32> data1{4, 3};
			Expect(data1.Size()).ToEqual(2);

			TSet<i32> data2{Move(data1)};
			Expect(data1.Size()).ToEqual(0);
			Expect(data2.Size()).ToEqual(2);

			TSet<i32> data3{4, 3};
			TSet<i32> data4;
			Expect(data3.Size()).ToEqual(2);
			Expect(data4.Size()).ToEqual(0);

			data4 = Move(data3);
			Expect(data3.Size()).ToEqual(0);
			Expect(data4.Size()).ToEqual(2);
			Expect(data4[3]).ToEqual(3);
			Expect(data4[4]).ToEqual(4);
		});

		It("Can access data", []()
		{
			TSet<i32> data1;
			TSet<i32> data2{1, 5};

			Expect(data1.Size()).ToEqual(0);
			Expect(data2.Size()).ToBeGreaterOrEqual(2);

			Expect(data1.Contains(3)).ToEqual(false);
			Expect(data2.Contains(1)).ToEqual(true);
			Expect(data2.Contains(5)).ToEqual(true);
			Expect(data2.Contains(34)).ToEqual(false);
			Expect(data2[1]).ToEqual(1);
			Expect(data2[5]).ToEqual(5);
		});
	});
}
