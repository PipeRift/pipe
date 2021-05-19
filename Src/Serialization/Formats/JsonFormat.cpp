// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Math/Math.h"
#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"
#include "Strings/String.h"

#include <yyjson.h>


namespace Rift::Serl
{
	JsonFormatReader::Scope& JsonFormatReader::GetContainer()
	{
		return scopeStack.Last();
	}

	bool JsonFormatReader::PushContainer()
	{
		if (!currentValue) [[unlikely]]
		{
			return true;
		}

		Scope& scope = scopeStack.AddRef({});
		scope.id     = 0;
		scope.size   = 0;
		scope.parent = currentValue;
		return false;
	}

	void JsonFormatReader::PopContainer()
	{
		currentValue = scopeStack.Last().parent;
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	JsonFormatReader::JsonFormatReader(const String& data)
	{
		// TODO: Support json from file (yyjson_read_file)
		doc          = yyjson_read(data.data(), data.length(), 0);
		root         = yyjson_doc_get_root(doc);
		currentValue = root;
		PushContainer();
	}

	JsonFormatReader::~JsonFormatReader()
	{
		PopContainer();
		Ensure(scopeStack.Size() == 0,
		    "Missing LeaveScope somewhere? An scope of more have not been closed.");
		yyjson_doc_free(doc);
	}

	void JsonFormatReader::BeginObject()
	{
		assert(IsObject());
		if (!currentValue) [[unlikely]]
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
		assert(IsArray());
		if (!currentValue) [[unlikely]]
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
		assert(IsObject());

		auto& container = GetContainer();
		if (container.id < container.size)
		{
			currentValue = unsafe_yyjson_get_next(currentValue);
		}
		PushContainer();


		if (yyjson_val* newScope = yyjson_obj_getn(currentValue, name.data(), name.size()))
		{
			scopeStack.Add({currentValue, 0});
			currentValue = newScope;
			return true;
		}
		return false;
	}

	bool JsonFormatReader::EnterNext()
	{
		assert(IsArray());

		auto& container = GetContainer();
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
		Ensure(scopeStack.Size() >= 1,
		    "Closed an extra scope! When surrounding EnterScope in if(), make sure to call leave "
		    "scope inside the brackets.");

		PopContainer();
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

	void JsonFormatReader::IterateObject(TFunction<void()> callback)
	{
		yyjson_obj_iter objectIt;
		scopeStack.Add(currentValue);    // Manually enter scope
		yyjson_obj_iter_init(currentValue, &objectIt);
		yyjson_val* key;
		while (key = yyjson_obj_iter_next(&objectIt))
		{
			currentValue = yyjson_obj_iter_get_val(key);
			callback();
		}
		Leave();
	}

	void JsonFormatReader::IterateObject(TFunction<void(const char*)> callback)
	{
		yyjson_obj_iter objectIt;
		scopeStack.Add(currentValue);    // Manually enter scope
		yyjson_obj_iter_init(currentValue, &objectIt);
		yyjson_val* key;
		while (key = yyjson_obj_iter_next(&objectIt))
		{
			currentValue = yyjson_obj_iter_get_val(key);
			callback(yyjson_get_str(key));
		}
		Leave();
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(currentValue);
	}

	sizet JsonFormatReader::ReadObjectSize() const
	{
		return yyjson_obj_size(currentValue);
	}

	void JsonFormatReader::IterateArray(TFunction<void()> callback)
	{
		yyjson_arr_iter arrayIt;
		scopeStack.Add(currentValue);    // Manually enter scope
		yyjson_arr_iter_init(currentValue, &arrayIt);
		while (currentValue = yyjson_arr_iter_next(&arrayIt))
		{
			callback();
		}
		LeaveScope();
	}

	void JsonFormatReader::IterateArray(TFunction<void(u32)> callback)
	{
		yyjson_arr_iter arrayIt;
		scopeStack.Add(currentValue);    // Manually enter scope
		yyjson_arr_iter_init(currentValue, &arrayIt);
		while (currentValue = yyjson_arr_iter_next(&arrayIt))
		{
			callback(arrayIt.idx - 1);
		}
		LeaveScope();
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(currentValue);
	}
}    // namespace Rift::Serl
