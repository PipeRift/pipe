// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Platform.h"
#include "Core/StringView.h"
#include "Reflection/EnumType.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/SerializationTypes.h"
#include "Templates/Tuples.h"
#include "TypeTraits.h"


namespace pipe::serl
{
	struct CORE_API ReadContext
	{
		template<Format format>
		friend struct TFormatReader;

		Flags flags           = Flags_None;
		Format format         = Format::None;
		IFormatReader* reader = nullptr;


	private:
		// Initialize child classes using the copy constructor
		ReadContext() = default;

	public:
		explicit ReadContext(const ReadContext&) = default;
		ReadContext& operator=(const ReadContext&) = default;
		virtual ~ReadContext() {}


		/**
		 * Marks current scope as an Object.
		 * See EnterNext(name) & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * To make an object scope, see 'BeginObject()'
		 * Complexity: Ordered access O(1), Inverse order O(n)
		 * @return true if inside an object scope and the key is found.
		 */
		bool EnterNext(StringView name);

		/**
		 * Finds and reads a value at key "name" of an object.
		 * Complexity: Ordered access O(1), Inverse order O(n)
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		void Next(StringView name, T& val)
		{
			if (EnterNext(name))
			{
				Serialize(val);
				Leave();
			}
		}


		/**
		 * Marks current scope as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being read
		 */
		void BeginArray(u32& size);

		/**
		 * Enters the scope of the next element of an array.
		 * To make an object scope, see 'BeginArray()'
		 * Complexity: O(1)
		 * @return true if inside an array scope and num elements is not exceeded.
		 */
		bool EnterNext();

		/**
		 * Reads a type from the next element of an array.
		 * Complexity: O(1)
		 * This function won't do anything on object or uninitialized scopes
		 */
		template<typename T>
		void Next(T& val)
		{
			if (EnterNext())
			{
				Serialize(val);
				Leave();
			}
		}

		// Reads a type from the current scope
		template<typename T>
		void Serialize(T& val)
		{
			Read(*this, val);
		}

		void Leave();

		bool IsObject();
		bool IsArray();

		constexpr bool IsReading()
		{
			return true;
		}
		constexpr bool IsWriting()
		{
			return false;
		}

	public:
		template<Format format>
		typename FormatBind<format>::Reader& GetReader() requires(HasReader<format>);
	};


	CORE_API void Read(ReadContext& ct, bool& val);
	CORE_API void Read(ReadContext& ct, u8& val);
	CORE_API void Read(ReadContext& ct, i32& val);
	CORE_API void Read(ReadContext& ct, u32& val);
	CORE_API void Read(ReadContext& ct, i64& val);
	CORE_API void Read(ReadContext& ct, u64& val);
	CORE_API void Read(ReadContext& ct, float& val);
	CORE_API void Read(ReadContext& ct, double& val);
	CORE_API void Read(ReadContext& ct, StringView& val);

	template<typename T1, typename T2>
	void Read(ReadContext& ct, TPair<T1, T2>& val)
	{
		ct.BeginObject();
		ct.Next("first", val.first);
		ct.Next("second", val.second);
	}

	template<typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize))
	{
		val.Read(ct);
	}

	template<typename T>
	void Read(ReadContext& ct, T& val) requires(IsArray<T>())
	{
		u32 size;
		ct.BeginArray(size);
		val.Resize(size);
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(val[i]);
		}
	}

	template<typename T>
	void Read(ReadContext& ct, T& val) requires IsEnum<T>
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			String typeStr;
			ct.Serialize(typeStr);
			if (std::optional<T> value = refl::GetEnumValue<T>(typeStr))
			{
				val = value.value();
			}
		}
	}
}    // namespace pipe::serl

namespace pipe
{
	using namespace pipe::serl;
}
