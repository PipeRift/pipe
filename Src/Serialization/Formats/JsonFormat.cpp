// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Math/Math.h"
#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"
#include "Strings/String.h"

#include <yyjson.h>


namespace Rift::Serl
{
	u64 GetKeyTag(sizet size)
	{
		return (((u64) size) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
	}
	bool CheckKey(yyjson_val* key, const StringView& name)
	{
		return memcmp(key->uni.ptr, name.data(), name.size()) == 0;
	}

	JsonFormatReader::JsonFormatReader(const String& data)
	{
		// TODO: Support json from file (yyjson_read_file)
		doc          = yyjson_read(data.data(), data.length(), 0);
		root         = yyjson_doc_get_root(doc);
		PushScope(root);
	}

	JsonFormatReader::~JsonFormatReader()
	{
		if (scopeStack.Size() > 0)
		{
			PopScope();
		}
		EnsureMsg(scopeStack.Size() == 0,
		    "Forgot to Leave() some scope? One or more scopes have not been closed.");
		yyjson_doc_free(doc);
	}

	JsonFormatReader::Scope& JsonFormatReader::GetScope()
	{
		return scopeStack.Last();
	}

	void JsonFormatReader::PushScope(yyjson_val* newScope)
	{
		scopeStack.Add({});
		current = newScope;
	}

	void JsonFormatReader::PopScope()
	{
		if (auto* parent = GetScope().parent)
		{
			current = parent;
		}
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	void JsonFormatReader::BeginObject()
	{
		if (!yyjson_is_obj(current)) [[unlikely]]
		{
			return;    // The current scope is not an object
		}
		InternalBegin();
	}

	void JsonFormatReader::BeginArray(u32& size)
	{
		if (!yyjson_is_arr(current)) [[unlikely]]
		{
			return;    // The current scope is not an array
		}
		size = InternalBegin();
	}

	bool JsonFormatReader::EnterNext(StringView name)
	{
		auto& scope = GetScope();
		if (!EnsureMsg(yyjson_is_obj(scope.parent),
		        "Current scope is not an object or has not been initialized with BeginObject()"))
		    [[unlikely]]
		{
			return false;
		}

		u32 firstId;
		yyjson_val* firstKey;
		if (scope.id == 0) [[unlikely]]    // Look indexes
		{
			// Start looking from first element
			firstId  = 0;
			firstKey = unsafe_yyjson_get_first(scope.parent);
		}
		else
		{
			// Start looking from next element
			firstId  = scope.id;
			firstKey = unsafe_yyjson_get_next(current);
		}

		if (FindNextKey(firstId, firstKey, name, scope.id, current))
		{
			++scope.id;
			if (scope.id >= scope.size)
			{
				scope.id = 0;
			}
			PushScope(current);
			return true;
		}
		return false;
	}

	bool JsonFormatReader::EnterNext()
	{
		auto& scope = GetScope();
		if (!EnsureMsg(yyjson_is_arr(scope.parent),
		        "Current scope is not an array or has not been initialized with BeginArray()"))
		    [[unlikely]]
		{
			return false;
		}

		if (scope.id >= scope.size)
		{
			Log::Error("Tried enter more child scopes than available (Index: {}, Max: {})",
			    scope.id, scope.size);
			return false;
		}

		if (scope.id == 0) [[unlikely]]
		{
			++scope.id;
			PushScope(unsafe_yyjson_get_first(scope.parent));
		}
		else
		{
			++scope.id;
			PushScope(unsafe_yyjson_get_next(current));
		}
		return true;
	}

	void JsonFormatReader::Leave()
	{
		if (EnsureMsg(scopeStack.Size() >= 1,
		        "Closed an extra scope! When surrounding EnterScope in if(), make sure to call "
		        "leave scope inside the brackets."))
		{
			PopScope();
		}
	}

	void JsonFormatReader::Read(bool& val)
	{
		val = yyjson_get_bool(current);
	}

	void JsonFormatReader::Read(u8& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u8(unsafe_yyjson_get_uint(current));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u8(unsafe_yyjson_get_sint(current));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u8(unsafe_yyjson_get_real(current));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i32& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i32(unsafe_yyjson_get_sint(current));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i32(unsafe_yyjson_get_uint(current));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i32(unsafe_yyjson_get_real(current));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u32& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u32(unsafe_yyjson_get_uint(current));
				break;
			case YYJSON_SUBTYPE_SINT: {
				val = u32(Math::Max<i64>(unsafe_yyjson_get_sint(current), 0));
				break;
			}
			case YYJSON_SUBTYPE_REAL:
				val = u32(unsafe_yyjson_get_real(current));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i64& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(current);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i64(unsafe_yyjson_get_uint(current));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i64(unsafe_yyjson_get_real(current));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u64& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(current);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u64(Math::Max<i64>(unsafe_yyjson_get_sint(current), 0));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u64(unsafe_yyjson_get_real(current));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(float& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_REAL:
				val = float(unsafe_yyjson_get_real(current));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = float(unsafe_yyjson_get_sint(current));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = float(unsafe_yyjson_get_uint(current));
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(double& val)
	{
		switch (yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_REAL:
				val = unsafe_yyjson_get_real(current);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(current);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(current);
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(StringView& val)
	{
		if (yyjson_is_str(current))
		{
			val = {unsafe_yyjson_get_str(current), unsafe_yyjson_get_len(current)};
			return;
		}
		val = {};
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(current);
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(current);
	}

	u32 JsonFormatReader::InternalBegin()
	{
		Scope& scope = GetScope();
		if (scope.size > 0) [[unlikely]]
		{
			Log::Error("Have BeginObject() or BeginArray() been called already in this scope?");
			return;
		}
		scope.id     = 0;
		scope.size   = unsafe_yyjson_get_len(current);
		scope.parent = current;
		current      = nullptr;
		return scope.size;
	}

	bool JsonFormatReader::FindNextKey(
	    u32 firstId, yyjson_val* firstKey, StringView name, u32& outIndex, yyjson_val*& outValue)
	{
		const Scope& scope = GetScope();
		const u64 tag      = GetKeyTag(name.size());

		// Get key of the current value. See yyjson_obj_foreach
		auto* key = firstKey;
		u32 i;
		// Iterate [firstId, last]
		for (i = firstId; i < scope.size; ++i)
		{
			if (key->tag == tag && CheckKey(key, name))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
			key = unsafe_yyjson_get_next(key + 1);
		}

		// Iterate [first, firstId)
		key = unsafe_yyjson_get_first(scope.parent);
		for (i = 0; i < firstId; ++i)
		{
			if (key->tag == tag && CheckKey(key, name))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
			key = unsafe_yyjson_get_next(key + 1);
		}

		return false;
	}


	JsonFormatWriter::JsonFormatWriter()
	{
		doc = yyjson_mut_doc_new(nullptr);
		if (doc)
		{
			current = doc->root;
			PushScope({});
		}
	}

	JsonFormatWriter::~JsonFormatWriter()
	{
		Close();
		yyjson_mut_doc_free(doc);
	}

	JsonFormatWriter::Scope& JsonFormatWriter::GetScope()
	{
		return scopeStack.Last();
	}

	void JsonFormatWriter::PushScope(StringView key)
	{
		scopeStack.Add({key, current});
		current = nullptr;    // New scope
	}

	void JsonFormatWriter::PopScope()
	{
		const Scope& scope = GetScope();
		auto* parent       = scope.parent;
		if (current)
		{
			switch (yyjson_mut_get_type(parent))
			{
				case YYJSON_TYPE_OBJ:
					if (!scope.key.empty())
					{
						yyjson_mut_obj_add_val(doc, parent, scope.key.data(), current);
					}
					break;
				case YYJSON_TYPE_ARR:
					yyjson_mut_arr_append(parent, current);
					break;
			}
		}
		current = parent;
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	bool JsonFormatWriter::EnterNext(StringView name)
	{
		if (!EnsureMsg(yyjson_mut_is_obj(current),
		        "Current scope is not an object or has not been initialized with BeginObject()"))
		    [[unlikely]]
		{
			return false;
		}
		PushScope(name);
		return true;
	}

	bool JsonFormatWriter::EnterNext()
	{
		if (!EnsureMsg(yyjson_mut_is_arr(current),
		        "Current scope is not an array or has not been initialized with BeginArray()"))
		    [[unlikely]]
		{
			return false;
		}
		PushScope({});
		return true;
	}

	void JsonFormatWriter::Leave()
	{
		PopScope();
	}

	void JsonFormatWriter::BeginObject()
	{
		Scope& scope = GetScope();
		if (!EnsureMsg(!current, "Scope must not be initialized"))
		{
			return;
		}

		current = yyjson_mut_obj(doc);
	}

	void JsonFormatWriter::BeginArray(u32& size)
	{
		Scope& scope = GetScope();
		if (!EnsureMsg(!current, "Scope must not be initialized"))
		{
			return;
		}

		current = yyjson_mut_arr(doc);
	}

	void JsonFormatWriter::Write(bool val)
	{
		current = yyjson_mut_bool(doc, val);
	}
	void JsonFormatWriter::Write(u8 val)
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
		current = yyjson_mut_strn(doc, val.data(), val.size());
	}

	void JsonFormatWriter::Close()
	{
		if (open)
		{
			open = false;
			yyjson_mut_doc_set_root(doc, current);

			if (scopeStack.Size() > 0)
			{
				PopScope();
			}
			EnsureMsg(scopeStack.Size() == 0,
			    "Forgot to Leave() some scope? One or more scopes have not been closed.");
		}
	}

	StringView JsonFormatWriter::ToString(bool pretty, bool ensureClosed)
	{
		if (ensureClosed)
		{
			Close();
		}
		yyjson_write_flag flags = pretty ? YYJSON_WRITE_PRETTY : 0;
		sizet size;
		return {yyjson_mut_write(doc, flags, &size), size};
	}
}    // namespace Rift::Serl
