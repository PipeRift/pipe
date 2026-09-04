// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeECS.h>
#include <PipeTest.h>


using namespace p;
using namespace std::chrono_literals;


struct StaticType
{
	i32 i = 0;
};
struct StaticTypeTwo
{
	i32 i = 0;
};
struct StaticTypeThree
{
	i32 i = 0;
};


Spec("ECS.Statics", []()
{
	It("Can set an static", []()
	{
		IdContext ctx;
		Expect(ctx.HasStatic<StaticType>()).ToEqual(false);
		auto& var = ctx.SetStatic<StaticType>({4});
		Expect(var.i).ToEqual(4);
		Expect(ctx.HasStatic<StaticType>()).ToEqual(true);
		Expect(ctx.HasStatic<StaticTypeTwo>()).ToEqual(false);
	});
	It("Can set two statics", []()
	{
		IdContext ctx;
		Expect(ctx.HasStatic<StaticType>()).ToEqual(false);
		Expect(ctx.HasStatic<StaticTypeTwo>()).ToEqual(false);
		auto& var1 = ctx.SetStatic<StaticType>({4});
		auto& var2 = ctx.SetStatic<StaticTypeTwo>({2});
		Expect(var1.i).ToEqual(4);
		Expect(var2.i).ToEqual(2);
		Expect(ctx.HasStatic<StaticType>()).ToEqual(true);
		Expect(ctx.HasStatic<StaticTypeTwo>()).ToEqual(true);
	});
	It("Can replace an static", []()
	{
		IdContext ctx;
		Expect(ctx.HasStatic<StaticType>()).ToEqual(false);
		ctx.SetStatic<StaticType>({4});
		ctx.SetStatic<StaticType>({2});
		Expect(ctx.GetStatic<StaticType>().i).ToEqual(2);
		Expect(ctx.HasStatic<StaticType>()).ToEqual(true);
	});
	It("Can get or set an static", []()
	{
		IdContext ctx;
		// Can set
		Expect(ctx.GetOrSetStatic<StaticType>({4}).i).ToEqual(4);
		// Can get
		Expect(ctx.GetOrSetStatic<StaticType>({10}).i).ToEqual(4);
	});
	It("Can remove an static", []()
	{
		IdContext ctx;
		ctx.SetStatic<StaticType>();
		Expect(ctx.HasStatic<StaticType>()).ToEqual(true);
		Expect(ctx.RemoveStatic<StaticType>()).ToBeTrue();
		Expect(ctx.HasStatic<StaticType>()).ToEqual(false);

		Expect(ctx.RemoveStatic<StaticType>()).ToBeFalse();
	});

	It("Can get statics", []()
	{
		IdContext ctx;
		ctx.SetStatic<StaticType>({4});
		ctx.SetStatic<StaticTypeTwo>({2});
		Expect(ctx.GetStatic<StaticType>().i).ToEqual(4);
		Expect(ctx.GetStatic<StaticTypeTwo>().i).ToEqual(2);

		ctx.SetStatic<StaticTypeThree>({14});
		Expect(ctx.GetStatic<StaticTypeThree>().i).ToEqual(14);

		ctx.RemoveStatic<StaticTypeThree>();
		Expect(ctx.TryGetStatic<StaticTypeThree>()).ToEqual(nullptr);
	});
});
