// Copyright 2015-2021 Piperift - All rights reserved

#include <Context.h>
#include <Serialization/Formats/JsonFormat.h>
#include <Serialization/ReadContext.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Serl;


go_bandit([]() {
	describe("Serialization.Json", []() {
		before_each([]() {
			InitializeContext();
		});

		it("Can create a reader", [&]() {
			JsonFormatReader reader{""};
			AssertThat(reader.IsValid(), Equals(false));

			JsonFormatReader reader2{"{}"};
			AssertThat(reader2.IsValid(), Equals(true));
		});

		it("Can read from object value", [&]() {
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

			EnterScope(ct, "players");
			{
				String name;
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

		it("Can iterate arrays", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader.GetContext();

			if (EnterScope(ct, "players"))
			{
				static const StringView expected[]{"Miguel", "Juan"};

				IterateArray(ct, [&ct](u32 index) {
					StringView name;
					Read(ct, name);

					AssertThat(name, Equals(expected[index]));
				});
				LeaveScope(ct);
			}
		});

		it("Can check types", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader.GetContext();

			AssertThat(IsObject(ct), Equals(true));
			EnterScope(ct, "players");
			AssertThat(IsArray(ct), Equals(true));
			LeaveScope(ct);
		});

		describe("Types", []() {
			it("Can read bool values", [&]() {
				JsonFormatReader reader{"{\"alive\": true}"};
				bool value = false;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals(true));

				JsonFormatReader reader2{"{\"alive\": false}"};
				bool value2 = false;
				ReadScope(reader2, "alive", value2);
				AssertThat(value2, Equals(false));
			});

			it("Can read u8 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 3}"};
				u8 value = 0;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals(3));

				JsonFormatReader reader2{"{\"alive\": 1.344}"};
				u8 value2 = 0;
				ReadScope(reader2, "alive", value2);
				AssertThat(value2, Equals(1));
			});

			it("Can read u32 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 35533}"};
				u32 value = 0;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals(35533));


				JsonFormatReader reader2{"{\"alive\": -5}"};
				u32 value2 = 0;
				ReadScope(reader2, "alive", value2);
				AssertThat(value2, Equals(0));
			});

			it("Can read i32 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 35533}"};
				i32 value = 0;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals(35533));

				JsonFormatReader reader2{"{\"alive\": -35533}"};
				i32 value2 = 0;
				ReadScope(reader2, "alive", value2);
				AssertThat(value2, Equals(-35533));
			});

			it("Can read float values", [&]() {
				JsonFormatReader reader{"{\"alive\": 0.344}"};
				float value = 0.f;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals(0.344f));

				JsonFormatReader reader2{"{\"alive\": 4}"};
				float value2 = 0.f;
				ReadScope(reader2, "alive", value2);
				AssertThat(value2, Equals(4.f));
			});

			it("Can read StringView values", [&]() {
				JsonFormatReader reader{"{\"alive\": \"yes\"}"};
				StringView value;
				ReadScope(reader, "alive", value);
				AssertThat(value, Equals("yes"));
			});
		});
	});
});
