// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Math/Math.h"
#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"
#include "Strings/String.h"

#include <yyjson.h>


namespace Rift::Serl
{
	u64 GetKeyTag(const StringView& name)
	{
		return (u64(name.size()) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
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
		currentValue = root;
		if (currentValue)
		{
			PushContainer();
		}
	}

	JsonFormatReader::~JsonFormatReader()
	{
		if (scopeStack.Size() > 0)
		{
			PopContainer();
		}
		EnsureMsg(scopeStack.Size() == 0,
		    "Missing LeaveScope somewhere? An scope of more have not been closed.");
		yyjson_doc_free(doc);
	}

	JsonFormatReader::Scope& JsonFormatReader::GetContainer()
	{
		return scopeStack.Last();
	}

	void JsonFormatReader::PushContainer()
	{
		Scope& scope = scopeStack.AddRef({});
		scope.id     = 0;
		scope.size   = 0;
		scope.parent = currentValue;

		// The iterator is now invalid
		currentValue = nullptr;
	}

	void JsonFormatReader::PopContainer()
	{
		currentValue = scopeStack.Last().parent;
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	void JsonFormatReader::BeginObject()
	{
		Scope& container = GetContainer();
		if (currentValue != container.parent) [[unlikely]]
		{
			Log::Error(
			    "First value is not the container. Has this scope already been declared with "
			    "BeginObject() or BeginArray()?");
			return;
		}

		if (unsafe_yyjson_get_type(currentValue) == YYJSON_TYPE_OBJ)
		{
			container.id   = 0;
			container.size = unsafe_yyjson_get_len(currentValue);

			// Clean the iterating value until the first Next()
			currentValue = nullptr;
		}
	}

	void JsonFormatReader::BeginArray(u32& size)
	{
		if (!Ensure(IsArray())) [[unlikely]]
		{
			return;
		}

		Scope& container = GetContainer();
		if (currentValue != container.parent) [[unlikely]]
		{
			Log::Error(
			    "First value is not the container. Has this scope already been declared with "
			    "BeginObject() or BeginArray()?");
			return;
		}

		if (unsafe_yyjson_get_type(currentValue) == YYJSON_TYPE_ARR)
		{
			container.id   = 0;
			container.size = unsafe_yyjson_get_len(currentValue);

			// Iterate to first value
			currentValue = unsafe_yyjson_get_first(currentValue);
		}
	}

	bool JsonFormatReader::EnterNext(StringView name)
	{
		auto& container = GetContainer();
		if (!Ensure(yyjson_is_obj(container.parent))) [[unlikely]]
		{
			return;
		}

		if (!currentValue)
		{
			// Check first element
			yyjson_val* firstKey = unsafe_yyjson_get_first(container.parent);
			const u64 tag        = GetKeyTag(name);
			if (firstKey && firstKey->tag == tag && CheckKey(firstKey, name))
			{
				PushContainer();
				return true;
			}
		}

		if (FindNextKey(name, container.id, currentValue))
		{
			PushContainer();
			return true;
		}
		return false;
	}

	bool JsonFormatReader::EnterNext()
	{
		auto& container = GetContainer();
		if (!Ensure(yyjson_is_arr(container.parent))) [[unlikely]]
		{
			return;
		}

		if (container.id >= container.size)
		{
			Log::Error("Tried enter more child scopes than available (Index: {}, Max: {})",
			    container.id, container.size);
			return false;
		}

		// First value is already asigned to first, so we only check other ids
		if (container.id > 0) [[likely]]
		{
			currentValue = unsafe_yyjson_get_next(currentValue);
		}
		PushContainer();
		return true;
	}

	void JsonFormatReader::Leave()
	{
		if (EnsureMsg(scopeStack.Size() >= 1,
		        "Closed an extra scope! When surrounding EnterScope in if(), make sure to call "
		        "leave scope inside the brackets."))
		{
			PopContainer();
		}
	}

	void JsonFormatReader::Read(bool& val)
	{
		val = yyjson_get_bool(currentValue);
	}

	void JsonFormatReader::Read(u8& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u8(unsafe_yyjson_get_uint(currentValue));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u8(unsafe_yyjson_get_sint(currentValue));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u8(unsafe_yyjson_get_real(currentValue));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i32& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i32(unsafe_yyjson_get_sint(currentValue));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i32(unsafe_yyjson_get_uint(currentValue));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i32(unsafe_yyjson_get_real(currentValue));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u32& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u32(unsafe_yyjson_get_uint(currentValue));
				break;
			case YYJSON_SUBTYPE_SINT: {
				val = u32(Math::Max<i64>(unsafe_yyjson_get_sint(currentValue), 0));
				break;
			}
			case YYJSON_SUBTYPE_REAL:
				val = u32(unsafe_yyjson_get_real(currentValue));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i64& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(currentValue);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i64(unsafe_yyjson_get_uint(currentValue));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i64(unsafe_yyjson_get_real(currentValue));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u64& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(currentValue);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u64(Math::Max<i64>(unsafe_yyjson_get_sint(currentValue), 0));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u64(unsafe_yyjson_get_real(currentValue));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(float& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_REAL:
				val = float(unsafe_yyjson_get_real(currentValue));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = float(unsafe_yyjson_get_sint(currentValue));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = float(unsafe_yyjson_get_uint(currentValue));
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(double& val)
	{
		switch (yyjson_get_subtype(currentValue))
		{
			case YYJSON_SUBTYPE_REAL:
				val = unsafe_yyjson_get_real(currentValue);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(currentValue);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(currentValue);
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(StringView& val)
	{
		if (yyjson_is_str(currentValue))
		{
			val = {unsafe_yyjson_get_str(currentValue), unsafe_yyjson_get_len(currentValue)};
			return;
		}
		val = {};
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(currentValue);
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(currentValue);
	}

	bool JsonFormatReader::FindNextKey(StringView name, u32& outIndex, yyjson_val*& outValue)
	{
		const Scope& container = GetContainer();
		const u64 tag          = GetKeyTag(name);

		// Get key of the current value. See yyjson_obj_foreach
		auto* key = currentValue - 1;
		u32 i;
		// Iterate (current, last]
		for (i = container.id + 1; i < container.size; ++i)
		{
			key = unsafe_yyjson_get_next(key);
			if (key->tag == tag && CheckKey(key, name))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
		}

		// Iterate [first, current]
		key = unsafe_yyjson_get_first(container.parent);
		for (i = 0; i <= container.id; ++i)
		{
			key = unsafe_yyjson_get_next(key);
			if (key->tag == tag && CheckKey(key, name))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
		}

		return false;
	}
}    // namespace Rift::Serl
