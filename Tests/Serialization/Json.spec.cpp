// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeSerialize.h>


using namespace p;


Spec("Serialization.Json", []()
{
Describe("Reader", []()
{
	It("Can create a reader", []()
	{
		JsonFormatReader reader{"{}"};
		Expect(reader.IsValid()).ToBeTrue();
	});

	It("Can read from object value", []()
	{
		String data{"{\"name\": \"Miguel\"}"};
		JsonFormatReader reader{data};

		Reader& ct = reader;
		ct.BeginObject();
		String name;
		ct.Next("name", name);

		Expect(name.data()).ToEqual("Miguel");
	});

	It("Can read from array values", []()
	{
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
			Expect(name.data()).ToEqual("Miguel");

			ct.Next(name);
			Expect(name.data()).ToEqual("Juan");

			ct.Leave();
		}
	});

	It("Can iterate arrays", []()
	{
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
				Expect(name).ToEqual(expected[i]);
			}
			ct.Leave();
		}
	});

	It("Can check types", []()
	{
		String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
		JsonFormatReader reader{data};

		Reader& ct = reader;
		Expect(reader.IsObject()).ToEqual(true);
		ct.BeginObject();
		if (ct.EnterNext("players"))
		{
			Expect(reader.IsArray()).ToEqual(true);
			ct.Leave();
		}
	});

	It("Can find multiple keys", []()
	{
		String data{"{\"one\": \"Miguel\", \"other\": \"Juan\"}"};
		JsonFormatReader reader{data};

		Reader& ct = reader;
		Expect(reader.IsObject()).ToEqual(true);
		ct.BeginObject();
		StringView name;
		ct.Next("one", name);
		Expect(name).ToEqual("Miguel");

		ct.Next("other", name);
		Expect(name).ToEqual("Juan");
	});

	It("Can find multiple unordered keys", []()
	{
		String data{"{\"one\": \"Miguel\", \"other\": \"Juan\"}"};
		JsonFormatReader reader{data};

		Reader& ct = reader;
		Expect(reader.IsObject()).ToEqual(true);
		ct.BeginObject();
		StringView name;
		ct.Next("other", name);
		Expect(name).ToEqual("Juan");

		ct.Next("one", name);
		Expect(name).ToEqual("Miguel");
	});

	Describe("Types", []()
	{
		It("Can read bool values", []()
		{
			JsonFormatReader reader{"{\"alive\": true}"};
			Reader& ct = reader;
			ct.BeginObject();
			bool value = false;
			ct.Next("alive", value);
			Expect(value).ToEqual(true);

			JsonFormatReader reader2{"{\"alive\": false}"};
			ct = reader2;
			ct.BeginObject();
			bool value2 = true;
			ct.Next("alive", value2);
			Expect(value2).ToEqual(false);
		});

		It("Can read i8 values", []()
		{
			JsonFormatReader reader{"{\"alive\": -3}"};
			Reader& ct = reader;
			ct.BeginObject();
			i8 value = 0;
			ct.Next("alive", value);
			Expect(value).ToEqual(-3);

			JsonFormatReader reader2{"{\"alive\": -1.344}"};
			ct = reader2;
			ct.BeginObject();
			i8 value2 = 0;
			ct.Next("alive", value2);
			Expect(value2).ToEqual(-1);
		});

		It("Can read u8 values", []()
		{
			JsonFormatReader reader{"{\"alive\": 3}"};
			Reader& ct = reader;
			ct.BeginObject();
			u8 value = 0;
			ct.Next("alive", value);
			Expect(value).ToEqual(3);

			JsonFormatReader reader2{"{\"alive\": 1.344}"};
			ct = reader2;
			ct.BeginObject();
			u8 value2 = 0;
			ct.Next("alive", value2);
			Expect(value2).ToEqual(1);
		});

		It("Can read i16 values", []()
		{
			// Test inbounds and out of bounds values
			JsonFormatReader reader{
			    Format("{{\"a\":{},\"b\":{},\"c\":{},\"d\":{}}}", Limits<i16>::Max(),
			        Limits<i16>::Lowest(), Limits<i32>::Max(), Limits<i32>::Lowest())};
			Reader ct = reader;
			ct.BeginObject();
			i16 value = 0;
			ct.Next("a", value);
			Expect(value).ToEqual(Limits<i16>::Max());
			ct.Next("b", value);
			Expect(value).ToEqual(Limits<i16>::Lowest());
			ct.Next("c", value);
			Expect(value).ToEqual(Limits<i16>::Max());
			ct.Next("d", value);
			Expect(value).ToEqual(Limits<i16>::Lowest());
		});

		It("Can read u16 values", []()
		{
			JsonFormatReader reader{Format("{{\"a\":{},\"b\":{},\"c\":{}}}",
			    Limits<u16>::Max(), Limits<u16>::Lowest(), -32)};
			Reader ct = reader;
			ct.BeginObject();
			u16 value = 0;
			ct.Next("a", value);
			Expect(value).ToEqual(Limits<u16>::Max());
			ct.Next("b", value);
			Expect(value).ToEqual(Limits<u16>::Lowest());
			ct.Next("c", value);
			Expect(value).ToEqual(0);
		});

		It("Can read i32 values", []()
		{
			// Test inbounds and out of bounds values
			JsonFormatReader reader{
			    Format("{{\"a\":{},\"b\":{},\"c\":{},\"d\":{}}}", Limits<i32>::Max(),
			        Limits<i32>::Lowest(), Limits<i64>::Max(), Limits<i64>::Lowest())};
			Reader ct = reader;
			ct.BeginObject();
			i32 value = 0;
			ct.Next("a", value);
			Expect(value).ToEqual(Limits<i32>::Max());
			ct.Next("b", value);
			Expect(value).ToEqual(Limits<i32>::Lowest());
			ct.Next("c", value);
			Expect(value).ToEqual(Limits<i32>::Max());
			ct.Next("d", value);
			Expect(value).ToEqual(Limits<i32>::Lowest());
		});

		It("Can read u32 values", []()
		{
			JsonFormatReader reader{Format("{{\"a\":{},\"b\":{},\"c\":{}}}",
			    Limits<u32>::Max(), Limits<u32>::Lowest(), -32)};
			Reader ct = reader;
			ct.BeginObject();
			u32 value = 0;
			ct.Next("a", value);
			Expect(value).ToEqual(Limits<u32>::Max());
			ct.Next("b", value);
			Expect(value).ToEqual(Limits<u32>::Lowest());
			ct.Next("c", value);
			Expect(value).ToEqual(0);
		});

		It("Can read float values", []()
		{
			JsonFormatReader reader{"{\"alive\": 0.344}"};
			Reader& ct = reader;
			ct.BeginObject();
			float value = 0.f;
			ct.Next("alive", value);
			Expect(value).ToEqual(0.344f);

			JsonFormatReader reader2{"{\"alive\": 4}"};
			ct = reader2;
			ct.BeginObject();
			float value2 = 0.f;
			ct.Next("alive", value2);
			Expect(value2).ToEqual(4.f);
		});

		It("Can read StringView values", []()
		{
			JsonFormatReader reader{"{\"alive\": \"yes\"}"};
			Reader& ct = reader;
			ct.BeginObject();
			StringView value;
			ct.Next("alive", value);
			Expect(value).ToEqual("yes");
		});
	});
});

