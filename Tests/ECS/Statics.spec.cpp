// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
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


go_bandit([]() {
	describe("ECS.Statics", []() {
		it("Can set an static", [&]() {
			ECS::Context ast;
			auto& var = ast.SetStatic<StaticType>({4});
			AssertThat(var.i, Equals(4));
		});
		it("Can set two statics", [&]() {
			ECS::Context ast;
			auto& var1 = ast.SetStatic<StaticType>({4});
			auto& var2 = ast.SetStatic<StaticTypeTwo>({2});
			AssertThat(var1.i, Equals(4));
			AssertThat(var2.i, Equals(2));
		});
		it("Can replace an static", [&]() {
			ECS::Context ast;
			ast.SetStatic<StaticType>({4});
			ast.SetStatic<StaticType>({2});
			AssertThat(ast.GetStatic<StaticType>().i, Equals(2));
		});
		it("Can get or set an static", [&]() {
			ECS::Context ast;
			// Can set
			AssertThat(ast.GetOrSetStatic<StaticType>({4}).i, Equals(4));
			// Can get
			AssertThat(ast.GetOrSetStatic<StaticType>({10}).i, Equals(4));
		});
		it("Can remove an static", [&]() {
			ECS::Context ast;
			ast.SetStatic<StaticType>();
			AssertThat(ast.RemoveStatic<StaticType>(), Is().True());

			AssertThat(ast.RemoveStatic<StaticType>(), Is().False());
		});

		it("Can get statics", [&]() {
			ECS::Context ast;
			ast.SetStatic<StaticType>({4});
			ast.SetStatic<StaticTypeTwo>({2});
			AssertThat(ast.GetStatic<StaticType>().i, Equals(4));
			AssertThat(ast.GetStatic<StaticTypeTwo>().i, Equals(2));

			ast.SetStatic<StaticTypeThree>({14});
			AssertThat(ast.GetStatic<StaticTypeThree>().i, Equals(14));

			ast.RemoveStatic<StaticTypeThree>();
			AssertThat(ast.TryGetStatic<StaticTypeThree>(), Is().Null());
		});
	});
});
