// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeSerialize.h>


using namespace p;


struct SerTypeA
{
	bool value = false;
};
void Read(Reader& ct, SerTypeA& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}
void Write(Writer& ct, const SerTypeA& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}


struct SerTypeB
{
	bool value = false;
};
template<>
struct p::TFlags<SerTypeB> : public p::DefaultTFlags
{
	enum
	{
		HasSingleSerialize = true
	};
};

void Serialize(ReadWriter& ct, SerTypeB& val)
{
	ct.BeginObject();
	ct.Next("value", val.value);
}


struct SerTypeC
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
struct p::TFlags<SerTypeC> : public p::DefaultTFlags
{
	enum
	{
		HasMemberSerialize = true
	};
};


struct SerTypeD
{
	bool value = false;

	void Serialize(ReadWriter& ct)
	{
		ct.BeginObject();
		ct.Next("value", value);
	}
};
template<>
struct p::TFlags<SerTypeD> : public p::DefaultTFlags
{
	enum
	{
		HasMemberSerialize = true,
		HasSingleSerialize = true
	};
};


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Serialization", []()
{
	Describe("Serializers in global scope", []()
	{
		It("Can use custom Read()", []()
		{
			SerTypeA val{};
			JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

			Reader& ct = reader;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(val.value).ToEqual(true);
		});

		It("Can use custom Write()", []()
		{
			SerTypeA val{};
			val.value = true;

			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(writer.ToString(false)).ToEqual("{\"type\":{\"value\":true}}");
		});

		It("Can use Serialize() instead of Read()", []()
		{
			SerTypeB val{};
			JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

			Reader& ct = reader;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(val.value).ToEqual(true);
		});

		It("Can use Serialize() instead of Write()", []()
		{
			SerTypeB val{};
			val.value = true;

			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(writer.ToString(false)).ToEqual("{\"type\":{\"value\":true}}");
		});
	});

	Describe("Serializers as members", []()
	{
		It("Can use custom Read()", []()
		{
			SerTypeC val{};
			JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

			Reader& ct = reader;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(val.value).ToEqual(true);
		});

		It("Can use custom Write()", []()
		{
			SerTypeC val{};
			val.value = true;

			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(writer.ToString(false)).ToEqual("{\"type\":{\"value\":true}}");
		});

		It("Can use Serialize() instead of Read()", []()
		{
			SerTypeD val{};
			JsonFormatReader reader{"{\"type\": {\"value\": true }}"};

			Reader& ct = reader;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(val.value).ToEqual(true);
		});

		It("Can use Serialize() instead of Write()", []()
		{
			SerTypeD val{};
			val.value = true;

			JsonFormatWriter writer{};
			Writer ct = writer;
			ct.BeginObject();
			ct.Next("type", val);
			Expect(writer.ToString(false)).ToEqual("{\"type\":{\"value\":true}}");
		});
	});
});
return true;
}();
}    // namespace
