// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeSerialize.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Guid.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Extern/yyjson.h"
#include "PipeMath.h"
#include "PipeStrings.h"


static void* yyjson_malloc(void* ctx, p::sizet size)
{
	return p::HeapAlloc(size);
}
static void* yyjson_realloc(void* ctx, void* ptr, p::sizet oldSize, p::sizet size)
{
	return p::HeapRealloc(ptr, size);
}
static void yyjson_free(void* ctx, void* ptr)
{
	p::HeapFree(ptr);
}
yyjson_alc yyjsonAllocator = {yyjson_malloc, yyjson_realloc, yyjson_free, nullptr};


bool yyjson_mut_obj_add_val(
    yyjson_mut_doc* doc, yyjson_mut_val* obj, p::StringView _key, yyjson_mut_val* _val)
{
	if (yyjson_unlikely(!_val))
	{
		return false;
	}

	if (yyjson_likely(yyjson_mut_is_obj(obj) && _key.data()))
	{
		yyjson_mut_val* key = unsafe_yyjson_mut_val(doc, 2);
		if (yyjson_likely(key))
		{
			size_t len          = unsafe_yyjson_get_len(obj);
			yyjson_mut_val* val = key + 1;
			key->tag            = YYJSON_TYPE_STR | YYJSON_SUBTYPE_NONE;
			key->tag |= (uint64_t)_key.size() << YYJSON_TAG_BIT;
			key->uni.str = _key.data();
			val          = _val;
			unsafe_yyjson_set_len(obj, len + 1);
			unsafe_yyjson_mut_obj_add(obj, key, val, len);
			return true;
		}
	}
	return false;
}


namespace p
{
#pragma region Reader
	void Reader::BeginObject()
	{
		GetFormat().BeginObject();
	}

	bool Reader::EnterNext(StringView matchKey)
	{
		return GetFormat().EnterNext(matchKey);
	}

	void Reader::Leave()
	{
		GetFormat().Leave();
	}

	StringView Reader::GetCurrentKey() const
	{
		return GetFormat().GetCurrentKey();
	}

	i32 Reader::GetCurrentIndex() const
	{
		return GetFormat().GetCurrentIndex();
	}

	i32 Reader::GetCurrentCount() const
	{
		return GetFormat().GetCurrentCount();
	}

	void Reader::PushAddFlags(FormatFlags flags)
	{
		GetFormat().PushAddFlags(flags);
	}

	void Reader::PushRemoveFlags(FormatFlags flags)
	{
		GetFormat().PushRemoveFlags(flags);
	}

	void Reader::PopFlags()
	{
		GetFormat().PopFlags();
	}

	FormatFlags Reader::GetFlags()
	{
		return GetFormat().GetFlags();
	}

	void Reader::BeginArray(i32& size)
	{
		GetFormat().BeginArray(size);
	}

	bool Reader::IsObject()
	{
		return GetFormat().IsObject();
	}

	bool Reader::IsArray()
	{
		return GetFormat().IsArray();
	}

	void Read(Reader& r, bool& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i8& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u8& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i16& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u16& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i32& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u32& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i64& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u64& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, float& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, double& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, StringView& val)
	{
		return r.GetFormat().Read(val);
	}
#pragma endregion Reader


#pragma region Writer
	void Writer::BeginObject()
	{
		GetFormat().BeginObject();
	}

	bool Writer::EnterNext(StringView key)
	{
		return GetFormat().EnterNext(key);
	}

	void Writer::BeginArray(i32 size)
	{
		GetFormat().BeginArray(size);
	}

	void Writer::Leave()
	{
		GetFormat().Leave();
	}

	void Writer::PushAddFlags(FormatFlags flags)
	{
		GetFormat().PushAddFlags(flags);
	}
	void Writer::PushRemoveFlags(FormatFlags flags)
	{
		GetFormat().PushRemoveFlags(flags);
	}
	void Writer::PopFlags()
	{
		GetFormat().PopFlags();
	}

	void Write(Writer& w, bool val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, float val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, double val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, StringView val)
	{
		w.GetFormat().Write(val);
	}
#pragma endregion Writer


#pragma region ReadWriter
	void ReadWriter::BeginObject()
	{
		if (IsWriting())
		{
			writer->BeginObject();
		}
		else
		{
			reader->BeginObject();
		}
	}

	bool ReadWriter::EnterNext(StringView key)
	{
		if (IsWriting())
		{
			return writer->EnterNext(key);
		}
		else
		{
			return reader->EnterNext(key);
		}
	}

	void ReadWriter::BeginArray(i32& size)
	{
		if (IsWriting())
		{
			writer->BeginArray(size);
		}
		else
		{
			reader->BeginArray(size);
		}
	}

	void ReadWriter::Leave()
	{
		if (IsWriting())
		{
			writer->Leave();
		}
		else
		{
			reader->Leave();
		}
	}

	void ReadWriter::PushAddFlags(FormatFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushAddFlags(flags);
		}
	}

	void ReadWriter::PushRemoveFlags(FormatFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushRemoveFlags(flags);
		}
	}

	void ReadWriter::PopFlags()
	{
		if (IsWriting())
		{
			GetWrite().PopFlags();
		}
	}
#pragma endregion ReadWriter


