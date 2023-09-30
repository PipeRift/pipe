// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/Reader.h"
#include "Pipe/Serialize/Writer.h"


namespace p
{
	struct PIPE_API ReadWriter
	{
	private:
		Reader* reader;
		Writer* writer;


	public:
		ReadWriter(Reader& reader) : reader{&reader}, writer{nullptr} {}
		ReadWriter(Writer& writer) : reader{nullptr}, writer{&writer} {}
		ReadWriter(const ReadWriter& other)            = default;
		ReadWriter& operator=(const ReadWriter& other) = default;

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
				writer->Next(name, val);
			}
			else
			{
				reader->Next(name, val);
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
				reader->Next(val);
			}
		}

		// Reads or Writes a type from the current scope
		template<typename T>
		void Serialize(T& val)
		{
			if (IsWriting())
			{
				writer->Serialize(const_cast<const T&>(val));
			}
			else
			{
				reader->Serialize(val);
			}
		}

		void Leave();

		bool IsReading() const
		{
			return reader != nullptr;
		}
		bool IsWriting() const
		{
			return writer != nullptr;
		}

		Reader& GetRead() const
		{
			Check(IsReading());
			return *reader;
		}

		Writer& GetWrite() const
		{
			Check(IsWriting());
			return *writer;
		}

		void PushAddFlags(WriteFlags flags);
		void PushRemoveFlags(WriteFlags flags);
		void PopFlags();
	};


	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Read(Reader& ct, T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		val.Serialize(commonContext);
	}
	template<typename T>
	void Read(Reader& ct, T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		Serialize(commonContext, val);
	}

	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Write(Writer& ct, const T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		const_cast<T&>(val).Serialize(commonContext);
	}
	template<typename T>
	void Write(Writer& ct, const T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		Serialize(commonContext, const_cast<T&>(val));
	}
}    // namespace p
