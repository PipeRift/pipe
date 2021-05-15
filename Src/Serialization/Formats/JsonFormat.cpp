// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Math/Math.h"
#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"
#include "Strings/String.h"

#include <yyjson.h>


namespace Rift::Serl
{
	JsonFormatReader::JsonFormatReader(const String& data)
	{
		// TODO: Support json from file (yyjson_read_file)
		doc  = yyjson_read(data.data(), data.length(), 0);
		root = yyjson_doc_get_root(doc);
		currentScope = root;
	}

	JsonFormatReader::~JsonFormatReader()
	{
		Ensure(scopeStack.Size() == 0,
		    "Missing LeaveScope somewhere? An scope of more have not been closed.");
		yyjson_doc_free(doc);
	}

	bool JsonFormatReader::EnterScope(StringView name)
	{
		assert(IsObject());
		if (yyjson_val* newScope = yyjson_obj_getn(currentScope, name.data(), name.size()))
		{
			scopeStack.Add(currentScope);
			currentScope = newScope;
			return true;
		}
		return false;
	}

	bool JsonFormatReader::EnterScope(u32 index)
	{
		assert(IsArray());
		if (yyjson_val* newScope = yyjson_arr_get(currentScope, index))
		{
			scopeStack.Add(currentScope);
			currentScope = newScope;
			return true;
		}
		return false;
	}

	void JsonFormatReader::LeaveScope()
	{
		Ensure(scopeStack.Size() >= 1,
		    "Closed an extra scope! When surrounding EnterScope in if(), make sure to call leave "
		    "scope inside the brackets.");
		currentScope = scopeStack.Last();
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	void JsonFormatReader::Read(bool& val)
	{
		val = yyjson_get_bool(currentScope);
	}

	void JsonFormatReader::Read(u8& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u8(unsafe_yyjson_get_uint(currentScope));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u8(unsafe_yyjson_get_sint(currentScope));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u8(unsafe_yyjson_get_real(currentScope));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i32& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i32(unsafe_yyjson_get_sint(currentScope));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i32(unsafe_yyjson_get_uint(currentScope));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i32(unsafe_yyjson_get_real(currentScope));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u32& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u32(unsafe_yyjson_get_uint(currentScope));
				break;
			case YYJSON_SUBTYPE_SINT: {
				val = u32(Math::Max<i64>(unsafe_yyjson_get_sint(currentScope), 0));
				break;
			}
			case YYJSON_SUBTYPE_REAL:
				val = u32(unsafe_yyjson_get_real(currentScope));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(i64& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(currentScope);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i64(unsafe_yyjson_get_uint(currentScope));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = i64(unsafe_yyjson_get_real(currentScope));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(u64& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(currentScope);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u64(Math::Max<i64>(unsafe_yyjson_get_sint(currentScope), 0));
				break;
			case YYJSON_SUBTYPE_REAL:
				val = u64(unsafe_yyjson_get_real(currentScope));
				break;
			default:
				val = 0;
		}
	}

	void JsonFormatReader::Read(float& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_REAL:
				val = float(unsafe_yyjson_get_real(currentScope));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = float(unsafe_yyjson_get_sint(currentScope));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = float(unsafe_yyjson_get_uint(currentScope));
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(double& val)
	{
		switch (yyjson_get_subtype(currentScope))
		{
			case YYJSON_SUBTYPE_REAL:
				val = unsafe_yyjson_get_real(currentScope);
				break;
			case YYJSON_SUBTYPE_SINT:
				val = unsafe_yyjson_get_sint(currentScope);
				break;
			case YYJSON_SUBTYPE_UINT:
				val = unsafe_yyjson_get_uint(currentScope);
				break;
			default:
				val = 0.f;
		}
	}

	void JsonFormatReader::Read(StringView& val)
	{
		if (yyjson_is_str(currentScope))
		{
			val = {unsafe_yyjson_get_str(currentScope), unsafe_yyjson_get_len(currentScope)};
			return;
		}
		val = {};
	}

	void JsonFormatReader::IterateObject(TFunction<void()> callback)
	{
		yyjson_obj_iter objectIt;
		scopeStack.Add(currentScope);    // Manually enter scope
		yyjson_obj_iter_init(currentScope, &objectIt);
		yyjson_val* key;
		while (key = yyjson_obj_iter_next(&objectIt))
		{
			currentScope = yyjson_obj_iter_get_val(key);
			callback();
		}
		LeaveScope();
	}

	void JsonFormatReader::IterateObject(TFunction<void(const char*)> callback)
	{
		yyjson_obj_iter objectIt;
		scopeStack.Add(currentScope);    // Manually enter scope
		yyjson_obj_iter_init(currentScope, &objectIt);
		yyjson_val* key;
		while (key = yyjson_obj_iter_next(&objectIt))
		{
			currentScope = yyjson_obj_iter_get_val(key);
			callback(yyjson_get_str(key));
		}
		LeaveScope();
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(currentScope);
	}

	sizet JsonFormatReader::GetObjectSize() const
	{
		return yyjson_obj_size(currentScope);
	}

	void JsonFormatReader::IterateArray(TFunction<void()> callback)
	{
		yyjson_arr_iter arrayIt;
		scopeStack.Add(currentScope);    // Manually enter scope
		yyjson_arr_iter_init(currentScope, &arrayIt);
		while (currentScope = yyjson_arr_iter_next(&arrayIt))
		{
			callback();
		}
		LeaveScope();
	}

	void JsonFormatReader::IterateArray(TFunction<void(u32)> callback)
	{
		yyjson_arr_iter arrayIt;
		scopeStack.Add(currentScope);    // Manually enter scope
		yyjson_arr_iter_init(currentScope, &arrayIt);
		while (currentScope = yyjson_arr_iter_next(&arrayIt))
		{
			callback(arrayIt.idx - 1);
		}
		LeaveScope();
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(currentScope);
	}

	sizet JsonFormatReader::GetArraySize() const
	{
		return yyjson_arr_size(currentScope);
	}
}    // namespace Rift::Serl
