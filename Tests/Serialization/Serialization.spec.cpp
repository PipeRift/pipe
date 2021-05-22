// Copyright 2015-2021 Piperift - All rights reserved

#include <Context.h>
#include <Serialization/CommonContext.h>
#include <Serialization/Formats/JsonFormat.h>
#include <Serialization/ReadContext.h>
#include <Serialization/WriteContext.h>
#include <bandit/bandit.h>



using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Serl;


struct TypeA
{
	bool value = false;
};
void Read(ReadContext& ct, TypeA& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}
void Write(WriteContext& ct, const TypeA& val)
{
	// ct.BeginObject();
	// ct.Next("value", val.value);
}

struct TypeB
{
	bool value = false;
};
DEFINE_TYPE_FLAGS(TypeB, HasSingleSerialize = true);
void Serialize(CommonContext& ct, TypeB& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}

struct TypeC
{
	bool value = false;

	void Read(ReadContext& ct)
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
	void Write(WriteContext& ct)
	{
		// ct.BeginObject();
		// ct.Next("value", value);
	}
};
DEFINE_TYPE_FLAGS(TypeC, HasMemberSerialize = true);

struct TypeD
{
	bool value = false;

	void Serialize(CommonContext& ct)
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
};
DEFINE_TYPE_FLAGS(TypeD, HasMemberSerialize = true, HasSingleSerialize = true);


go_bandit([]() {
	describe("Serialization", []() {
		before_each([]() {
			InitializeContext();
		});

		describe("Serializers in global scope", [&]() {
			it("Can use custom Read()", [&]() {
				TypeA val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				ReadContext& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			xit("Can use custom Write()", [&]() {
				TypeA val{};
				val.value = true;
				JsonFormatWriter writer{};
				// WriteScope(writer, "type", val);
				// AssertThat(writer.Data(), Equals("{\"type\": {\"value\": true }}"));
			});

			it("Can use Serialize() instead of Read()", [&]() {
				TypeB val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				ReadContext& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			xit("Can use Serialize() instead of Write()", [&]() {
			});
		});

		describe("Serializers as members", [&]() {
			it("Can use custom Read()", [&]() {
				TypeC val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				ReadContext& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			xit("Can use custom Write()", [&]() {
			});

			it("Can use Serialize() instead of Read()", [&]() {
				TypeD val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				ReadContext& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			xit("Can use Serialize() instead of Write()", [&]() {
			});
		});
	});
});
