// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PipeSerialize.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct TypeA
{
	bool value = false;
};
void Read(Reader& ct, TypeA& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}
void Write(Writer& ct, const TypeA& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}


struct TypeB
{
	bool value = false;
};
template<>
struct p::TFlags<TypeB> : public p::DefaultTFlags
{
	enum
	{
		HasSingleSerialize = true
	};
};

void Serialize(ReadWriter& ct, TypeB& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}


struct TypeC
{
	bool value = false;

	void Read(Reader& ct)
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
	void Write(Writer& ct) const
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
};
template<>
struct p::TFlags<TypeC> : public p::DefaultTFlags
{
	enum
	{
		HasMemberSerialize = true
	};
};


struct TypeD
{
	bool value = false;

	void Serialize(ReadWriter& ct)
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
};
template<>
struct p::TFlags<TypeD> : public p::DefaultTFlags
{
	enum
	{
		HasMemberSerialize = true,
		HasSingleSerialize = true
	};
};


go_bandit([]() {
	describe("Serialization", []() {
		describe("Serializers in global scope", [&]() {
			it("Can use custom Read()", [&]() {
				TypeA val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				Reader& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			it("Can use custom Write()", [&]() {
				TypeA val{};
				val.value = true;

				JsonFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(writer.ToString(false), Equals("{\"type\":{\"value\":true}}"));
			});

			it("Can use Serialize() instead of Read()", [&]() {
				TypeB val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				Reader& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			it("Can use Serialize() instead of Write()", [&]() {
				TypeB val{};
				val.value = true;

				JsonFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(writer.ToString(false), Equals("{\"type\":{\"value\":true}}"));
			});
		});

		describe("Serializers as members", [&]() {
			it("Can use custom Read()", [&]() {
				TypeC val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				Reader& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			it("Can use custom Write()", [&]() {
				TypeC val{};
				val.value = true;

				JsonFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(writer.ToString(false), Equals("{\"type\":{\"value\":true}}"));
			});

			it("Can use Serialize() instead of Read()", [&]() {
				TypeD val{};
				JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

				Reader& ct = reader;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(val.value, Equals(true));
			});

			it("Can use Serialize() instead of Write()", [&]() {
				TypeD val{};
				val.value = true;

				JsonFormatWriter writer{};
				Writer ct = writer;
				ct.BeginObject();
				ct.Next("type", val);
				AssertThat(writer.ToString(false), Equals("{\"type\":{\"value\":true}}"));
			});
		});
	});
});
