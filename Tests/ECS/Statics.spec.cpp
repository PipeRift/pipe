// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
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


go_bandit([]()
{
	describe("ECS.Statics", []()
	{
		it("Can set an static", [&]()
		{
			IdContext ctx;
			AssertThat(ctx.HasStatic<StaticType>(), Equals(false));
			auto& var = ctx.SetStatic<StaticType>({4});
			AssertThat(var.i, Equals(4));
			AssertThat(ctx.HasStatic<StaticType>(), Equals(true));
			AssertThat(ctx.HasStatic<StaticTypeTwo>(), Equals(false));
		});
		it("Can set two statics", [&]()
		{
			IdContext ctx;
			AssertThat(ctx.HasStatic<StaticType>(), Equals(false));
			AssertThat(ctx.HasStatic<StaticTypeTwo>(), Equals(false));
			auto& var1 = ctx.SetStatic<StaticType>({4});
			auto& var2 = ctx.SetStatic<StaticTypeTwo>({2});
			AssertThat(var1.i, Equals(4));
			AssertThat(var2.i, Equals(2));
			AssertThat(ctx.HasStatic<StaticType>(), Equals(true));
			AssertThat(ctx.HasStatic<StaticTypeTwo>(), Equals(true));
		});
		it("Can replace an static", [&]()
		{
			IdContext ctx;
			AssertThat(ctx.HasStatic<StaticType>(), Equals(false));
			ctx.SetStatic<StaticType>({4});
			ctx.SetStatic<StaticType>({2});
			AssertThat(ctx.GetStatic<StaticType>().i, Equals(2));
			AssertThat(ctx.HasStatic<StaticType>(), Equals(true));
		});
		it("Can get or set an static", [&]()
		{
			IdContext ctx;
			// Can set
			AssertThat(ctx.GetOrSetStatic<StaticType>({4}).i, Equals(4));
			// Can get
			AssertThat(ctx.GetOrSetStatic<StaticType>({10}).i, Equals(4));
		});
		it("Can remove an static", [&]()
		{
			IdContext ctx;
			ctx.SetStatic<StaticType>();
			AssertThat(ctx.HasStatic<StaticType>(), Equals(true));
			AssertThat(ctx.RemoveStatic<StaticType>(), Is().True());
			AssertThat(ctx.HasStatic<StaticType>(), Equals(false));

			AssertThat(ctx.RemoveStatic<StaticType>(), Is().False());
		});

		it("Can get statics", [&]()
		{
			IdContext ctx;
			ctx.SetStatic<StaticType>({4});
			ctx.SetStatic<StaticTypeTwo>({2});
			AssertThat(ctx.GetStatic<StaticType>().i, Equals(4));
			AssertThat(ctx.GetStatic<StaticTypeTwo>().i, Equals(2));

			ctx.SetStatic<StaticTypeThree>({14});
			AssertThat(ctx.GetStatic<StaticTypeThree>().i, Equals(14));

			ctx.RemoveStatic<StaticTypeThree>();
			AssertThat(ctx.TryGetStatic<StaticTypeThree>(), Is().Null());
		});
	});
});
