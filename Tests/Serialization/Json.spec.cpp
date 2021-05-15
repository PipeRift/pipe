// Copyright 2015-2021 Piperift - All rights reserved

#include <Serialization/Formats/JsonFormat.h>
#include <Serialization/ReadContext.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Serl;


go_bandit([]() {
	describe("Serialization.Json", []() {
		it("Can create a reader", [&]() {
			String data0{""};
			JsonFormatReader reader{data0};

			AssertThat(reader.IsValid(), Equals(true));
		});

		it("Can read from scope value", [&]() {
			String data{"{\"name\": \"Miguel\"}"};
			JsonFormatReader reader{data};

			String name;
			ReadScope(reader, "name", name);

			AssertThat(name.data(), Equals("Miguel"));
		});

		it("Can read from array values", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader.GetContext();

			String name;
			EnterScope(ct, "players");
			{
				EnterScope(ct, 0);
				Read(ct, name);
				LeaveScope(ct);
				AssertThat(name.data(), Equals("Miguel"));

				EnterScope(ct, 1);
				Read(ct, name);
				LeaveScope(ct);
				AssertThat(name.data(), Equals("Juan"));
			}
			LeaveScope(ct);
		});
	});
});
