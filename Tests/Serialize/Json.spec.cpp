// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeSerialize.h>
#include <PipeTest.h>


using namespace p;


P_SPEC("Serialization.Json", []()
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
			if (ct.Enter("players"))
			{
				i32 size = 0;
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
			if (ct.Enter("players"))
			{
				static const StringView expected[]{"Miguel", "Juan"};
				i32 size = 0;
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

		It("Enters the next key of an object without a name", []()
		{
			String data{"{\"a\": 1, \"b\": 2}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			ct.BeginObject();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "a").ToBeTrue();
			u8 value = 0;
			ct.Serialize(value);
			Expect(value).ToEqual(1);
			ct.Leave();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "b").ToBeTrue();
			ct.Serialize(value);
			Expect(value).ToEqual(2);
			ct.Leave();

			Expect(ct.EnterNext()).ToBeFalse();
		});

		It("Enters the next object member whatever its key", []()
		{
			// The first value is a container, which spans more than one value slot
			String data{"{\"one\": {\"x\": 1}, \"two\": 2, \"three\": 3}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			ct.BeginObject();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "one").ToBeTrue();
			ct.Leave();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "two").ToBeTrue();
			ct.Leave();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "three").ToBeTrue();
			ct.Leave();

			Expect(ct.EnterNext()).ToBeFalse();
		});

		It("Does not leak a name search into the parent object", []()
		{
			// "y" lives in the outer object, but the inner object's pool is contiguous
			String data{"{\"inner\": {\"x\": 1}, \"y\": 2}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			ct.BeginObject();
			Expect(ct.Enter("inner")).ToBeTrue();
			ct.BeginObject();

			Expect(ct.Enter("y")).ToBeFalse();    // Not a member of the inner object
			Expect(ct.Enter("x")).ToBeTrue();
			u8 value = 0;
			ct.Serialize(value);
			Expect(value).ToEqual(1);

			ct.Leave();
			ct.Leave();
			Expect(ct.Enter("y")).ToBeTrue();
			ct.Serialize(value);
			Expect(value).ToEqual(2);
			ct.Leave();
		});

		It("Includes the current member when wrapping a name search", []()
		{
			String data{"{\"a\": 1, \"b\": 2}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			ct.BeginObject();
			Expect(ct.Enter("b")).ToBeTrue();
			ct.Leave();

			// A full loop reaches the member we came from
			Expect(ct.Enter("b")).ToBeTrue();
			Expect(ct.GetCurrentKey() == "b").ToBeTrue();
			ct.Leave();
		});

		It("Reports the current member from the document", []()
		{
			String data{"{\"a\": 1, \"b\": 2}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			ct.BeginObject();

			u8 value = 0;
			Expect(ct.Enter("b")).ToBeTrue();
			Expect(ct.GetCurrentKey() == "b").ToBeTrue();
			Expect(ct.GetCurrentIndex()).ToEqual(1);
			ct.Serialize(value);
			Expect(value).ToEqual(2);
			ct.Leave();

			// Wraps around, and the index is the member's position
			Expect(ct.Enter("a")).ToBeTrue();
			Expect(ct.GetCurrentKey() == "a").ToBeTrue();
			Expect(ct.GetCurrentIndex()).ToEqual(0);
			ct.Serialize(value);
			Expect(value).ToEqual(1);
			ct.Leave();
		});

		It("Enters array elements by index", []()
		{
			String data{"[7, 8, 9]"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			i32 size = 0;
			ct.BeginArray(size);
			Expect(size).ToEqual(3u);

			u8 value = 0;
			Expect(ct.Enter(2)).ToBeTrue();
			Expect(ct.GetCurrentIndex()).ToEqual(2);
			ct.Serialize(value);
			Expect(value).ToEqual(9);
			ct.Leave();

			Expect(ct.Enter(0)).ToBeTrue();
			Expect(ct.GetCurrentIndex()).ToEqual(0);
			ct.Serialize(value);
			Expect(value).ToEqual(7);
			ct.Leave();

			Expect(ct.Enter(3)).ToBeFalse();
		});

		It("Can check types", []()
		{
			String data{"{\"players\": [\"Miguel\", \"Juan\"]}"};
			JsonFormatReader reader{data};

			Reader& ct = reader;
			Expect(reader.IsObject()).ToEqual(true);
			ct.BeginObject();
			if (ct.Enter("players"))
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
				Reader& ct2 = reader2;
				ct2.BeginObject();
				bool value2 = true;
				ct2.Next("alive", value2);
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
				Reader& ct2 = reader2;
				ct2.BeginObject();
				i8 value2 = 0;
				ct2.Next("alive", value2);
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
				Reader& ct2 = reader2;
				ct2.BeginObject();
				u8 value2 = 0;
				ct2.Next("alive", value2);
				Expect(value2).ToEqual(1);
			});

			It("Can read i16 values", []()
			{
				// Test inbounds and out of bounds values
				JsonFormatReader reader{
				    Format("{{\"a\":{},\"b\":{},\"c\":{},\"d\":{}}}", Limits<i16>::Max(),
				        Limits<i16>::Lowest(), Limits<i32>::Max(), Limits<i32>::Lowest())};
				Reader& ct = reader;
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
				JsonFormatReader reader{Format("{{\"a\":{},\"b\":{},\"c\":{}}}", Limits<u16>::Max(),
				    Limits<u16>::Lowest(), -32)};
				Reader& ct = reader;
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
				Reader& ct = reader;
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
				JsonFormatReader reader{Format("{{\"a\":{},\"b\":{},\"c\":{}}}", Limits<u32>::Max(),
				    Limits<u32>::Lowest(), -32)};
				Reader& ct = reader;
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
				Reader& ct2 = reader2;
				ct2.BeginObject();
				float value2 = 0.f;
				ct2.Next("alive", value2);
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

		It("Replaces existing values with Enter and appends with EnterNext", []()
		{
			JsonFormatWriter writer{};
			Writer& out = writer;
			out.BeginObject();
			if (out.EnterNext("a"))
			{
				out.Serialize(u8(1));
				out.Leave();
			}
			if (out.EnterNext("b"))
			{
				out.Serialize(u8(2));
				out.Leave();
			}

			// Enter modifies the existing member
			if (out.Enter("a"))
			{
				out.Serialize(u8(5));
				out.Leave();
			}
			Expect(writer.ToString(false)).ToEqual("{\"a\":5,\"b\":2}");
		});

		It("Replaces and appends array elements by index", []()
		{
			JsonFormatWriter writer{};
			Writer& out = writer;
			i32 size = 2;
			out.BeginArray(size);
			out.Next(u8(1));
			out.Next(u8(2));

			// Existing elements are replaced
			if (out.Enter(0))
			{
				out.Serialize(u8(9));
				out.Leave();
			}
			// Entering the size appends
			if (out.Enter(2))
			{
				out.Serialize(u8(3));
				out.Leave();
			}
			Expect(writer.ToString(false)).ToEqual("[9,2,3]");
		});

		It("Writes empty object keys and unnamed array elements", []()
		{
			JsonFormatWriter writer{};
			Writer& out = writer;
			out.BeginObject();

			StringView empty{""};
			Expect(out.EnterNext({})).ToBeFalse();    // Null key: member name is unknown
			if (out.EnterNext(empty))                 // "" is a valid key
			{
				out.Serialize(u8(1));
				out.Leave();
			}
			if (out.EnterNext("array"))
			{
				out.BeginArray(1);
				Expect(out.EnterNext()).ToBeTrue();    // Null key: array element
				out.Serialize(u8(3));
				out.Leave();
			}
			out.Leave();
			Expect(writer.ToString(false)).ToEqual("{\"\":1,\"array\":[3]}");

			// The reader finds the empty key by name
			JsonFormatReader reader{writer.ToString(false)};
			Reader& in = reader;
			in.BeginObject();
			Expect(in.Enter("")).ToBeTrue();
			Expect(in.GetCurrentKey().empty()).ToBeTrue();
			u8 value = 0;
			in.Serialize(value);
			Expect(value).ToEqual(1);
			in.Leave();
		});

		It("Reports the entered key through GetCurrentKey", []()
		{
			String data{"{\"a\": 1, \"b\": 2}"};
			JsonFormatReader reader{data};
			Reader& ct = reader;
			ct.BeginObject();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "a").ToBeTrue();
			ct.Leave();

			Expect(ct.EnterNext()).ToBeTrue();
			Expect(ct.GetCurrentKey() == "b").ToBeTrue();
			ct.Leave();

			Expect(ct.EnterNext()).ToBeFalse();    // End of the object
		});

		It("Fails to enter when the next key does not match", []()
		{
			String data{"{\"a\": 1, \"b\": 2}"};
			JsonFormatReader reader{data};
			Reader& ct = reader;
			ct.BeginObject();

			Expect(ct.EnterNext("b")).ToBeFalse();    // Next is "a"

			Expect(ct.EnterNext("a")).ToBeTrue();    // "a" was not consumed
			Expect(ct.GetCurrentKey() == "a").ToBeTrue();
			u8 value = 0;
			ct.Serialize(value);
			Expect(value).ToEqual(1);
			ct.Leave();

			Expect(ct.EnterNext("b")).ToBeTrue();
			Expect(ct.GetCurrentKey() == "b").ToBeTrue();
			ct.Serialize(value);
			Expect(value).ToEqual(2);
			ct.Leave();
		});

		It("Treats an empty key as a key, not as no key", []()
		{
			String data{"{\"a\": 1, \"\": 2}"};
			JsonFormatReader reader{data};
			Reader& ct = reader;
			ct.BeginObject();

			Expect(ct.EnterNext()).ToBeTrue();    // Positional: enters "a"
			Expect(ct.GetCurrentIndex()).ToEqual(0);
			ct.Leave();

			Expect(ct.Enter("")).ToBeTrue();    // Searches the empty key
			Expect(ct.GetCurrentKey().empty()).ToBeTrue();
			Expect(ct.GetCurrentIndex()).ToEqual(1);
			u8 value = 0;
			ct.Serialize(value);
			Expect(value).ToEqual(2);
			ct.Leave();
		});

		It("Can write arrays", []()
		{
			JsonFormatWriter writer{};

			Writer& ct = writer;
			ct.BeginObject();
			if (ct.EnterNext("players"))
			{
				static const StringView expected[]{"Miguel", "Juan"};
				i32 size = 2;
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
			Expect(writer.ToString(false)).ToEqual("{\"one\":\"Miguel\",\"other\":\"Juan\"}");
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
				Writer& ct2 = writer2;
				ct2.BeginObject();
				ct2.Next("alive", false);
				Expect(writer2.ToString(false)).ToEqual("{\"alive\":false}");
			});

			It("Can write i8 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("alive", i8(-3));
				Expect(writer.ToString(false)).ToEqual("{\"alive\":-3}");
			});

			It("Can write u8 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("alive", u8(3));
				Expect(writer.ToString(false)).ToEqual("{\"alive\":3}");
			});

			It("Can write i16 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("a", i16(-3000));
				ct.Next("b", Limits<i16>::Max());
				ct.Next("c", Limits<i16>::Lowest());
				Expect(writer.ToString(false)).ToEqual("{\"a\":-3000,\"b\":32767,\"c\":-32768}");
			});

			It("Can write u16 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("a", u16(3000));
				ct.Next("b", Limits<u16>::Max());
				ct.Next("c", Limits<u16>::Lowest());
				Expect(writer.ToString(false)).ToEqual("{\"a\":3000,\"b\":65535,\"c\":0}");
			});

			It("Can write u32 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("alive", u32(35533));
				Expect(writer.ToString(false)).ToEqual("{\"alive\":35533}");
			});

			It("Can write i32 values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				i32 value = 0;
				ct.Next("alive", u32(35533));
				Expect(writer.ToString(false)).ToEqual("{\"alive\":35533}");

				JsonFormatWriter writer2{};
				Writer& ct2 = writer2;
				ct2.BeginObject();
				ct2.Next("alive", i32(-35533));
				Expect(writer2.ToString(false)).ToEqual("{\"alive\":-35533}");
			});

			It("Can write float values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("alive", 0.344f);
				Expect(Strings::Contains(writer.ToString(false), "0.344")).ToEqual(true);

				JsonFormatWriter writer2{};
				Writer& ct2 = writer2;
				ct2.BeginObject();
				ct2.Next("alive", 4.f);
				Expect(writer2.ToString(false)).ToEqual("{\"alive\":4.0}");
			});

			It("Can write StringView values", []()
			{
				JsonFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("alive", StringView{"yes"});
				Expect(writer.ToString(false)).ToEqual("{\"alive\":\"yes\"}");
			});
		});
	});
});
