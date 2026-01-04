// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeSerialize.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Serialization.Binary", []() {
		describe("Reader", [&]() {
			it("Can create a reader", [&]() {
				BinaryFormatReader reader{TArray<u8>{}};
				AssertThat(reader.IsValid(), Equals(false));

				BinaryFormatReader reader2{TArray<u8>{255}};
				AssertThat(reader2.IsValid(), Equals(true));
			});

			it("Can read from object value", [&]() {
				TArray<u8> data{255};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				ct.BeginObject();
				u8 value = 0;
				ct.Next(value);
				AssertThat(value, Equals(255));
			});

			it("Can read from array values", [&]() {
				TArray<u8> data{1, 0, 0, 0, 255};
				BinaryFormatReader reader{data};
				Reader ct = reader;
				u32 size  = 0;
				ct.BeginArray(size);
				AssertThat(size, Equals(1));
				u8 value = 0;
				ct.Next(value);
				AssertThat(value, Equals(255));
			});

			it("Can iterate arrays", [&]() {
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
						AssertThat(name, Equals(expected[i]));
					}
					ct.Leave();
				}
			});

			describe("Types", []() {
				it("Can read bool values", [&]() {
					TArray<u8> data{1, 0};
					BinaryFormatReader reader{data};
					Reader& ct = reader;
					ct.BeginObject();
					bool value = false;
					ct.Next("a", value);
					AssertThat(value, Equals(true));
					ct.Next("b", value);
					AssertThat(value, Equals(false));
				});

				it("Can read i8 values", [&]() {
					TArray<u8> data{0, 127, 128};
					BinaryFormatReader reader{data};
					Reader& ct = reader;
					ct.BeginObject();
					i8 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(127));
					ct.Next("b", value);
					AssertThat(value, Equals(-128));
				});

				it("Can read u8 values", [&]() {
					TArray<u8> data{0, 255};
					BinaryFormatReader reader{data};
					Reader& ct = reader;
					ct.BeginObject();
					u8 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(255));
				});

				it("Can read i16 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 0, 128, 255, 127};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					i16 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<i16>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(Limits<i16>::Max()));
				});

				it("Can read u16 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 255, 255};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					u16 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<u16>::Max()));
				});

				it("Can read i32 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 128, 255, 255, 255, 127};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					i32 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<i32>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(Limits<i32>::Max()));
				});

				it("Can read u32 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 0, 0, 255, 255, 255, 255};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					u32 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<u32>::Max()));
				});

				it("Can read i64 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 255, 255, 255,
					    255, 255, 255, 255, 127};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					i64 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<i64>::Lowest()));
					ct.Next("c", value);
					AssertThat(value, Equals(Limits<i64>::Max()));
				});

				it("Can read u64 values", [&]() {
					// Test inbounds and out of bounds values
					TArray<u8> data{0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					u64 value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(0));
					ct.Next("b", value);
					AssertThat(value, Equals(Limits<u64>::Max()));
				});

				it("Can read float values", [&]() {
					TArray<u8> data{51, 51, 179, 191, 0, 0, 96, 64};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					float value = 0.f;
					ct.Next("a", value);
					AssertThat(value, Equals(-1.4f));
					ct.Next("b", value);
					AssertThat(value, Equals(3.5f));
				});

				it("Can read double values", [&]() {
					TArray<u8> data{
					    102, 102, 102, 102, 102, 102, 246, 191, 0, 0, 0, 0, 0, 0, 12, 64};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					double value = 0;
					ct.Next("a", value);
					AssertThat(value, Equals(-1.4));
					ct.Next("b", value);
					AssertThat(value, Equals(3.5));
				});

				it("Can read StringView values", [&]() {
					TArray<u8> data{3, 0, 0, 0, 'y', 'e', 's'};
					BinaryFormatReader reader{data};
					Reader ct = reader;
					ct.BeginObject();
					StringView string;
					ct.Next("a", string);
					AssertThat(string, Equals("yes"));
				});
			});
		});

		describe("Writer", [&]() {
			it("Can create a writer", [&]() {
				BinaryFormatWriter writer{};
				AssertThat(writer.IsValid(), Equals(true));
			});

			it("Can write to object key", [&]() {
				BinaryFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginObject();
				ct.Next("name", StringView{"Miguel"});

				TArray<u8> expected{6, 0, 0, 0, 'M', 'i', 'g', 'u', 'e', 'l'};
				AssertThat(writer.GetData(), Equals(TView<u8>{expected}));
			});

			it("Can write arrays", [&]() {
				BinaryFormatWriter writer{};
				Writer& ct = writer;
				ct.BeginArray(2);
				ct.Next(u8(255));
				ct.Next(u8(255));

				TArray<u8> expected{2, 0, 0, 0, 255, 255};
				AssertThat(writer.GetData(), Equals(TView<u8>{expected}));
			});

			describe("Types", []() {
				it("Can write bool values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", true);
					ct.Next("b", false);
					TArray<u8> expected{1, 0};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write i8 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", i8(127));
					ct.Next("b", i8(-128));
					TArray<u8> expected{127, 128};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write u8 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", u8(0));
					ct.Next("b", u8(255));
					TArray<u8> expected{0, 255};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write i16 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<i16>::Max());
					ct.Next("b", Limits<i16>::Lowest());
					TArray<u8> expected{255, 127, 0, 128};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write u16 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<u16>::Max());
					ct.Next("b", Limits<u16>::Lowest());
					TArray<u8> expected{255, 255, 0, 0};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write i32 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<i32>::Max());
					ct.Next("b", Limits<i32>::Lowest());
					TArray<u8> expected{255, 255, 255, 127, 0, 0, 0, 128};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write u32 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<u32>::Max());
					ct.Next("b", Limits<u32>::Lowest());
					TArray<u8> expected{255, 255, 255, 255, 0, 0, 0, 0};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write i64 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<i64>::Max());
					ct.Next("b", Limits<i64>::Lowest());
					TArray<u8> expected{
					    255, 255, 255, 255, 255, 255, 255, 127, 0, 0, 0, 0, 0, 0, 0, 128};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write u64 values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", Limits<u64>::Max());
					ct.Next("b", Limits<u64>::Lowest());
					TArray<u8> expected{
					    255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write float values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", -1.4f);
					ct.Next("b", 3.5f);
					TArray<u8> expected{51, 51, 179, 191, 0, 0, 96, 64};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write double values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", -1.4);
					ct.Next("b", 3.5);
					TArray<u8> expected{
					    102, 102, 102, 102, 102, 102, 246, 191, 0, 0, 0, 0, 0, 0, 12, 64};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});

				it("Can write StringView values", [&]() {
					BinaryFormatWriter writer{};
					Writer ct = writer;
					ct.BeginObject();
					ct.Next("a", StringView{"yes"});
					TArray<u8> expected{3, 0, 0, 0, 'y', 'e', 's'};
					AssertThat(writer.GetData(), Equals(TView<u8>(expected)));
				});
			});
		});
	});
});
