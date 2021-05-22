// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/SerializationTypes.h"
#include "Strings/String.h"
#include "Strings/StringView.h"
#include "Templates/Tuples.h"


namespace Rift::Serl
{
	struct CORE_API ReadContext
	{
		template <Format format>
		friend struct TFormatReader;

		Flags flags           = Flags_None;
		Format format         = Format::None;
		IFormatReader* reader = nullptr;


	private:
		ReadContext() = default;

	public:
		ReadContext(const ReadContext&) = default;
		ReadContext& operator=(const ReadContext&) = default;
		virtual ~ReadContext() {}


		/**
		 * Starts the deserialization of an scope as an object.
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * This function will fail on array scopes
		 */
		bool EnterNext(StringView name);

		/**
		 * Deserializes a value from an object key
		 * This function will fail on array scopes
		 */
		template <typename T>
		void Next(StringView name, T& val)
		{
			if (EnterNext(name))
			{
				Serialize(val);
				Leave();
			}
		}


		/**
		 * Starts the deserialization of an scope as an array.
		 * @param size of the array being read
		 */
		void BeginArray(u32& size);

		/**
		 * Enters the scope of the next element of an array.
		 * This function will fail on object scopes
		 */
		bool EnterNext();

		/**
		 * Deserializes a value from the next element of an array.
		 * This function will fail on object scopes
		 */
		template <typename T>
		void Next(T& val)
		{
			if (EnterNext())
			{
				Serialize(val);
				Leave();
			}
		}

		// Reads a type from the current scope
		template <typename T>
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
		template <Format format>
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
	CORE_API void Read(ReadContext& ct, String& val);

	template <typename T1, typename T2>
	void Read(ReadContext& ct, TPair<T1, T2>& val)
	{
		ct.BeginObject();
		ct.Next("first", v.first);
		ct.Next("second", v.second);
	}

	template <typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasMemberSerialize && !TypeFlags<T>::HasSingleSerialize))
	{
		val.Read(ct);
	}

	template <typename T>
	void Read(ReadContext& ct, T& val) requires(IsArray<T>())
	{
		u32 size;
		ct.BeginArray(size);
		val.Resize(size);
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(val[i])
		}
	}
}    // namespace Rift::Serl
