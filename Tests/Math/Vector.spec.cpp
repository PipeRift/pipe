// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Math/Vector.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Math.Vector", []() {
		describe("v2", []() {
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
				v      = p::v2{0.f, 1.f};
				normal = p::v2{0.f, 1.f};
				v2     = v.Reflect(normal);
				AssertThat(v2.Equals({0.f, -1.f}), Equals(true));
				v      = p::v2{0.f, -1.f};
				normal = p::v2{0.f, 1.f};
				v2     = v.Reflect(normal);
				AssertThat(v2.Equals({0.f, 1.f}), Equals(true));
			});

			it("Can convert to angle", [&]() {
				float anglea = p::v2{0.f, 1.f}.Angle();
				AssertThat(anglea, Equals(90.f));
				float angleb = p::v2{0.f, -1.f}.Angle();
				AssertThat(angleb, Equals(-90.f));
				float anglec = p::v2{1.f, 0.f}.Angle();
				AssertThat(anglec, Equals(0.f));
				float angled = p::v2{-1.f, 0.f}.Angle();
				AssertThat(angled, Equals(180.f));
			});

			it("Can convert from angle", [&]() {
				AssertThat(p::v2::FromAngle(0.f).Angle(), Equals(0));
				AssertThat(p::v2::FromAngle(90.f).Angle(), Equals(90.f));
			});
		});
	});
});