#pragma region JsonFormat
	static constexpr StringView emptyJSON{"{}"};
	JsonFormatReader::JsonFormatReader(StringView data)
	{
		if (data.empty())
		{
			data = emptyJSON;
		}
		InternalInit((char*)data.data(), data.length(), false);
	}

	JsonFormatReader::JsonFormatReader(String& data)
	{
		if (data.empty())
		{
			data.append(emptyJSON);
		}

		// Ensure there is at least 4 bytes of extra memory at the end for insitu reading.
		if (data.capacity() - data.size() < 4)
		{
			data.reserve(data.size() + 4);
		}

		InternalInit(data.data(), data.length(), true);
	}

	JsonFormatReader::JsonFormatReader(JsonFormatReader&& other) noexcept
	    : IFormatReader(Move(other))
	    , doc(Exchange(other.doc, nullptr))
	    , root(Exchange(other.root, nullptr))
	    , current(Exchange(other.current, Value{}))
	    , valueStack(Move(other.valueStack))
	    , error(other.error)
	{}

	JsonFormatReader& JsonFormatReader::operator=(JsonFormatReader&& other) noexcept
	{
		if (this != &other)
		{
			yyjson_doc_free(doc);
			IFormatReader::operator=(Move(other));

			doc        = Exchange(other.doc, nullptr);
			root       = Exchange(other.root, nullptr);
			current    = Exchange(other.current, Value{});
			valueStack = Move(other.valueStack);
			error      = other.error;
		}
		return *this;
	}

	JsonFormatReader::~JsonFormatReader()
	{
		P_EnsureMsg(valueStack.Size() == 0,
		    "Forgot to Leave() some value? One or more values have not been closed.");
		yyjson_doc_free(doc);
	}

	void JsonFormatReader::PushValue(yyjson_val* newValue, i32 newIndex, StringView newKey)
	{
		valueStack.Add(current);    // The enclosing value moves to the stack
		current = Value{newValue, newIndex, newKey};
	}

	void JsonFormatReader::PopValue()
	{
		Value& parent    = valueStack.Last();
		parent.last      = current.value;
		parent.lastIndex = current.index;
		current          = parent;
		valueStack.RemoveLast(1, Shrink::No);
	}

	void JsonFormatReader::BeginObject()
	{
		P_CheckMsg(
		    yyjson_is_obj(current.value), "BeginObject() called on a value that is not an object");
	}

	void JsonFormatReader::BeginArray(i32& size)
	{
		if (P_EnsureMsg(yyjson_is_arr(current.value),
		        "BeginArray() called on a value that is not an array"))
		{
			size = i32(yyjson_get_len(current.value));
		}
		else
		{
			size = 0;
		}
	}

	bool JsonFormatReader::EnterNext(StringView matchKey)
	{
		if (!current.value)
		{
			return false;
		}

		if (unsafe_yyjson_is_arr(current.value))
		{
			const i32 size = i32(unsafe_yyjson_get_len(current.value));
			const i32 next = current.lastIndex + 1;    // NO_INDEX (-1) + 1 is the first element
			if (next >= size) [[unlikely]]
			{
				return false;
			}

			yyjson_val* const keyValue = current.last ? unsafe_yyjson_get_next(current.last)
			                                          : unsafe_yyjson_get_first(current.value);
			PushValue(keyValue, next);
			return true;
		}
		else if (unsafe_yyjson_is_obj(current.value))
		{
			const i32 size = i32(unsafe_yyjson_get_len(current.value));
			const i32 next = current.lastIndex + 1;
			if (next >= size) [[unlikely]]
			{
				return false;
			}

			// Next member, whatever its key is
			yyjson_val* const keyValue = current.last ? unsafe_yyjson_get_next(current.last)
			                                          : unsafe_yyjson_get_first(current.value);
			const StringView enteredKey{
			    unsafe_yyjson_get_str(keyValue), unsafe_yyjson_get_len(keyValue)};
			if (matchKey.data() != nullptr && enteredKey != matchKey)
			{
				return false;    // Key mismatch: nothing is entered
			}
			PushValue(keyValue + 1, next, enteredKey);
			return true;
		}

		P_CheckMsg(false,
		    "Tried to enter a value outside an array or object. Has BeginArray()/"
		    "BeginObject() been called?");
		return false;
	}

	void JsonFormatReader::Leave()
	{
		if (P_EnsureMsg(valueStack.Size() >= 1,
		        "Closed an extra value! When surrounding EnterNext in if(), make sure to call "
		        "call Leave() inside the brackets."))
		{
			PopValue();
		}
	}

	StringView JsonFormatReader::GetCurrentKey() const
	{
		return current.key;
	}

	i32 JsonFormatReader::GetCurrentIndex() const
	{
		return current.index;
	}

	i32 JsonFormatReader::GetCurrentCount() const
	{
		if (current.value
		    && (unsafe_yyjson_is_obj(current.value) || unsafe_yyjson_is_arr(current.value)))
		{
			return i32(unsafe_yyjson_get_len(current.value));
		}
		return 0;
	}

	void JsonFormatReader::Read(bool& val)
	{
		val = yyjson_get_bool(current.value);
	}

	void JsonFormatReader::Read(i8& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i8(Clamp<i64>(unsafe_yyjson_get_sint(current.value), Limits<i8>::Lowest(),
				    Limits<i8>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i8(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<i8>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i8(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<i8>::Lowest()),
				    f64(Limits<i8>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(u8& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u8(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<u8>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u8(Clamp<i64>(unsafe_yyjson_get_sint(current.value), 0, Limits<u8>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL:
				val = u8(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<u8>::Lowest()),
				    f64(Limits<u8>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(i16& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i16(Clamp<i64>(unsafe_yyjson_get_sint(current.value), Limits<i16>::Lowest(),
				    Limits<i16>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i16(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<i16>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i16(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<i16>::Lowest()),
				    f64(Limits<i16>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(u16& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u16(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<u16>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u16(Clamp<i64>(unsafe_yyjson_get_sint(current.value), 0, Limits<u16>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL:
				val = u16(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<u16>::Lowest()),
				    f64(Limits<u16>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(i32& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i32(Clamp<i64>(unsafe_yyjson_get_sint(current.value), Limits<i32>::Lowest(),
				    Limits<i32>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i32(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<i32>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i32(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<i32>::Lowest()),
				    f64(Limits<i32>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(u32& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u32(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<u32>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u32(Clamp<i64>(unsafe_yyjson_get_sint(current.value), 0, Limits<u32>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL:
				val = u32(Clamp(unsafe_yyjson_get_real(current.value), f64(Limits<u32>::Lowest()),
				    f64(Limits<u32>::Max())));
				break;
		}
	}
	void JsonFormatReader::Read(i64& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_SINT: val = unsafe_yyjson_get_sint(current.value); break;
			case YYJSON_SUBTYPE_UINT:
				val = i64(Min<u64>(unsafe_yyjson_get_uint(current.value), Limits<i64>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL: {
				// f64(i64 max) rounds up to 2^63, so the bounds must be checked before casting
				const f64 real = unsafe_yyjson_get_real(current.value);
				if (IsNAN(real))
				{
					val = 0;
				}
				else if (real <= f64(Limits<i64>::Lowest()))
				{
					val = Limits<i64>::Lowest();
				}
				else if (real >= f64(Limits<i64>::Max()))
				{
					val = Limits<i64>::Max();
				}
				else
				{
					val = i64(real);
				}
				break;
			}
		}
	}
	void JsonFormatReader::Read(u64& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_UINT: val = unsafe_yyjson_get_uint(current.value); break;
			case YYJSON_SUBTYPE_SINT:
				val = u64(Max<i64>(unsafe_yyjson_get_sint(current.value), 0));
				break;
			case YYJSON_SUBTYPE_REAL: {
				// f64(u64 max) rounds up to 2^64, so the bounds must be checked before casting
				const f64 real = unsafe_yyjson_get_real(current.value);
				if (IsNAN(real) || real <= f64(0))
				{
					val = 0;
				}
				else if (real >= f64(Limits<u64>::Max()))
				{
					val = Limits<u64>::Max();
				}
				else
				{
					val = u64(real);
				}
				break;
			}
		}
	}
	void JsonFormatReader::Read(float& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0.f;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_REAL: val = float(unsafe_yyjson_get_real(current.value)); break;
			case YYJSON_SUBTYPE_SINT: val = float(unsafe_yyjson_get_sint(current.value)); break;
			case YYJSON_SUBTYPE_UINT: val = float(unsafe_yyjson_get_uint(current.value)); break;
		}
	}
	void JsonFormatReader::Read(double& val)
	{
		if (!yyjson_is_num(current.value)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current.value))
		{
			case YYJSON_SUBTYPE_REAL: val = unsafe_yyjson_get_real(current.value); break;
			case YYJSON_SUBTYPE_SINT: val = double(unsafe_yyjson_get_sint(current.value)); break;
			case YYJSON_SUBTYPE_UINT: val = double(unsafe_yyjson_get_uint(current.value)); break;
		}
	}
	void JsonFormatReader::Read(StringView& val)
	{
		if (yyjson_is_str(current.value))
		{
			val = {unsafe_yyjson_get_str(current.value), unsafe_yyjson_get_len(current.value)};
			return;
		}
		val = {};
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(current.value);
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(current.value);
	}

	void JsonFormatReader::InternalInit(char* data, sizet size, bool insitu)
	{
		yyjson_read_flag flags = insitu ? YYJSON_READ_INSITU : 0;
		yyjson_read_err err;

		doc = yyjson_read_opts(data, size, flags, &yyjsonAllocator, &err);
		if (doc)
		{
			root    = yyjson_doc_get_root(doc);
			current = Value{root};
		}
		else
		{
			error = {ReadErrorCode(err.code), err.msg, err.pos};
		}
	}

	JsonFormatWriter::JsonFormatWriter()
	{
		doc = yyjson_mut_doc_new(nullptr);
		if (doc)
		{
			valueStack.Add(Value{});    // Root frame
		}
	}

	JsonFormatWriter::JsonFormatWriter(JsonFormatWriter&& other) noexcept
	    : IFormatWriter(Move(other))
	    , doc(Exchange(other.doc, nullptr))
	    , current(Exchange(other.current, nullptr))
	    , valueStack(Move(other.valueStack))
	    , open(Exchange(other.open, false))
	    , asString(Exchange(other.asString, {}))
	{}

	JsonFormatWriter& JsonFormatWriter::operator=(JsonFormatWriter&& other) noexcept
	{
		if (this != &other)
		{
			Close();
			yyjson_mut_doc_free(doc);
			if (asString.data())
			{
				yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<char*>(asString.data()));
			}
			IFormatWriter::operator=(Move(other));

			doc        = Exchange(other.doc, nullptr);
			current    = Exchange(other.current, nullptr);
			valueStack = Move(other.valueStack);
			open       = Exchange(other.open, false);
			asString   = Exchange(other.asString, {});
		}
		return *this;
	}

	JsonFormatWriter::~JsonFormatWriter()
	{
		Close();
		yyjson_mut_doc_free(doc);

		if (asString.data())
		{
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<char*>(asString.data()));
		}
	}

	JsonFormatWriter::Value& JsonFormatWriter::GetCurrentValue()
	{
		return valueStack.Last();
	}

	void JsonFormatWriter::PushValue(StringView key)
	{
		P_CheckMsg(key.data() != nullptr, "Tried to add an object key value without a key");
		yyjson_mut_val* keyValue;
		if ((GetFlags() & FF_W_CacheStringKeys) > 0)
		{
			keyValue = yyjson_mut_strncpy(doc, key.data(), key.size());
		}
		else
		{
			keyValue = yyjson_mut_strn(doc, key.data(), key.size());
		}
		valueStack.Add({.parent = current, .key = keyValue});
		current = nullptr;    // New value
	}

	void JsonFormatWriter::PushValue(i32 index)
	{
		valueStack.Add({.parent = current, .index = index});
		current = nullptr;    // New value
	}

	void JsonFormatWriter::PopValue()
	{
		const Value& value = GetCurrentValue();
		if (current)
		{
			switch (yyjson_mut_get_type(value.parent))
			{
				case YYJSON_TYPE_OBJ:
					if (value.key)
					{
						yyjson_mut_obj_add(value.parent, value.key, current);
					}
					break;
				case YYJSON_TYPE_ARR:
					if (value.index >= 0 && value.index < i32(yyjson_mut_arr_size(value.parent)))
					{
						yyjson_mut_arr_replace(value.parent, size_t(value.index), current);
					}
					else
					{
						yyjson_mut_arr_append(value.parent, current);
					}
					break;
			}
		}
		current = value.parent;
		valueStack.RemoveLast(1, Shrink::No);
	}

	bool JsonFormatWriter::EnterNext(StringView key)
	{
		if (!current)
		{
			return false;
		}
		else if (unsafe_yyjson_is_obj(current))
		{
			if (key.data() == nullptr) [[unlikely]]
			{
				return false;    // Objects need a key
			}
			PushValue(key);    // "" is a valid key
			return true;
		}
		else if (unsafe_yyjson_is_arr(current))
		{
			if (key.data() != nullptr) [[unlikely]]
			{
				P_CheckMsg(false, "Tried to enter a named value in an array. Arrays have no keys.");
				return false;
			}
			PushValue(NO_INDEX);
			return true;
		}

		P_CheckMsg(false,
		    "Tried to enter a value outside an array or object. Has BeginArray()/"
		    "BeginObject() been called?");
		return false;
	}

	void JsonFormatWriter::Leave()
	{
		// The root value is pushed by the constructor and only closed by Close()
		if (P_EnsureMsg(valueStack.Size() > 1,
		        "Closed an extra value! When surrounding Enter in if(), make sure to call "
		        "call Leave() inside the brackets."))
		{
			PopValue();
		}
	}

	void JsonFormatWriter::BeginObject()
	{
		if (current) [[unlikely]]
		{
			P_CheckMsg(unsafe_yyjson_is_obj(current),
			    "Value is already initialized but it is not an object. Is BeginObject() being "
			    "mixed with BeginArray() in the same value?");
			return;
		}

		current = yyjson_mut_obj(doc);
	}

	void JsonFormatWriter::BeginArray(i32 size)
	{
		if (current) [[unlikely]]
		{
			P_CheckMsg(unsafe_yyjson_is_arr(current),
			    "Value is already initialized but it is not an array. Is BeginArray() being mixed "
			    "with BeginObject() in the same value?");
			return;
		}

		current = yyjson_mut_arr(doc);
	}

	void JsonFormatWriter::Write(bool val)
	{
		current = yyjson_mut_bool(doc, val);
	}
	void JsonFormatWriter::Write(i8 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(u8 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i16 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(const u16 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i32 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(const u32 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i64 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(u64 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(float val)
	{
		current = yyjson_mut_real(doc, val);
	}
	void JsonFormatWriter::Write(double val)
	{
		current = yyjson_mut_real(doc, val);
	}
	void JsonFormatWriter::Write(StringView val)
	{
		if ((GetFlags() & FF_W_CacheStringValues) > 0)
		{
			current = yyjson_mut_strncpy(doc, val.data(), val.size());
		}
		else
		{
			current = yyjson_mut_strn(doc, val.data(), val.size());
		}
	}

	void JsonFormatWriter::Close()
	{
		if (open)
		{
			open = false;
			yyjson_mut_doc_set_root(doc, current);

			if (valueStack.Size() > 0)
			{
				PopValue();
			}
			P_EnsureMsg(valueStack.Size() == 0,
			    "Forgot to Leave() some value? One or more values have not been closed.");
		}
	}

	StringView JsonFormatWriter::ToString(bool pretty, bool ensureClosed)
	{
		if (ensureClosed)
		{
			Close();
		}

		if (asString.data())
		{
			// Free previous string value
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<char*>(asString.data()));
		}

		yyjson_write_flag flags = pretty ? YYJSON_WRITE_PRETTY : 0;
		sizet size;
		asString =
		    StringView{yyjson_mut_write_opts(doc, flags, &yyjsonAllocator, &size, nullptr), size};
		return asString;
	}
#pragma endregion JsonFormat


#pragma region BinaryFormat
	namespace Binary
	{
		enum class Flags : u8
		{
			// Type
			TypeMask = 7,    //_____111
			/** No type, invalid. */
			Type_None = 0,    // _____000
			/** Raw type, no subtype. */
			Type_Raw = 1,    // _____001
			/** Null type: `null` literal, no subtype. */
			Type_Null = 2,    // _____010
			/** Boolean type, subtype: TRUE, FALSE. */
			Type_Bool = 3,    // _____011
			/** Number type, subtype: UINT, SINT, REAL. */
			Type_Number = 4,    // _____100
			/** String type, subtype: NONE, NOESC. */
			Type_String = 5,    // _____101
			/** Array type, no subtype. */
			Type_Array = 6,    // _____110
			/** Object type, no subtype. */
			Type_Object = 7,    // _____111

			// SubType
			SubTypeMask  = 3 << 3,    // ___11___
			SubType_None = 0 << 3,    // ___00___
			/** False subtype: `false` literal. */
			SubType_False = 0 << 3,    // ___00___
			/** True subtype: `true` literal. */
			SubType_True = 1 << 3,    // ___01___
			/** Unsigned integer subtype: `uint64_t`. */
			SubType_UInt = 0 << 3,    // ___00___
			/** Signed integer subtype: `int64_t`. */
			SubType_SInt = 1 << 3,    // ___01___
			/** Real number subtype: `double`. */
			SubType_Real = 2 << 3,    // ___10___

			// Number Size
			SizeMask = 3 << 5,    // _11_____
			Size_8   = 0 << 5,    // _00_____
			Size_16  = 1 << 5,    // _01_____
			Size_32  = 2 << 5,    // _10_____
			Size_64  = 3 << 5,    // _11_____

			// Value type helpers
			Type_True  = Type_Bool | SubType_True,
			Type_False = Type_Bool | SubType_False,
			Type_U8    = Type_Number | SubType_UInt | Size_8,
			Type_I8    = Type_Number | SubType_SInt | Size_8,
			Type_U16   = Type_Number | SubType_UInt | Size_16,
			Type_I16   = Type_Number | SubType_SInt | Size_16,
			Type_U32   = Type_Number | SubType_UInt | Size_32,
			Type_I32   = Type_Number | SubType_SInt | Size_32,
			Type_U64   = Type_Number | SubType_UInt | Size_64,
			Type_I64   = Type_Number | SubType_SInt | Size_64,
			Type_F32   = Type_Number | SubType_Real | Size_32,
			Type_F64   = Type_Number | SubType_Real | Size_64,
		};
		P_DEFINE_FLAG_OPERATORS(Flags)

		constexpr u32 TypeShift    = 0;
		constexpr u32 SubTypeShift = 3;
		constexpr u32 SizeShift    = 5;

		constexpr Flags GetType(Flags flags)
		{
			return flags & Flags::TypeMask;
		}

		constexpr bool IsStructured(Flags flags)
		{
			return GetType(flags) != Flags::Type_None;
		}

		/** Non-structured values are schema-driven and accept any type.
		 * Structured values must match the type stored in their flags. */
		constexpr bool AcceptsStructuredType(Flags flags, Flags expected)
		{
			switch (GetType(flags))
			{
				case Flags::Type_None: return true;
				case Flags::Type_Bool: return GetType(expected) == Flags::Type_Bool;
				default: return flags == expected;
			}
		}

		u8* ReadRaw(u8* p, bool& val)
		{
			val = bool(p[0]);
			return p + 1;
		}

		u8* ReadRaw(u8* p, i8& val)
		{
			val = i8(p[0]);
			return p + 1;
		}

		u8* ReadRaw(u8* p, u8& val)
		{
			val = p[0];
			return p + 1;
		}

		u8* ReadRaw(u8* p, i16& val)
		{
			val = i16(p[0]);
			val |= i16(p[1]) << 8;
			return p + 2;
		}

		u8* ReadRaw(u8* p, u16& val)
		{
			val = u16(p[0]);
			val |= u16(p[1]) << 8;
			return p + 2;
		}

		u8* ReadRaw(u8* p, i32& val)
		{
			val = i32(p[0]);
			val |= i32(p[1]) << 8;
			val |= i32(p[2]) << 16;
			val |= i32(p[3]) << 24;
			return p + 4;
		}

		u8* ReadRaw(u8* p, u32& val)
		{
			val = u32(p[0]);
			val |= u32(p[1]) << 8;
			val |= u32(p[2]) << 16;
			val |= u32(p[3]) << 24;
			return p + 4;
		}

		u8* ReadRaw(u8* p, i64& val)
		{
			val = i64(p[0]);
			val |= i64(p[1]) << 8;
			val |= i64(p[2]) << 16;
			val |= i64(p[3]) << 24;
			val |= i64(p[4]) << 32;
			val |= i64(p[5]) << 40;
			val |= i64(p[6]) << 48;
			val |= i64(p[7]) << 56;
			return p + 8;
		}

		u8* ReadRaw(u8* p, u64& val)
		{
			val = u64(p[0]);
			val |= u64(p[1]) << 8;
			val |= u64(p[2]) << 16;
			val |= u64(p[3]) << 24;
			val |= u64(p[4]) << 32;
			val |= u64(p[5]) << 40;
			val |= u64(p[6]) << 48;
			val |= u64(p[7]) << 56;
			return p + 8;
		}

		u8* ReadRaw(u8* p, float& val)
		{
			CopyMem(&val, p, 4);
			return p + 4;
		}

		u8* ReadRaw(u8* p, double& val)
		{
			CopyMem(&val, p, 8);
			return p + 8;
		}

		u8* ReadFlags(Flags& currentFlags, u8* p)
		{
			if (IsStructured(currentFlags))
			{
				u8 rawFlags  = 0;
				p            = ReadRaw(p, rawFlags);
				currentFlags = Binary::Flags(rawFlags);
			}
			return p;
		}

		u8* ReadKey(u8* at, u8* end, StringView& key)
		{
			u32 size = 0;
			if ((end - at) < sizeof(u32))
			{
				key = {};
				return nullptr;
			}
			at = ReadRaw(at, size);
			if ((end - at) < size)
			{
				key = {};
				return nullptr;
			}
			key = StringView{reinterpret_cast<const char*>(at), size};
			at += size;
			return at;
		}

		sizet WriteRaw(u8* p, sizet offset, i8 val)
		{
			p[offset] = u8(val);
			return offset + 1;
		}

		sizet WriteRaw(u8* p, sizet offset, u8 val)
		{
			p[offset] = val;
			return offset + 1;
		}

		sizet WriteRaw(u8* p, sizet offset, i16 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = val >> 8;
			return offset + 2;
		}

		sizet WriteRaw(u8* p, sizet offset, u16 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = val >> 8;
			return offset + 2;
		}

		sizet WriteRaw(u8* p, sizet offset, i32 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = (val >> 8) & 0xFF;
			p[2] = (val >> 16) & 0xFF;
			p[3] = val >> 24;
			return offset + 4;
		}

		sizet WriteRaw(u8* p, sizet offset, u32 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = (val >> 8) & 0xFF;
			p[2] = (val >> 16) & 0xFF;
			p[3] = val >> 24;
			return offset + 4;
		}

		sizet WriteRaw(u8* p, sizet offset, i64 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = (val >> 8) & 0xFF;
			p[2] = (val >> 16) & 0xFF;
			p[3] = (val >> 24) & 0xFF;
			p[4] = (val >> 32) & 0xFF;
			p[5] = (val >> 40) & 0xFF;
			p[6] = (val >> 48) & 0xFF;
			p[7] = val >> 56;
			return offset + 8;
		}

		sizet WriteRaw(u8* p, sizet offset, u64 val)
		{
			p += offset;
			p[0] = val & 0xFF;
			p[1] = (val >> 8) & 0xFF;
			p[2] = (val >> 16) & 0xFF;
			p[3] = (val >> 24) & 0xFF;
			p[4] = (val >> 32) & 0xFF;
			p[5] = (val >> 40) & 0xFF;
			p[6] = (val >> 48) & 0xFF;
			p[7] = val >> 56;
			return offset + 8;
		}

		sizet WriteRaw(u8* p, sizet offset, float val)
		{
			CopyMem(p + offset, &val, 4);
			return offset + 4;
		}

		sizet WriteRaw(u8* p, sizet offset, double val)
		{
			CopyMem(p + offset, &val, 8);
			return offset + 8;
		}

		sizet WriteFlags(Flags& currentFlags, u8* p, sizet offset, Flags flags)
		{
			if (IsStructured(currentFlags))
			{
				currentFlags = flags;
				return WriteRaw(p, offset, u8(currentFlags));
			}
			return offset;
		}
	}    // namespace Binary

#define EnsureCanRead(expectedType)                                                             \
	if (!P_EnsureMsg(!current.used,                                                             \
	        "Already read this value. Has BeginArray, BeginObject or Read been called before?") \
	    || !Binary::AcceptsStructuredType(current.flags, expectedType)) [[unlikely]]            \
	{                                                                                           \
		return;                                                                                 \
	}                                                                                           \
	current.used = true

#define EnsureCanWrite()                                                                    \
	if (!P_EnsureMsg(!current.used,                                                         \
	        "Already written this value. Has BeginArray, BeginObject or Write been called " \
	        "before?")) [[unlikely]]                                                        \
	{                                                                                       \
		return;                                                                             \
	}                                                                                       \
	current.used = true


	BinaryFormatReader::BinaryFormatReader(TView<u8> data) : data{data}, pointer{data.Data()}
	{
		current = Value{};
	}

	BinaryFormatReader::~BinaryFormatReader()
	{
		P_EnsureMsg(valueStack.IsEmpty(),
		    "Forgot to Leave() some value? One or more values have not been closed.");
	}

	sizet BinaryFormatReader::Remaining() const
	{
		return pointer ? (data.EndData() - pointer) : 0;
	}

	u8* BinaryFormatReader::ReadBytes(sizet size)
	{
		if (!pointer || size > Remaining()) [[unlikely]]
		{
			pointer = nullptr;
			return nullptr;
		}

		u8* ptr = pointer;
		pointer += size;
		return ptr;
	}

	void BinaryFormatReader::PushValue(Value value)
	{
		valueStack.Add(current);
		current = value;
		pointer = data.Data() + value.offset;
	}

	void BinaryFormatReader::PopValue()
	{
		const i32 index = current.index;
		SkipValue();
		current = valueStack.Last();
		valueStack.RemoveLast(1, Shrink::No);
		current.lastIndex = index;
	}

	void BinaryFormatReader::SkipValue()
	{
		if (!pointer)
		{
			return;
		}

		if (current.end)
		{
			pointer = current.end;
		}
		else if (Binary::GetType(current.flags) == Binary::Flags::Type_String)
		{
			// Strings store their length in the stream; read it only to skip an unread string
			if (sizeof(i32) > Remaining()) [[unlikely]]
			{
				pointer = nullptr;
				return;
			}
			u8* next = Binary::ReadRaw(pointer, current.count);
			if (current.count < 0 || current.count > (data.EndData() - next)) [[unlikely]]
			{
				pointer = nullptr;
				return;
			}
			pointer = next + current.count;
		}
		// No way to know the size of this value
	}

	void BinaryFormatReader::ReadSize()
	{
		current.end = nullptr;

		const auto type = Binary::GetType(current.flags);
		switch (type)
		{
			case Binary::Flags::Type_Number: {
				const u32 size = 1u << ((u32(current.flags) & u32(Binary::Flags::SizeMask))
				                        >> Binary::SizeShift);
				if (size > Remaining()) [[unlikely]]
				{
					pointer = nullptr;
					return;
				}
				current.end = pointer + size;
				break;
			}
			case Binary::Flags::Type_Object:
			case Binary::Flags::Type_Array: {
				if (sizeof(u32) > Remaining()) [[unlikely]]
				{
					pointer = nullptr;
					return;
				}
				u32 dataSize = 0;
				pointer      = Binary::ReadRaw(pointer, dataSize);
				if (dataSize > Remaining()) [[unlikely]]
				{
					pointer = nullptr;
					return;
				}
				current.end = pointer + dataSize;
				break;
			}
		}
	}

	void BinaryFormatReader::BeginObject()
	{
		EnsureCanRead(Binary::Flags::Type_Object);

		// Structured objects already had their size read by EnterNext()/ReadSize().
		// Root and non-structured objects still carry the size header here.
		if (!Binary::IsStructured(current.flags))
		{
			if (sizeof(u32) > Remaining()) [[unlikely]]
			{
				pointer = nullptr;
				return;
			}

			u32 dataSize = 0;
			pointer      = Binary::ReadRaw(pointer, dataSize);
			if (dataSize > Remaining()) [[unlikely]]
			{
				pointer = nullptr;
				return;
			}
			current.end = pointer + dataSize;
		}

		current.flags              = Binary::Flags::Type_Object;
		current.structuredChildren = HasFlag(GetFlags(), FF_RW_Structured);
	}

	void BinaryFormatReader::BeginArray(i32& size)
	{
		EnsureCanRead(Binary::Flags::Type_Array);

		if (!Binary::IsStructured(current.flags))
		{
			// No flags: data size followed by the element count
			if (!pointer || sizeof(u32) > Remaining()) [[unlikely]]
			{
				pointer       = nullptr;
				size          = 0;
				current.count = 0;
				return;
			}
			u32 dataSize = 0;
			pointer      = Binary::ReadRaw(pointer, dataSize);
			if (dataSize > Remaining()) [[unlikely]]
			{
				pointer       = nullptr;
				size          = 0;
				current.count = 0;
				return;
			}
			current.end = pointer + dataSize;
		}

		if (!pointer || sizeof(i32) > Remaining()) [[unlikely]]
		{
			pointer       = nullptr;
			size          = 0;
			current.count = 0;
			return;
		}
		pointer = Binary::ReadRaw(pointer, size);
		if (size < 0 || size > Remaining()) [[unlikely]]
		{
			pointer       = nullptr;
			size          = 0;
			current.count = 0;
			return;
		}
		current.count = size;

		current.flags              = Binary::Flags::Type_Array;
		current.structuredChildren = HasFlag(GetFlags(), FF_RW_Structured);
	}

	bool BinaryFormatReader::EnterNext(StringView matchKey)
	{
		if (!pointer) [[unlikely]]
		{
			return false;
		}
		const auto type = Binary::GetType(current.flags);
		if (type == Binary::Flags::Type_Array)
		{
			const i32 next = current.lastIndex + 1;    // NO_INDEX (-1) + 1 is the first element
			if (next >= current.count) [[unlikely]]
			{
				return false;    // End of the array
			}

			Value value;
			value.offset = u32(pointer - data.Data());
			value.index  = next;
			if (current.structuredChildren)
			{
				value.flags = Binary::Flags::Type_Null;
				PushValue(value);
				pointer = Binary::ReadFlags(current.flags, pointer);
				ReadSize();
				if (!pointer)
				{
					PopValue();
					return false;
				}
			}
			else
			{
				value.flags = Binary::Flags::Type_None;
				PushValue(value);
			}
			return true;
		}
		else if (type == Binary::Flags::Type_Object)
		{
			const i32 next = current.lastIndex + 1;
			if (!current.end || pointer >= current.end) [[unlikely]]
			{
				return false;    // End of the object
			}

			if (current.structuredChildren)
			{
				StringView key;
				u8* afterKey = Binary::ReadKey(pointer, data.EndData(), key);
				if (!afterKey || (matchKey.data() != nullptr && key != matchKey))
				{
					return false;
				}
				pointer = afterKey;
				Value value;
				value.index  = next;
				value.offset = u32(pointer - data.Data());
				value.flags  = Binary::Flags::Type_Null;
				value.key    = key;
				PushValue(value);
				pointer = Binary::ReadFlags(current.flags, pointer);
				ReadSize();
				if (!pointer)
				{
					PopValue();
					return false;
				}
			}
			else
			{
				// Non-structured values carry no keys. We dont check
				Value value;
				value.index  = next;
				value.flags  = Binary::Flags::Type_None;
				value.offset = u32(pointer - data.Data());
				PushValue(value);
			}
			return true;
		}

		P_CheckMsg(false,
		    "Tried to enter a value outside an array or object. Has BeginArray()/"
		    "BeginObject() been called?");
		return false;
	}

	void BinaryFormatReader::Leave()
	{
		if (P_EnsureMsg(!valueStack.IsEmpty(),
		        "Closed an extra value! When surrounding Enter in if(), make sure to call "
		        "call Leave() inside the brackets.")) [[unlikely]]
		{
			PopValue();
		}
	}

	StringView BinaryFormatReader::GetCurrentKey() const
	{
		return current.key;
	}

	i32 BinaryFormatReader::GetCurrentIndex() const
	{
		return current.index;
	}

	i32 BinaryFormatReader::GetCurrentCount() const
	{
		return current.count;
	}

	void BinaryFormatReader::Read(bool& val)
	{
		EnsureCanRead(Binary::Flags::Type_Bool);
		// If value is structured and bool, we store as flag
		if (Binary::GetType(current.flags) == Binary::Flags::Type_Bool)
		{
			val = HasFlag(current.flags, Binary::Flags::SubType_True);
			return;
		}

		if (const u8* ptr = ReadBytes(1))
		{
			val = bool(*ptr);
		}
	}

	void BinaryFormatReader::Read(i8& val)
	{
		EnsureCanRead(Binary::Flags::Type_I8);
		if (const u8* ptr = ReadBytes(1))
		{
			val = i8(*ptr);
		}
	}

	void BinaryFormatReader::Read(u8& val)
	{
		EnsureCanRead(Binary::Flags::Type_U8);
		if (const u8* ptr = ReadBytes(1))
		{
			val = *ptr;
		}
	}

	void BinaryFormatReader::Read(i16& val)
	{
		EnsureCanRead(Binary::Flags::Type_I16);
		if (const u8* ptr = ReadBytes(2))
		{
			val = ptr[0];
			val |= i16(ptr[1]) << 8;
		}
	}

	void BinaryFormatReader::Read(u16& val)
	{
		EnsureCanRead(Binary::Flags::Type_U16);
		if (const u8* ptr = ReadBytes(2))
		{
			val = ptr[0];
			val |= u16(ptr[1]) << 8;
		}
	}

	void BinaryFormatReader::Read(i32& val)
	{
		EnsureCanRead(Binary::Flags::Type_I32);
		if (const u8* ptr = ReadBytes(4))
		{
			val = ptr[0];
			val |= i32(ptr[1]) << 8;
			val |= i32(ptr[2]) << 16;
			val |= i32(ptr[3]) << 24;
		}
	}

	void BinaryFormatReader::Read(u32& val)
	{
		EnsureCanRead(Binary::Flags::Type_U32);
		if (const u8* ptr = ReadBytes(4))
		{
			val = ptr[0];
			val |= u32(ptr[1]) << 8;
			val |= u32(ptr[2]) << 16;
			val |= u32(ptr[3]) << 24;
		}
	}

	void BinaryFormatReader::Read(i64& val)
	{
		EnsureCanRead(Binary::Flags::Type_I64);
		if (const u8* ptr = ReadBytes(8))
		{
			val = ptr[0];
			val |= i64(ptr[1]) << 8;
			val |= i64(ptr[2]) << 16;
			val |= i64(ptr[3]) << 24;
			val |= i64(ptr[4]) << 32;
			val |= i64(ptr[5]) << 40;
			val |= i64(ptr[6]) << 48;
			val |= i64(ptr[7]) << 56;
		}
	}

	void BinaryFormatReader::Read(u64& val)
	{
		EnsureCanRead(Binary::Flags::Type_U64);
		if (const u8* ptr = ReadBytes(8))
		{
			val = ptr[0];
			val |= u64(ptr[1]) << 8;
			val |= u64(ptr[2]) << 16;
			val |= u64(ptr[3]) << 24;
			val |= u64(ptr[4]) << 32;
			val |= u64(ptr[5]) << 40;
			val |= u64(ptr[6]) << 48;
			val |= u64(ptr[7]) << 56;
		}
	}

	void BinaryFormatReader::Read(float& val)
	{
		EnsureCanRead(Binary::Flags::Type_F32);
		if (const u8* ptr = ReadBytes(4))
		{
			p::CopyMem(&val, ptr, 4);
		}
	}

	void BinaryFormatReader::Read(double& val)
	{
		EnsureCanRead(Binary::Flags::Type_F64);
		if (const u8* ptr = ReadBytes(8))
		{
			p::CopyMem(&val, ptr, 8);
		}
	}

	void BinaryFormatReader::Read(StringView& val)
	{
		EnsureCanRead(Binary::Flags::Type_String);
		if (!pointer || sizeof(i32) > Remaining()) [[unlikely]]
		{
			pointer = nullptr;
			return;
		}
		pointer = Binary::ReadRaw(pointer, current.count);
		if (!pointer || current.count < 0 || current.count > Remaining()) [[unlikely]]
		{
			pointer = nullptr;
			return;
		}
		val = StringView{reinterpret_cast<char*>(pointer), sizet(current.count)};
		pointer += current.count;
		current.end = pointer;
	}

	bool BinaryFormatReader::IsObject() const
	{
		return Binary::GetType(current.flags) == Binary::Flags::Type_Object;
	}

	bool BinaryFormatReader::IsArray() const
	{
		return Binary::GetType(current.flags) == Binary::Flags::Type_Array;
	}

	bool BinaryFormatReader::IsValid() const
	{
		return data.Data() && !data.IsEmpty();
	}

	bool BinaryFormatReader::HasError() const
	{
		return IsValid() && pointer == nullptr;
	}


	BinaryFormatWriter::BinaryFormatWriter(BinaryFormatWriter&& other) noexcept
	    : IFormatWriter(Move(other))
	    , arena(Exchange(other.arena, nullptr))
	    , data(Exchange(other.data, nullptr))
	    , size(Exchange(other.size, 0u))
	    , capacity(Exchange(other.capacity, 0u))
	    , closed(Exchange(other.closed, false))
	    , current(Exchange(other.current, Value{}))
	    , valueStack(Move(other.valueStack))
	{}

	BinaryFormatWriter& BinaryFormatWriter::operator=(BinaryFormatWriter&& other) noexcept
	{
		if (this != &other)
		{
			if (data)
			{
				Free(*arena, data, capacity);
			}
			IFormatWriter::operator=(Move(other));

			arena      = Exchange(other.arena, nullptr);
			data       = Exchange(other.data, nullptr);
			size       = Exchange(other.size, 0u);
			capacity   = Exchange(other.capacity, 0u);
			closed     = Exchange(other.closed, false);
			current    = Exchange(other.current, Value{});
			valueStack = Move(other.valueStack);
		}
		return *this;
	}

	BinaryFormatWriter::BinaryFormatWriter(Arena& arena)
	    : arena{&arena}, data{static_cast<u8*>(Alloc<u8>(arena, 64))}, capacity{64}
	{
		current = Value{};
	}

	BinaryFormatWriter::~BinaryFormatWriter()
	{
		if (data)
		{
			Free(*arena, data, capacity);
		}
	}

	void BinaryFormatWriter::BeginObject()
	{
		EnsureCanWrite();

		PreAlloc(sizeof(u8) + sizeof(u32));
		size          = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_Object);
		size          = Binary::WriteRaw(data, size, u32(0));    // Patched on leave
		current.flags = Binary::Flags::Type_Object;
		current.structuredChildren = HasFlag(GetFlags(), FF_RW_Structured);
		current.count              = 0;
	}

	void BinaryFormatWriter::BeginArray(i32 arraySize)
	{
		EnsureCanWrite();

		PreAlloc(sizeof(u8) + sizeof(u32) + sizeof(i32));
		size          = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_Array);
		size          = Binary::WriteRaw(data, size, u32(0));       // Size patched on leave
		size          = Binary::WriteRaw(data, size, arraySize);    // Count patched on leave
		current.flags = Binary::Flags::Type_Array;
		current.structuredChildren = HasFlag(GetFlags(), FF_RW_Structured);
		current.count              = 0;
	}

	bool BinaryFormatWriter::EnterNext(StringView key)
	{
		auto type = Binary::GetType(current.flags);
		if (!P_EnsureMsg(type >= Binary::Flags::Type_Array,
		        "Tried to enter value but current is not array or object. Call "
		        "BeginArray() or BeginObject().")) [[unlikely]]
		{
			return false;
		}

		if (type == Binary::Flags::Type_Array)
		{
			P_EnsureMsg(key.data() == nullptr, "Cant EnterNext with a key on arrays. Ignored.");
		}

		if (current.structuredChildren)
		{
			if (type == Binary::Flags::Type_Object)
			{
				// Write Key
				const u32 keySize = u32(key.size());
				PreAlloc(sizeof(u32) + keySize);
				size = Binary::WriteRaw(data, size, keySize);
				CopyMem(data + size, const_cast<char*>(key.data()), keySize);
				size += keySize;
			}
		}

		++current.count;

		const bool structuredChildren = current.structuredChildren;
		valueStack.Add(current);
		current        = Value{};
		current.offset = size;
		current.flags  = structuredChildren ? Binary::Flags::Type_Null : Binary::Flags::Type_None;
		return true;
	}

	void BinaryFormatWriter::Leave()
	{
		if (valueStack.IsEmpty()) [[unlikely]]
		{
			return;    // Unbalanced (no value was entered)
		}

		UpdateValueSize(valueStack.Last().structuredChildren);

		current = valueStack.Last();
		valueStack.RemoveLast(1, Shrink::No);
	}

	void BinaryFormatWriter::Write(bool val)
	{
		EnsureCanWrite();
		PreAlloc(1);
		size = Binary::WriteFlags(
		    current.flags, data, size, val ? Binary::Flags::Type_True : Binary::Flags::Type_False);
		if (!Binary::IsStructured(current.flags))
		{
			size = Binary::WriteRaw(data, size, u8(val));
		}
	}
	void BinaryFormatWriter::Write(i8 val)
	{
		EnsureCanWrite();
		PreAlloc(2);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_I8);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(u8 val)
	{
		EnsureCanWrite();
		PreAlloc(2);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_U8);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(i16 val)
	{
		EnsureCanWrite();
		PreAlloc(3);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_I16);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(u16 val)
	{
		EnsureCanWrite();
		PreAlloc(3);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_U16);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(i32 val)
	{
		EnsureCanWrite();
		PreAlloc(5);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_I32);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(u32 val)
	{
		EnsureCanWrite();
		PreAlloc(5);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_U32);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(i64 val)
	{
		EnsureCanWrite();
		PreAlloc(9);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_I64);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(u64 val)
	{
		EnsureCanWrite();
		PreAlloc(9);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_U64);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(float val)
	{
		EnsureCanWrite();
		PreAlloc(5);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_F32);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(double val)
	{
		EnsureCanWrite();
		PreAlloc(9);
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_F64);
		size = Binary::WriteRaw(data, size, val);
	}
	void BinaryFormatWriter::Write(StringView val)
	{
		EnsureCanWrite();
		current.count = i32(val.size());
		PreAlloc(1u + sizeof(u32) + u32(current.count));
		size = Binary::WriteFlags(current.flags, data, size, Binary::Flags::Type_String);
		size = Binary::WriteRaw(data, size, current.count);
		CopyMem(data + size, const_cast<char*>(val.data()), u32(current.count));
		size += u32(current.count);
	}

	TView<p::u8> BinaryFormatWriter::GetData()
	{
		if (!P_EnsureMsg(closed, "Call Close() before GetData()")) [[unlikely]]
		{
			return {};
		}
		return {data, i32(size)};
	}

	void BinaryFormatWriter::Close()
	{
		if (!closed)
		{
			if (!P_EnsureMsg(valueStack.IsEmpty(), "Close() called with open values")) [[unlikely]]
			{
				return;
			}
			UpdateValueSize(false);
			closed = true;
		}
	}

	void BinaryFormatWriter::UpdateValueSize(bool hasFlags)
	{
		const auto type        = Binary::GetType(current.flags);
		const u32 headerOffset = current.offset + (hasFlags ? 1u : 0u);
		switch (type)
		{
			case Binary::Flags::Type_Object:
				Binary::WriteRaw(data, headerOffset, u32(size - (headerOffset + sizeof(u32))));
				break;
			case Binary::Flags::Type_Array:
				Binary::WriteRaw(data, headerOffset, u32(size - (headerOffset + sizeof(u32))));
				Binary::WriteRaw(data, headerOffset + sizeof(u32), current.count);
				break;
			default: break;
		}
	}

	void BinaryFormatWriter::PreAlloc(u32 offset)
	{
		if (size + offset > capacity) [[unlikely]]
		{
			const u32 oldCapacity = capacity;
			do
			{
				capacity *= 2;    // Grow capacity exponentially
			} while (size + offset > capacity);

			u8* oldData = data;

			data = static_cast<u8*>(Alloc<u8>(*arena, capacity));
			MoveMem(data, oldData, size);
			Free<u8>(*arena, oldData, oldCapacity);
		}
	}
#undef EnsureCanRead
#undef EnsureCanWrite
#pragma endregion BinaryFormat


#pragma region CoreSupport
	void Read(Reader& ct, String& val)
	{
		StringView view;
		ct.Serialize(view);
		val = view;
	}
	void Write(Writer& ct, const String& val)
	{
		ct.Serialize(StringView{val});
	}
	void Read(Reader& ct, Tag& tag)
	{
		StringView str;
		ct.Serialize(str);
		tag = Tag(str);
	}
	void Write(Writer& ct, const Tag& tag)
	{
		ct.Serialize(tag.AsString());
	}

	void Read(Reader& ct, Guid& guid)
	{
		ct.BeginObject();
		ct.Next("a", guid.a);
		ct.Next("b", guid.b);
		ct.Next("c", guid.c);
		ct.Next("d", guid.d);
	}

	void Write(Writer& ct, const Guid& guid)
	{
		ct.BeginObject();
		ct.Next("a", guid.a);
		ct.Next("b", guid.b);
		ct.Next("c", guid.c);
		ct.Next("d", guid.d);
	}

	void Read(Reader& ct, TColor<ColorMode::RGBA>& color)
	{
		u32 value;
		ct.Serialize(value);
		color = TColor<ColorMode::RGBA>::FromHexAlpha(value);
	}
	void Write(Writer& ct, const TColor<ColorMode::RGBA>& color)
	{
		ct.Serialize(color.ToPackedRGBA());
	}
	void Read(Reader& ct, TColor<ColorMode::Linear>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Write(Writer& ct, const TColor<ColorMode::Linear>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Read(Reader& ct, TColor<ColorMode::sRGB>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Write(Writer& ct, const TColor<ColorMode::sRGB>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Read(Reader& r, TColor<ColorMode::HSV>& color)
	{
		r.BeginObject();
		r.Next("h", color.h);
		r.Next("s", color.s);
		r.Next("v", color.v);
		r.Next("a", color.a);
	}
	void Write(Writer& w, const TColor<ColorMode::HSV>& color)
	{
		w.BeginObject();
		w.Next("h", color.h);
		w.Next("s", color.s);
		w.Next("v", color.v);
		w.Next("a", color.a);
	}


	void Read(Reader& ct, Vec<2, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<2, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<2, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<3, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Vec<3, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Vec<3, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Quat& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
		ct.Next("w", val.w);
	}
	void Write(Writer& ct, const Quat& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
		ct.Next("w", val.w);
	}
#pragma endregion CoreSupport
}    // namespace p
