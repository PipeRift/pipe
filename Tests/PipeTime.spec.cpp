// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeTime.h>


using namespace p;


Spec("Time.DateTime", []()
{
	It("Can get day of year", []()
	{
		DateTime time1{2024, 1, 1};
		Expect(time1.GetDayOfYear()).ToEqual(1);
		DateTime time11{2024, 1, 30};
		Expect(time11.GetDayOfYear()).ToEqual(30);
		DateTime time12{2024, 1, 31};
		Expect(time12.GetDayOfYear()).ToEqual(31);

		DateTime time2{2024, 2, 1};
		Expect(time2.GetDayOfYear()).ToEqual(32);
		DateTime time3{2024, 3, 1};
		Expect(time3.GetDayOfYear()).ToEqual(60);
		DateTime time4{2024, 12, 31};
		Expect(time4.GetDayOfYear()).ToEqual(365);
	});
});
