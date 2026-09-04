// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeSerialize.h>
#include <PipeTest.h>


using namespace p;


Spec("Serialization.Binary", []()
{
	Describe("Reader", []()
	{
		It("Can create a reader", []()
		{
			BinaryFormatReader reader{TArray<u8>{}};
			Expect(reader.IsValid()).ToEqual(false);

			BinaryFormatReader reader2{TArray<u8>{255}};
			Expect(reader2.IsValid()).ToEqual(true);
		});

		It("Can read from object value", []()
		{
			TArray<u8> data{255};
			BinaryFormatReader reader{data};
			Reader ct = reader;
			ct.BeginObject();
			u8 value = 0;
			ct.Next(value);
			Expect(value).ToEqual(255);
		});

		It("Can read from array values", []()
		{
			TArray<u8> data{1, 0, 0, 0, 255};
			BinaryFormatReader reader{data};
			Reader ct = reader;
			u32 size  = 0;
			ct.BeginArray(size);
			Expect(size).ToEqual(1);
			u8 value = 0;
			ct.Next(value);
			Expect(value).ToEqual(255);
		});

		It("Can iterate arrays", []()
		{
			TArray<u8> data{2, 0, 0, 0,          // Array size of 2
			    6, 0, 0, 0,                      // size 6
			    'M', 'i', 'g', 'u', 'e', 'l',    //
			    4, 0, 0, 0,                      // size 4
			    'J', 'u', 'a', 'n'};
			BinaryFormatReader reader{data};

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

		Describe("Types", []()
		{
			It("Can read bool values", []()
			{
				TArray<u8> data{1, 0};
				BinaryFormatReader reader{data};
				Reader& ct = reader;
				ct.BeginObject();
				bool value = false;
				ct.Next("a", value);
				Expect(value).ToEqual(true);
				ct.Next("b", value);
				Expect(value).ToEqual(false);
			});

			It("Can read i8 values", []()
			{
				TArray<u8> data{0, 127, 128};
				BinaryFormatReader reader{data};
				Reader& ct = reader;
				ct.BeginObject();
				i8 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(127);
				ct.Next("b", value);
				Expect(value).ToEqual(-128);
			});

			It("Can read u8 values", []()
			{
				TArray<u8> data{0, 255};
				BinaryFormatReader reader{data};
				Reader& ct = reader;
				ct.BeginObject();
				u8 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(255);
			});

			It("Can read i16 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 0, 128, 255, 127};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				i16 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<i16>::Lowest());
				ct.Next("c", value);
				Expect(value).ToEqual(Limits<i16>::Max());
			});

			It("Can read u16 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 255, 255};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				u16 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<u16>::Max());
			});

			It("Can read i32 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 128, 255, 255, 255, 127};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				i32 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<i32>::Lowest());
				ct.Next("c", value);
				Expect(value).ToEqual(Limits<i32>::Max());
			});

			It("Can read u32 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 0, 0, 255, 255, 255, 255};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				u32 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<u32>::Max());
			});

			It("Can read i64 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 255, 255, 255,
				    255, 255, 255, 255, 127};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				i64 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<i64>::Lowest());
				ct.Next("c", value);
				Expect(value).ToEqual(Limits<i64>::Max());
			});

			It("Can read u64 values", []()
			{
				// Test inbounds and out of bounds values
				TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				u64 value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(0);
				ct.Next("b", value);
				Expect(value).ToEqual(Limits<u64>::Max());
			});

			It("Can read float values", []()
			{
				TArray<u8> data{51, 51, 179, 191, 0, 0, 96, 64};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				float value = 0.f;
				ct.Next("a", value);
				Expect(value).ToEqual(-1.4f);
				ct.Next("b", value);
				Expect(value).ToEqual(3.5f);
			});

			It("Can read double values", []()
			{
				TArray<u8> data{102, 102, 102, 102, 102, 102, 246, 191, 0, 0, 0, 0, 0, 0, 12, 64};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				double value = 0;
				ct.Next("a", value);
				Expect(value).ToEqual(-1.4);
				ct.Next("b", value);
				Expect(value).ToEqual(3.5);
			});

			It("Can read StringView values", []()
			{
				TArray<u8> data{3, 0, 0, 0, 'y', 'e', 's'};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				StringView string;
				ct.Next("a", string);
				Expect(string).ToEqual("yes");
			});
		});
	});

	Describe("Writer", []()
	{
		It("Can create a writer", []()
		{
			BinaryFormatWriter writer{};
			Expect(writer.IsValid()).ToEqual(true);
		});

		It("Can write to object key", []()
		{
			BinaryFormatWriter writer{};
			Writer& ct = writer;
			ct.BeginObject();
			ct.Next("name", StringView{"Miguel"});

			TArray<u8> expected{6, 0, 0, 0, 'M', 'i', 'g', 'u', 'e', 'l'};
			Expect(writer.GetData()).ToEqual(TView<u8>{expected});
		});

		It("Can write arrays", []()
		{
			BinaryFormatWriter writer{};
			Writer& ct = writer;
			ct.BeginArray(2);
			ct.Next(u8(255));
			ct.Next(u8(255));

			TArray<u8> expected{2, 0, 0, 0, 255, 255};
			Expect(writer.GetData()).ToEqual(TView<u8>{expected});
		});

		Describe("Types", []()
		{
			It("Can write bool values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", true);
				ct.Next("b", false);
				TArray<u8> expected{1, 0};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write i8 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", i8(127));
				ct.Next("b", i8(-128));
				TArray<u8> expected{127, 128};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write u8 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", u8(0));
				ct.Next("b", u8(255));
				TArray<u8> expected{0, 255};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write i16 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<i16>::Max());
				ct.Next("b", Limits<i16>::Lowest());
				TArray<u8> expected{255, 127, 0, 128};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write u16 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<u16>::Max());
				ct.Next("b", Limits<u16>::Lowest());
				TArray<u8> expected{255, 255, 0, 0};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write i32 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<i32>::Max());
				ct.Next("b", Limits<i32>::Lowest());
				TArray<u8> expected{255, 255, 255, 127, 0, 0, 0, 128};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write u32 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<u32>::Max());
				ct.Next("b", Limits<u32>::Lowest());
				TArray<u8> expected{255, 255, 255, 255, 0, 0, 0, 0};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write i64 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<i64>::Max());
				ct.Next("b", Limits<i64>::Lowest());
				TArray<u8> expected{
				    255, 255, 255, 255, 255, 255, 255, 127, 0, 0, 0, 0, 0, 0, 0, 128};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write u64 values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", Limits<u64>::Max());
				ct.Next("b", Limits<u64>::Lowest());
				TArray<u8> expected{255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write float values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", -1.4f);
				ct.Next("b", 3.5f);
				TArray<u8> expected{51, 51, 179, 191, 0, 0, 96, 64};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write double values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", -1.4);
				ct.Next("b", 3.5);
				TArray<u8> expected{
				    102, 102, 102, 102, 102, 102, 246, 191, 0, 0, 0, 0, 0, 0, 12, 64};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});

			It("Can write StringView values", []()
			{
				BinaryFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("a", StringView{"yes"});
				TArray<u8> expected{3, 0, 0, 0, 'y', 'e', 's'};
				Expect(writer.GetData()).ToEqual(TView<u8>(expected));
			});
		});
	});
});
