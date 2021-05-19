// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/ReadContext.h"
#include "Serialization/SerializationTypes.h"
#include "Serialization/WriteContext.h"


namespace Rift::Serl
{
	struct CommonContext
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

		template <typename T>
		void Serialize(T& val)
		{
			if (IsWriting())
			{
				writeContext->Write(*this, val);
			}
			readContext->Read(*this, val);
		}

		bool IsReading() const
		{
			return mode == Mode::Read;
		}
		bool IsWriting() const
		{
			return mode == Mode::Write;
		}
	};


	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template <typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasSingleSerialize&& TypeFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		val.Serialize(commonContext);
	}
	template <typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasSingleSerialize && !TypeFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		Serialize(commonContext, val);
	}

	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template <typename T>
	void Write(WriteContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasSingleSerialize&& TypeFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		val.Serialize(commonContext);
	}
	template <typename T>
	void Write(WriteContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasSingleSerialize && !TypeFlags<T>::HasMemberSerialize))
	{
		CommonContext commonContext{ct};
		Serialize(commonContext, val);
	}
}    // namespace Rift::Serl
