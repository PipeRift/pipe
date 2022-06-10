// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Math/Vector.h>

#include <cmath>
#include <limits>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Math.Vector", []() {
		it("Can reflect", [&]() {
			p::v2 v{0.f, 1.f};
			p::v2 normal{1.f, 0.f};
			p::v2 v2 = v.Reflect(normal);
			AssertThat(v2.Equals({0.f, 1.f}), Equals(true));
			v      = p::v2{0.f, 1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			AssertThat(v2.Equals({0.f, -1.f}), Equals(true));
			v      = p::v2{1.f, 1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			AssertThat(v2.Equals({1.f, -1.f}), Equals(true));
			v      = p::v2{1.f, 1.f};
			normal = p::v2{1.f, 0.f};
			v2     = v.Reflect(normal);
			AssertThat(v2.Equals({-1.f, 1.f}), Equals(true));
			v      = p::v2{-1.f, 1.f};
			normal = p::v2{-1.f, 0.f};
			v2     = v.Reflect(normal);
			AssertThat(v2.Equals({1.f, 1.f}), Equals(true));
			v      = p::v2{-1.f, -1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			AssertThat(v2.Equals({-1.f, 1.f}), Equals(true));
		});
	});
});
