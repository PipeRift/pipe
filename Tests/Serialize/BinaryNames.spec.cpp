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
		Expect(writer.GetData()).ToEqual(TView<u8>{raw});
	});

	It("Reads structured names", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));
		out.Next("b", u8(2));

		BinaryFormatReader reader{writer.GetData()};
		reader.PushAddFlags(FF_RW_Structured);
		Reader& ct = reader;
		ct.BeginObject();
		u8 value = 0;
		ct.Next("a", value);
		Expect(value).ToEqual(1);

		ct.Next("b", value);
		Expect(value).ToEqual(2);
		Expect(reader.HasError()).ToBeFalse();
	});

	It("Keeps the stream when the next key does not match", []()
	{
		BinaryFormatWriter writer{};
		writer.PushAddFlags(FF_RW_Structured);
		Writer& out = writer;
		out.BeginObject();
		out.Next("a", u8(1));

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

	It("Fails to enter when the next key does not match", []()
	{
		TArray<u8> data{
		    2, 0, 0, 0,             // Object count
		    1, 0, 0, 0, 'a',        // Key "a"
		    4, 1,                   // Type_U8, 1
		    1, 0, 0, 0, 'b',        // Key "b"
		    4, 2,                   // Type_U8, 2
		};

		BinaryFormatReader reader{data};
		reader.PushAddFlags(FF_RW_Structured);
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
		Expect(reader.HasError()).ToBeFalse();
	});


});
