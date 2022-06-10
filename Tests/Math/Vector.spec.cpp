// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Math/Vector.h>

#include <cmath>
#include <limits>


using namespace p;


go_bandit([]() {
	describe("Math.Vector", []() {
		it("LowerBound", [&]() {
			p::v2 v{0.f, 1.f};
			p::v2 normal{1.f, 0.f};
			p::v2 v2 = v.Reflect(normal);
			AssertThat(v.Equals(v2), Equals(true));
		});
	});
});
