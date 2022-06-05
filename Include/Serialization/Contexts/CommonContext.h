// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Checks.h"
#include "Core/Platform.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Contexts/ReadContext.h"
#include "Serialization/Contexts/WriteContext.h"
#include "Serialization/SerializationTypes.h"


namespace pipe::serl
{
	struct CORE_API CommonContext
	{
		enum class Mode : u8
		{
			Read,
			Write
		};


	private:
		Mode mode;
		union
		{
			ReadContext* readContext;
			WriteContext* writeContext;
		};


	public:
		CommonContext(ReadContext& readContext) : mode{Mode::Read}, readContext{&readContext} {}
		CommonContext(WriteContext& writeContext) : mode{Mode::Write}, writeContext{&writeContext}
		{}

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
		template<typename T>
		void Next(StringView name, T& val)
		{
			if (IsWriting())
			{
				writeContext->Next(name, val);
			}
			else
			{
				readContext->Next(name, val);
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
		template<typename T>
		void Next(T& val)
		{
			if (IsWriting())
			{
				// writeContext->Next(val);
			}
			else
			{
				readContext->Next(val);
			}
		}

		// Reads or Writes a type from the current scope
		template<typename T>
		void Serialize(T& val)
		{
			if (IsWriting())
			{
				writeContext->Serialize(const_cast<const T&>(val));
			}
			else
			{
				readContext->Serialize(val);
			}
		}

		void Leave();

		bool IsReading() const
		{
			return mode == Mode::Read;
		}
		bool IsWriting() const
		{
			return mode == Mode::Write;
		}

		ReadContext& GetRead() const
		{
			Check(IsReading());
			return *readContext;
		}

		WriteContext& GetWrite() const
		{
			Check(IsWriting());
			return *writeContext;
		}

		void PushAddFlags(WriteFlags flags);
		void PushRemoveFlags(WriteFlags flags);
		void PopFlags();
	};


	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		val.Serialize(commonContext);
	}
	template<typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		Serialize(commonContext, val);
	}

	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Write(WriteContext& ct, const T& val) requires(
	    bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		const_cast<T&>(val).Serialize(commonContext);
	}
	template<typename T>
	void Write(WriteContext& ct, const T& val) requires(
	    bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		Serialize(commonContext, const_cast<T&>(val));
	}
}    // namespace pipe::serl

namespace pipe
{
	using namespace pipe::serl;
}
