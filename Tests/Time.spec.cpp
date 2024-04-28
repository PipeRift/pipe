// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PipeTime.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Time.DateTime", []() {
		it("Can get day of year", [&]() {
			DateTime time1{2024, 1, 1};
			AssertThat(time1.GetDayOfYear(), Equals(1));
			DateTime time11{2024, 1, 30};
			AssertThat(time11.GetDayOfYear(), Equals(30));
			DateTime time12{2024, 1, 31};
			AssertThat(time12.GetDayOfYear(), Equals(31));

			DateTime time2{2024, 2, 1};
			AssertThat(time2.GetDayOfYear(), Equals(32));
			DateTime time3{2024, 3, 1};
			AssertThat(time3.GetDayOfYear(), Equals(60));
			DateTime time4{2024, 12, 31};
			AssertThat(time4.GetDayOfYear(), Equals(365));
		});
	});
});
