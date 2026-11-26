// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeSerialize.h>
#include <PipeTest.h>


using namespace p;


P_SPEC("Serialization.BinaryNames", []()
{
	It("Writes raw values when structured is disabled", []()
	{
		BinaryFormatWriter writer{};
		Writer& ct = writer;
		ct.BeginObject();
		ct.Next("a", u8(1));
		ct.Next("b", u8(2));

		TArray<u8> raw{2, 0, 0, 0, 1, 2};
		writer.Close();
		Expect(writer.GetData()).ToEqual(TView<u8>{raw});
	});

	It("Writes raw arrays with a count", []()
	{
		BinaryFormatWriter writer{};
		Writer& ct = writer;
		ct.BeginArray(2);
		ct.Next(u8(1));
		ct.Next(u8(2));

		TArray<u8> raw{2, 0, 0, 0, 2, 0, 0, 0, 1, 2};
		writer.Close();
		Expect(writer.GetData()).ToEqual(TView<u8>{raw});
	});

	It("Keeps the stream when the next key does not match", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		Expect(ct.EnterNext("missing")).ToBeFalse();

		u8 value = 0;
		ct.Next("a", value);
		Expect(value).ToEqual(1);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Reports the entered member", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));
		out.Next("b", u8(2));

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		Expect(ct.EnterNext()).ToBeTrue();
		Expect(ct.GetCurrentKey() == "a").ToBeTrue();
		Expect(ct.GetCurrentIndex()).ToEqual(0);
		ct.Leave();

		Expect(ct.EnterNext()).ToBeTrue();
		Expect(ct.GetCurrentKey() == "b").ToBeTrue();
		Expect(ct.GetCurrentIndex()).ToEqual(1);
		ct.Leave();

		Expect(ct.EnterNext()).ToBeFalse();
	});

	It("Round-trips structured scalar types", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("b", true);
		out.Next("i8", i8(-8));
		out.Next("u16", u16(65530));
		out.Next("i32", i32(-123456));
		out.Next("u64", u64(123456789));
		out.Next("f", 1.25f);
		out.Next("d", -2.5);
		out.Next("s", StringView{"hello"});
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		bool b = false;
		i8 i = 0;
		u16 u = 0;
		i32 i32Value = 0;
		u64 u64Value = 0;
		float f = 0.f;
		double d = 0.0;
		StringView s;

		ct.Next("b", b);
		ct.Next("i8", i);
		ct.Next("u16", u);
		ct.Next("i32", i32Value);
		ct.Next("u64", u64Value);
		ct.Next("f", f);
		ct.Next("d", d);
		ct.Next("s", s);

		Expect(b).ToEqual(true);
		Expect(i).ToEqual(i8(-8));
		Expect(u).ToEqual(u16(65530));
		Expect(i32Value).ToEqual(i32(-123456));
		Expect(u64Value).ToEqual(u64(123456789));
		Expect(f).ToEqual(1.25f);
		Expect(d).ToEqual(-2.5);
		Expect(s).ToEqual("hello");
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Enters array elements in order", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("items"))
		{
			out.BeginArray(3);
			out.Next(u8(7));
			out.Next(u8(8));
			out.Next(u8(9));
			out.Leave();
		}

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		if (ct.EnterNext("items"))
		{
			i32 size = 0;
			ct.BeginArray(size);
			Expect(size).ToEqual(3u);

			u8 value = 0;
			ct.Next(value);
			Expect(value).ToEqual(7);
			ct.Next(value);
			Expect(value).ToEqual(8);
			ct.Next(value);
			Expect(value).ToEqual(9);
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Writes unnamed members next to named ones", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next(u8(1));    // Unnamed member
		out.Next("b", u8(2));

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		u8 value = 0;
		Expect(ct.EnterNext()).ToBeTrue();
		Expect(ct.GetCurrentKey().empty()).ToBeTrue();
		ct.Serialize(value);
		Expect(value).ToEqual(1);
		ct.Leave();

		ct.Next("b", value);
		Expect(value).ToEqual(2);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Reads named members written after unnamed ones", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));
		out.Next(u8(2));    // Unnamed

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		u8 value = 0;
		ct.Next("a", value);
		Expect(value).ToEqual(1);

		Expect(ct.EnterNext()).ToBeTrue();
		Expect(ct.GetCurrentKey().empty()).ToBeTrue();
		ct.Serialize(value);
		Expect(value).ToEqual(2);
		ct.Leave();
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Round-trips a nested object with names", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("count", u32(2));
		if (out.EnterNext("items"))
		{
			out.BeginArray(2);
			out.Next(u8(4));
			out.Next(u8(5));
			out.Leave();
		}

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		u32 count = 0;
		ct.Next("count", count);
		Expect(count).ToEqual(2u);

		if (ct.EnterNext("items"))
		{
			i32 size = 0;
			ct.BeginArray(size);
			Expect(size).ToEqual(2u);
			u8 first  = 0;
			u8 second = 0;
			ct.Next(first);
			ct.Next(second);
			Expect(first).ToEqual(4);
			Expect(second).ToEqual(5);
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Round-trips a nested object", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("child"))
		{
			out.BeginObject();
			out.Next("x", u8(1));
			out.Leave();    // Leaves "child", not the object scope
		}
		out.Next("after", u8(2));

		writer.Close();
		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("child"))
		{
			ct.BeginObject();
			u8 x = 0;
			ct.Next("x", x);
			Expect(x).ToEqual(1);
			ct.Leave();
		}

		u8 after = 0;
		ct.Next("after", after);
		Expect(after).ToEqual(2);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Ignores keys on non-structured object members", []()
	{
		BinaryFormatWriter writer{};
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));
		out.Next("b", u8(2));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		Reader& ct = reader;
		ct.BeginObject();

		u8 value = 0;
		ct.Next("not-the-key", value);    // Raw members are entered in order
		Expect(value).ToEqual(1);
		ct.Next("also-not-the-key", value);
		Expect(value).ToEqual(2);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Skips a structured value read as the wrong type", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));
		out.Next("b", u8(2));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		u32 value = 0;
		ct.Next("a", value);
		Expect(value).ToEqual(0u);

		u8 next = 0;
		ct.Next("b", next);
		Expect(next).ToEqual(2);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Skips remaining structured object members", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("child"))
		{
			out.BeginObject();
			out.Next("a", u8(1));
			out.Next("b", u8(2));
			out.Next("c", u8(3));
			out.Leave();
		}
		out.Next("after", u8(4));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("child"))
		{
			ct.BeginObject();
			u8 a = 0;
			ct.Next("a", a);
			Expect(a).ToEqual(1);
			ct.Leave();    // Skips b and c
		}

		u8 after = 0;
		ct.Next("after", after);
		Expect(after).ToEqual(4);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Skips remaining structured array elements", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("items"))
		{
			out.BeginArray(3);
			out.Next(u8(1));
			out.Next(u8(2));
			out.Next(u8(3));
			out.Leave();
		}
		out.Next("after", u8(4));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("items"))
		{
			i32 size = 0;
			ct.BeginArray(size);
			u8 first = 0;
			ct.Next(first);
			Expect(first).ToEqual(1);
			ct.Leave();    // Skips 2 and 3
		}

		u8 after = 0;
		ct.Next("after", after);
		Expect(after).ToEqual(4);
		Expect(reader.HasError()).ToBeFalse();
	});
	It("Skips unread structured numbers", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u32(1));
		out.Next("b", u32(2));
		out.Next("c", u8(3));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();

		u32 a = 0;
		ct.Next("a", a);
		Expect(a).ToEqual(1u);

		Expect(ct.EnterNext("b")).ToBeTrue();
		ct.Leave();    // Skips "b" without reading it

		u8 c = 0;
		ct.Next("c", c);
		Expect(c).ToEqual(3);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Skips an unstructured object by its data size", []()
	{
		BinaryFormatWriter writer{};
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("child"))
		{
			out.BeginObject();
			out.Next("a", u32(1));
			out.Next("b", u32(2));
			out.Leave();
		}
		out.Next("after", u32(3));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("child"))
		{
			ct.BeginObject();
			ct.Leave();    // Skips the unread members through the object size
		}

		u32 after = 0;
		ct.Next("after", after);
		Expect(after).ToEqual(3u);
		Expect(reader.HasError()).ToBeFalse();
	});

	
	
	
	
	It("Reports a mismatch between the call and the loaded flags", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("items"))
		{
			out.BeginArray(3);
			out.Next(u8(1));
			out.Next(u8(2));
			out.Next(u8(3));
			out.Leave();
		}
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("items"))
		{
			// The loaded flags say array; the object call must fail without consuming it
			Expect(ct.BeginObject()).ToBeFalse();

			i32 size = 0;
			Expect(ct.BeginArray(size)).ToBeTrue();
			Expect(size).ToEqual(3);

			u8 value = 0;
			ct.Next(value);
			Expect(value).ToEqual(1);
			ct.Next(value);
			Expect(value).ToEqual(2);
			ct.Next(value);
			Expect(value).ToEqual(3);
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Uses the container subtype to drive structured children", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("structured"))
		{
			out.BeginObject();
			out.Next("x", u8(1));
			out.Leave();
		}
		if (out.EnterNext("raw"))
		{
			out.PushRemoveFlags(FF_RW_Structured);
			out.BeginObject();
			out.PopFlags();
			out.Next(u8(2));
			out.Leave();
		}
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("structured"))
		{
			ct.BeginObject();
			u8 x = 0;
			ct.Next("x", x);
			Expect(x).ToEqual(1);
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}

		if (ct.EnterNext("raw"))
		{
			ct.PushRemoveFlags(FF_RW_Structured);
			ct.BeginObject();
			ct.PopFlags();
			u8 x = 0;
			ct.Next(x);
			Expect(x).ToEqual(2);
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}
		Expect(reader.HasError()).ToBeFalse();
	});
	It("Skips a container whose loaded flags do not match", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		if (out.EnterNext("raw"))
		{
			out.PushRemoveFlags(FF_RW_Structured);
			out.BeginObject();
			out.PopFlags();
			out.Next(u8(2));
			out.Leave();
		}
		out.Next("after", u8(3));
		writer.Close();

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		if (ct.EnterNext("raw"))
		{
			Expect(ct.BeginObject()).ToBeFalse();    // Skipped, loaded flags say raw
			ct.Leave();
		}
		else
		{
			Expect(false).ToBeTrue();
		}

		u8 after = 0;
		ct.Next("after", after);
		Expect(after).ToEqual(3);
		Expect(reader.HasError()).ToBeFalse();
	});


});
