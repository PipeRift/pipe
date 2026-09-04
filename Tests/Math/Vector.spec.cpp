// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeVectors.h>


using namespace p;


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Math.Vector", []()
{
	Describe("v2", []()
	{
		It("Can reflect", []()
		{
			p::v2 v{0.f, 1.f};
			p::v2 normal{1.f, 0.f};
			p::v2 v2 = v.Reflect(normal);
			Expect(v2.Equals({0.f, 1.f})).ToEqual(true);
			v      = p::v2{0.f, 1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({0.f, -1.f})).ToEqual(true);
			v      = p::v2{1.f, 1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({1.f, -1.f})).ToEqual(true);
			v      = p::v2{1.f, 1.f};
			normal = p::v2{1.f, 0.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({-1.f, 1.f})).ToEqual(true);
			v      = p::v2{-1.f, 1.f};
			normal = p::v2{-1.f, 0.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({1.f, 1.f})).ToEqual(true);
			v      = p::v2{-1.f, -1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({-1.f, 1.f})).ToEqual(true);
			v      = p::v2{0.f, 1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({0.f, -1.f})).ToEqual(true);
			v      = p::v2{0.f, -1.f};
			normal = p::v2{0.f, 1.f};
			v2     = v.Reflect(normal);
			Expect(v2.Equals({0.f, 1.f})).ToEqual(true);
		});

		It("Can convert to angle", []()
		{
			float anglea = p::v2{0.f, 1.f}.Angle();
			Expect(anglea).ToEqual(90.f);
			float angleb = p::v2{0.f, -1.f}.Angle();
			Expect(angleb).ToEqual(-90.f);
			float anglec = p::v2{1.f, 0.f}.Angle();
			Expect(anglec).ToEqual(0.f);
			float angled = p::v2{-1.f, 0.f}.Angle();
			Expect(angled).ToEqual(180.f);
		});

		It("Can convert from angle", []()
		{
			Expect(p::v2::FromAngle(0.f).Angle()).ToEqual(0);
			Expect(p::v2::FromAngle(90.f).Angle()).ToEqual(90.f);
		});
	});
});
return true;
}();
}    // namespace
