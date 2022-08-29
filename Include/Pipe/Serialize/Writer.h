// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tuples.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/Formats/IFormat.h"
#include "Pipe/Serialize/SerializationTypes.h"


namespace p
{
	namespace internal
	{
		template<typename T>
		struct HasWrite : std::false_type
		{};
		template<typename T>
			requires IsVoid<decltype(Write(
			    std::declval<struct Writer&>(), std::declval<const T&>()))>
		struct HasWrite<T> : std::true_type
		{};
	}    // namespace internal

	template<typename T>
	static constexpr bool Writable = internal::template HasWrite<T>::value;

	struct PIPE_API Writer
	{
		template<SerializeFormat format>
		friend struct TFormatWriter;

		SerializeFormat format = SerializeFormat::None;
		IFormatWriter* writer  = nullptr;


	private:
		Writer() = default;

	public:
		explicit Writer(const Writer&)   = default;
		Writer& operator=(const Writer&) = default;
		virtual ~Writer() {}

		/**
		 * Marks current scope as an Object.
		 * See EnterNext(name) & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * To make object scope, see 'BeginObject()'
		 * @return true if inside an object scope and the key is found.
		 */
		bool EnterNext(StringView name);

		/**
		 * Finds and writes a type at key "name" of an object.
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		void Next(StringView name, const T& val)
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
		void Next(const T& val)
		{
			if (EnterNext())
			{
				Serialize(val);
				Leave();
			}
		}


		// Write a type into the current scope
		template<typename T>
		void Serialize(const T& val)
		{
			static_assert(Writable<T>,
			    "Type must be writable! No valid write function found. E.g: 'Write(Writer& w, "
			    "const T& value)'");
			if constexpr (Writable<T>)
			{
				Write(*this, val);
			}
		}

		void Leave();

		static constexpr bool IsReading()
		{
			return false;
		}
		static constexpr bool IsWriting()
		{
			return true;
		}

		void PushAddFlags(WriteFlags flags)
		{
			writer->PushAddFlags(flags);
		}
		void PushRemoveFlags(WriteFlags flags)
		{
			writer->PushRemoveFlags(flags);
		}
		void PopFlags()
		{
			writer->PopFlags();
		}

		template<SerializeFormat format>
		typename FormatBind<format>::Writer& GetWriter() requires(HasWriter<format>);
	};


	PIPE_API void Write(Writer& ct, bool val);
	PIPE_API void Write(Writer& ct, u8 val);
	PIPE_API void Write(Writer& ct, i32 val);
	PIPE_API void Write(Writer& ct, u32 val);
	PIPE_API void Write(Writer& ct, i64 val);
	PIPE_API void Write(Writer& ct, u64 val);
	PIPE_API void Write(Writer& ct, float val);
	PIPE_API void Write(Writer& ct, double val);
	PIPE_API void Write(Writer& ct, StringView val);

	template<typename T1, typename T2>
	void Write(Writer& ct, TPair<T1, T2>& val)
	{
		ct.BeginObject();
		ct.Next("first", val.first);
		ct.Next("second", val.second);
	}

	template<typename T>
	void Write(Writer& ct, const T& val) requires(
	    bool(TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize))
	{
		val.Write(ct);
	}

	template<typename T>
	void Write(Writer& ct, const T& val) requires(IsArray<T>())
	{
		u32 size = val.Size();
		ct.BeginArray(size);
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(val[i]);
		}
	}

	template<typename T>
	void Write(Writer& ct, T& val) requires IsEnum<T>
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			// Might not be necessary to cache string since enum name is static
			ct.PushAddFlags(WriteFlags_CacheStringValues);
			ct.Serialize(GetEnumName(val));
			ct.PopFlags();
		}
	}
}    // namespace p