Describe("Writer", []()
{
	It("Can create a writer", []()
	{
		JsonFormatWriter writer{};
		Expect(writer.IsValid()).ToEqual(true);
	});

	It("Can write to object key", []()
	{
		JsonFormatWriter writer{};
		Writer& ct = writer;
		ct.BeginObject();
		ct.Next("name", StringView{"Miguel"});
		Expect(writer.ToString(false)).ToEqual("{\"name\":\"Miguel\"}");
	});

	It("Can write arrays", []()
	{
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
		Expect(writer.ToString(false)).ToEqual("{\"players\":[\"Miguel\",\"Juan\"]}");
	});

	It("Can write multiple object keys", []()
	{
		JsonFormatWriter writer{};
		Writer& ct = writer;
		ct.BeginObject();
		ct.Next("one", StringView{"Miguel"});
		ct.Next("other", StringView{"Juan"});
		Expect(
		    writer.ToString(false)).ToEqual("{\"one\":\"Miguel\",\"other\":\"Juan\"}");
	});

	Describe("Types", []()
	{
		It("Can write bool values", []()
		{
			JsonFormatWriter writer{};
			Writer& ct = writer;
			ct.BeginObject();
			ct.Next("alive", true);
			Expect(writer.ToString(false)).ToEqual("{\"alive\":true}");

			JsonFormatWriter writer2{};
			ct = writer2;
			ct.BeginObject();
			ct.Next("alive", false);
			Expect(writer2.ToString(false)).ToEqual("{\"alive\":false}");
		});

		It("Can write i8 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("alive", i8(-3));
			Expect(writer.ToString(false)).ToEqual("{\"alive\":-3}");
		});

		It("Can write u8 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("alive", u8(3));
			Expect(writer.ToString(false)).ToEqual("{\"alive\":3}");
		});

		It("Can write i16 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("a", i16(-3000));
			ct.Next("b", Limits<i16>::Max());
			ct.Next("c", Limits<i16>::Lowest());
			Expect(
			    writer.ToString(false)).ToEqual("{\"a\":-3000,\"b\":32767,\"c\":-32768}");
		});

		It("Can write u16 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("a", u16(3000));
			ct.Next("b", Limits<u16>::Max());
			ct.Next("c", Limits<u16>::Lowest());
			Expect(writer.ToString(false)).ToEqual("{\"a\":3000,\"b\":65535,\"c\":0}");
		});

		It("Can write u32 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("alive", u32(35533));
			Expect(writer.ToString(false)).ToEqual("{\"alive\":35533}");
		});

		It("Can write i32 values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			i32 value = 0;
			ct.Next("alive", u32(35533));
			Expect(writer.ToString(false)).ToEqual("{\"alive\":35533}");

			JsonFormatWriter writer2{};
			ct = writer2;
			ct.BeginObject();
			ct.Next("alive", i32(-35533));
			Expect(writer2.ToString(false)).ToEqual("{\"alive\":-35533}");
		});

		It("Can write float values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("alive", 0.344f);
			Expect(Strings::Contains(writer.ToString(false), "0.344")).ToEqual(true);

			JsonFormatWriter writer2{};
			ct = writer2;
			ct.BeginObject();
			ct.Next("alive", 4.f);
			Expect(writer2.ToString(false)).ToEqual("{\"alive\":4.0}");
		});

		It("Can write StringView values", []()
		{
			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("alive", StringView{"yes"});
			Expect(writer.ToString(false)).ToEqual("{\"alive\":\"yes\"}");
		});
	});
});
});
