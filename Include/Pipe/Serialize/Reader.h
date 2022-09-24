// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tuples.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/Formats/IFormat.h"
#include "Pipe/Serialize/SerializationTypes.h"


namespace p
{
	namespace internal
	{
		template<typename T>
		struct HasRead : std::false_type
		{};
		template<typename T>
			requires IsVoid<decltype(Read(std::declval<struct Reader&>(), std::declval<T&>()))>
		struct HasRead<T> : std::true_type
		{};
	}    // namespace internal

	template<typename T>
	static constexpr bool Readable = internal::template HasRead<T>::value;


	struct PIPE_API Reader
	{
		template<SerializeFormat format>
		friend struct TFormatReader;

		SerializeFormat format      = SerializeFormat::None;
		IFormatReader* formatReader = nullptr;


	private:
		// Initialize child classes using the copy constructor
		Reader() = default;

	public:
		explicit Reader(const Reader&)   = default;
		Reader& operator=(const Reader&) = default;
		virtual ~Reader() {}


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
			static_assert(Readable<T>,
			    "Type must be readable! No valid read function found. E.g: 'Read(Reader& w, T& "
			    "value)'");
			if constexpr (Readable<T>)
			{
				Read(*this, val);
			}
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
		template<SerializeFormat format>
		typename FormatBind<format>::Reader& GetReader() requires(HasReader<format>);
	};


	// Format reads
	PIPE_API void Read(Reader& ct, bool& val);
	PIPE_API void Read(Reader& ct, u8& val);
	PIPE_API void Read(Reader& ct, i32& val);
	PIPE_API void Read(Reader& ct, u32& val);
	PIPE_API void Read(Reader& ct, i64& val);
	PIPE_API void Read(Reader& ct, u64& val);
	PIPE_API void Read(Reader& ct, float& val);
	PIPE_API void Read(Reader& ct, double& val);
	PIPE_API void Read(Reader& ct, StringView& val);

	// Pipe types reads
	PIPE_API void Read(Reader& ct, Type*& val);
	PIPE_API void Read(Reader& r, TypeId& val);

	template<typename T1, typename T2>
	void Read(Reader& ct, TPair<T1, T2>& val)
	{
		ct.BeginObject();
		ct.Next("first", val.first);
		ct.Next("second", val.second);
	}

	template<typename T>
	void Read(Reader& ct, T& val) requires(
	    bool(TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize))
	{
		val.Read(ct);
	}

	template<typename T>
	void Read(Reader& ct, T& val) requires(IsArray<T>())
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
	void Read(Reader& ct, T& val) requires(IsEnum<T>)
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			StringView typeStr;
			ct.Serialize(typeStr);
			if (std::optional<T> value = GetEnumValue<T>(typeStr))
			{
				val = value.value();
			}
		}
	}
}    // namespace p
