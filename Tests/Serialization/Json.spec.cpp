// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeSerialize.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Serialization.Json", []() {
		describe("Reader", [&]() {
			it("Can create a reader", [&]() {
				JsonFormatReader reader{""};
				AssertThat(reader.IsValid(), Equals(false));

				JsonFormatReader reader2{"{}"};
				AssertThat(reader2.IsValid(), Equals(true));
			});

			it("Can read from object value", [&]() {
				String data{"{\"name\": \"Miguel\"}"};
				JsonFormatReader reader{data};

				Reader& ct = reader;
				ct.BeginObject();
				String name;
				ct.Next("name", name);

				AssertThat(name.data(), Equals("Miguel"));
			});

			it("Can read from array values", [&]() {
				String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
				JsonFormatReader reader{data};

				Reader& ct = reader;
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

				Reader& ct = reader;
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

				Reader& ct = reader;
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

				Reader& ct = reader;
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

				Reader& ct = reader;
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
					Reader& ct = reader;
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

				it("Can read i8 values", [&]() {
					JsonFormatReader reader{"{\"alive\": -3}"};
					Reader& ct = reader;
					ct.BeginObject();
					i8 value = 0;
					ct.Next("alive", value);
					AssertThat(value, Equals(-3));

					JsonFormatReader reader2{"{\"alive\": -1.344}"};
					ct = reader2;
					ct.BeginObject();
					i8 value2 = 0;
					ct.Next("alive", value2);
					AssertThat(value2, Equals(-1));
				});

				it("Can read u8 values", [&]() {
					JsonFormatReader reader{"{\"alive\": 3}"};
					Reader& ct = reader;
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

				it("Can read i16 values", [&]() {
					// Test inbounds and out of bounds values
					JsonFormatReader reader{Strings::Format(
					    "{{\"a\":{},\"b\":{},\"c\":{},\"d\":{}}}", Limits<i16>::Max(),
					    Limits<i16>::Lowest(), Limits<i32>::Max(), Limits<i32>::Lowest())};
					Reader ct = reader;
					ct.BeginObject();
					i16 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(Limits<i16>::Max()));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<i16>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(Limits<i16>::Max()));
					ct.Next("d", value);
					AssertThat(value, Equals(Limits<i16>::Lowest()));
				});

				it("Can read u16 values", [&]() {
					JsonFormatReader reader{Strings::Format("{{\"a\":{},\"b\":{},\"c\":{}}}",
					    Limits<u16>::Max(), Limits<u16>::Lowest(), -32)};
					Reader ct = reader;
					ct.BeginObject();
					u16 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(Limits<u16>::Max()));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<u16>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(0));
				});

				it("Can read i32 values", [&]() {
					// Test inbounds and out of bounds values
					JsonFormatReader reader{Strings::Format(
					    "{{\"a\":{},\"b\":{},\"c\":{},\"d\":{}}}", Limits<i32>::Max(),
					    Limits<i32>::Lowest(), Limits<i64>::Max(), Limits<i64>::Lowest())};
					Reader ct = reader;
					ct.BeginObject();
					i32 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(Limits<i32>::Max()));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<i32>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(Limits<i32>::Max()));
					ct.Next("d", value);
					AssertThat(value, Equals(Limits<i32>::Lowest()));
				});

				it("Can read u32 values", [&]() {
					JsonFormatReader reader{Strings::Format("{{\"a\":{},\"b\":{},\"c\":{}}}",
					    Limits<u32>::Max(), Limits<u32>::Lowest(), -32)};
					Reader ct = reader;
					ct.BeginObject();
					u32 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(Limits<u32>::Max()));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<u32>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(0));
				});

				it("Can read float values", [&]() {
					JsonFormatReader reader{"{\"alive\": 0.344}"};
					Reader& ct = reader;
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
					Reader& ct = reader;
					ct.BeginObject();
					StringView value;
					ct.Next("alive", value);
					AssertThat(value, Equals("yes"));
				});
			});
		});

		describe("Writer", [&]() {
			it("Can create a writer", [&]() {
				JsonFormatWriter writer{};
				AssertThat(writer.IsValid(), Equals(true));
			});

			it("Can write to object key", [&]() {
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("name", StringView{"Miguel"});
				AssertThat(writer.ToString(false), Equals("{\"name\":\"Miguel\"}"));
			});

			it("Can write arrays", [&]() {
				JsonFormatWriter writer{};

				Writer& ct = writer;
				ct.BeginObject();
				if (ct.EnterNext("players"))
				{
					static const StringView expected[]{"Miguel", "Juan"};
					u32 size = 2;
					ct.BeginArray(size);
					for (u32 i = 0; i < size; ++i)
					{
						ct.Next(expected[i]);
					}
					ct.Leave();
				}
				AssertThat(writer.ToString(false), Equals("{\"players\":[\"Miguel\",\"Juan\"]}"));
			});

			it("Can write multiple object keys", [&]() {
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("one", StringView{"Miguel"});
				ct.Next("other", StringView{"Juan"});
				AssertThat(
				    writer.ToString(false), Equals("{\"one\":\"Miguel\",\"other\":\"Juan\"}"));
			});

			describe("Types", []() {
				it("Can write bool values", [&]() {
					JsonFormatWriter writer{};
					Writer& ct = writer;
					ct.BeginObject();
					ct.Next("alive", true);
					AssertThat(writer.ToString(false), Equals("{\"alive\":true}"));

					JsonFormatWriter writer2{};
					ct = writer2;
					ct.BeginObject();
					ct.Next("alive", false);
					AssertThat(writer2.ToString(false), Equals("{\"alive\":false}"));
				});

				it("Can write i8 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("alive", i8(-3));
					AssertThat(writer.ToString(false), Equals("{\"alive\":-3}"));
				});

				it("Can write u8 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("alive", u8(3));
					AssertThat(writer.ToString(false), Equals("{\"alive\":3}"));
				});

				it("Can write i16 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", i16(-3000));
					ct.Next("b", Limits<i16>::Max());
					ct.Next("c", Limits<i16>::Lowest());
					AssertThat(
					    writer.ToString(false), Equals("{\"a\":-3000,\"b\":32767,\"c\":-32768}"));
				});

				it("Can write u16 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", u16(3000));
					ct.Next("b", Limits<u16>::Max());
					ct.Next("c", Limits<u16>::Lowest());
					AssertThat(writer.ToString(false), Equals("{\"a\":3000,\"b\":65535,\"c\":0}"));
				});

				it("Can write u32 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("alive", u32(35533));
					AssertThat(writer.ToString(false), Equals("{\"alive\":35533}"));
				});

				it("Can write i32 values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					i32 value = 0;
					ct.Next("alive", u32(35533));
					AssertThat(writer.ToString(false), Equals("{\"alive\":35533}"));

					JsonFormatWriter writer2{};
					ct = writer2;
					ct.BeginObject();
					ct.Next("alive", i32(-35533));
					AssertThat(writer2.ToString(false), Equals("{\"alive\":-35533}"));
				});

				it("Can write float values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("alive", 0.344f);
					AssertThat(Strings::Contains(writer.ToString(false), "0.344"), Equals(true));

					JsonFormatWriter writer2{};
					ct = writer2;
					ct.BeginObject();
					ct.Next("alive", 4.f);
					AssertThat(writer2.ToString(false), Equals("{\"alive\":4.0}"));
				});

				it("Can write StringView values", [&]() {
					JsonFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("alive", StringView{"yes"});
					AssertThat(writer.ToString(false), Equals("{\"alive\":\"yes\"}"));
				});
			});
		});
	});
});
