// Copyright 2015-2021 Piperift - All rights reserved

#include <Serialization/Format_Json.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Serl;


go_bandit([]() {
	describe("Serialization.Json", []() {
		it("Can create a reader", [&]() {
			String data0{""};
			FormatReader_Json reader{data0};
		});
	});
});
