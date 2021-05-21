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

			ReadContext& ct = reader;
			ct.BeginObject();
			String name;
			ct.Next("name", name);

			AssertThat(name.data(), Equals("Miguel"));
		});

		it("Can read from array values", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader;
			ct.BeginObject();
			if (ct.EnterNext("players"))
			{
				u32 size;
				ct.BeginArray(size);
				String name;
				ct.Next(name);
				AssertThat(name.data(), Equals("Miguel"));

				ct.Next(name);
				AssertThat(name.data(), Equals("Juan"));

				ct.Leave();
			}
		});

		it("Can iterate arrays", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader;
			ct.BeginObject();
			if (ct.EnterNext("players"))
			{
				static const StringView expected[]{"Miguel", "Juan"};
				u32 size;
				ct.BeginArray(size);
				for (u32 i = 0; i < size; ++i)
				{
					StringView name;
					ct.Next(name);
					AssertThat(name, Equals(expected[i]));
				}
				ct.Leave();
			}
		});

		it("Can check types", [&]() {
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader;
			AssertThat(reader.IsObject(), Equals(true));
			ct.BeginObject();
			if (ct.EnterNext("players"))
			{
				AssertThat(reader.IsArray(), Equals(true));
				ct.Leave();
			}
		});

		it("Can find multiple keys", [&]() {
			String data{"{\"one\": \"Miguel\", \"other\": \"Juan\"}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader;
			AssertThat(reader.IsObject(), Equals(true));
			ct.BeginObject();
			StringView name;
			ct.Next("one", name);
			AssertThat(name, Equals("Miguel"));

			ct.Next("other", name);
			AssertThat(name, Equals("Juan"));
		});

		it("Can find multiple unordered keys", [&]() {
			String data{"{\"one\": \"Miguel\", \"other\": \"Juan\"}"};
			JsonFormatReader reader{data};

			ReadContext& ct = reader;
			AssertThat(reader.IsObject(), Equals(true));
			ct.BeginObject();
			StringView name;
			ct.Next("other", name);
			AssertThat(name, Equals("Juan"));

			ct.Next("one", name);
			AssertThat(name, Equals("Miguel"));
		});

		describe("Types", []() {
			it("Can read bool values", [&]() {
				JsonFormatReader reader{"{\"alive\": true}"};
				ReadContext& ct = reader;
				ct.BeginObject();
				bool value = false;
				ct.Next("alive", value);
				AssertThat(value, Equals(true));

				JsonFormatReader reader2{"{\"alive\": false}"};
				ct = reader2;
				ct.BeginObject();
				bool value2 = true;
				ct.Next("alive", value2);
				AssertThat(value2, Equals(false));
			});

			it("Can read u8 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 3}"};
				ReadContext& ct = reader;
				ct.BeginObject();
				u8 value = 0;
				ct.Next("alive", value);
				AssertThat(value, Equals(3));

				JsonFormatReader reader2{"{\"alive\": 1.344}"};
				ct = reader2;
				ct.BeginObject();
				u8 value2 = 0;
				ct.Next("alive", value2);
				AssertThat(value2, Equals(1));
			});

			it("Can read u32 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 35533}"};
				ReadContext ct = reader;
				ct.BeginObject();
				u32 value = 0;
				ct.Next("alive", value);
				AssertThat(value, Equals(35533));


				JsonFormatReader reader2{"{\"alive\": -5}"};

				ct = reader2;
				ct.BeginObject();
				u32 value2 = 0;
				ct.Next("alive", value2);
				AssertThat(value2, Equals(0));
			});

			it("Can read i32 values", [&]() {
				JsonFormatReader reader{"{\"alive\": 35533}"};
				ReadContext ct = reader;
				ct.BeginObject();
				i32 value = 0;
				ct.Next("alive", value);
				AssertThat(value, Equals(35533));

				JsonFormatReader reader2{"{\"alive\": -35533}"};
				ct = reader2;
				ct.BeginObject();
				i32 value2 = 0;
				ct.Next("alive", value2);
				AssertThat(value2, Equals(-35533));
			});

			it("Can read float values", [&]() {
				JsonFormatReader reader{"{\"alive\": 0.344}"};
				ReadContext& ct = reader;
				ct.BeginObject();
				float value = 0.f;
				ct.Next("alive", value);
				AssertThat(value, Equals(0.344f));

				JsonFormatReader reader2{"{\"alive\": 4}"};
				ct = reader2;
				ct.BeginObject();
				float value2 = 0.f;
				ct.Next("alive", value2);
				AssertThat(value2, Equals(4.f));
			});

			it("Can read StringView values", [&]() {
				JsonFormatReader reader{"{\"alive\": \"yes\"}"};
				ReadContext& ct = reader;
				ct.BeginObject();
				StringView value;
				ct.Next("alive", value);
				AssertThat(value, Equals("yes"));
			});
		});
	});
});
