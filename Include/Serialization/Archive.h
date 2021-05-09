// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"
#include "Object/Struct.h"
#include "Serialization/Json.h"
#include "TypeTraits.h"

#include <fstream>


namespace Rift
{
	class Archive;
};

// Placeholder for global serializers to avoid compiler errors
template <typename T>
void Serialize(Rift::Archive&, Rift::StringView name, T)
{}

namespace Rift
{
	class Archive
	{
		const bool bLoading = false;

		// TMap<void*, FGuid> ownPtrToGuid;


	public:
		class Context* context = nullptr;

		Archive() = default;
		Archive(bool bLoading) : bLoading(bLoading), context{nullptr} {}
		virtual ~Archive() = default;

		template <typename T>
		Archive& operator()(StringView name, T& val)
		{
			CustomSerializeOrNot(name, val);
			return *this;
		}

		virtual void Serialize(StringView name, bool& val) = 0;

		virtual void Serialize(StringView name, u8& val) = 0;

		virtual void Serialize(StringView name, i32& val) = 0;

		virtual void Serialize(StringView name, u32& val) = 0;

		virtual void Serialize(StringView name, float& val) = 0;

		virtual void Serialize(StringView name, String& val) = 0;

		virtual void Serialize(StringView name, Json& val) = 0;

		void Serialize(StringView name, v2& val);
		void Serialize(StringView name, v2_u32& val);
		void Serialize(StringView name, v3& val);
		void Serialize(StringView name, Quat& val);

		template <typename T>
		void Serialize(StringView name, TOwnPtr<T>& val)
		{
			BeginObject(name);

			if (IsLoading())
			{
				if (!IsDataValid())
				{
					// Object doesn't exist, we just reset the ptr
					val.Delete();
					EndObject();
					return;
				}

				auto* ptrType = FindType<T>();
			}
			else
			{
				if (val)
				{
					Serialize("type", GetType(val));
					// Serialize("id", FGuid::New());
					*this("instance", val);
					*this("id", val);
				}
			}
			EndObject();
		}

		template <typename T>
		void Serialize(StringView name, TPtr<T>& val)
		{}

		template <typename T>
		void Serialize(StringView name, TArray<T>& val);

		template <typename T>
		void SerializeStruct(StringView name, T& val)
		{
			BeginObject(name);
			val.SerializeReflection(*this);
			EndObject();
		}

		// Starts an object by name
		virtual void BeginObject(StringView name) = 0;
		virtual bool HasObject(StringView name)   = 0;
		virtual bool IsDataValid()                = 0;

		// Starts an object by index (Array)
		virtual void BeginObject(u32 index) = 0;
		virtual void EndObject()            = 0;

		bool IsLoading()
		{
			return bLoading;
		}
		bool IsSaving()
		{
			return !bLoading;
		}

		virtual void SerializeArraySize(u32& Size) = 0;

	private:
		// Selection of 'Serialize' call
		template <typename T>
		bool CustomSerializeOrNot(StringView name, T& val);
	};


	template <typename T>
	inline void Archive::Serialize(StringView name, TArray<T>& val)
	{
		BeginObject(name);
		if (IsLoading())
		{
			u32 size;
			SerializeArraySize(size);
			val.Reserve(size);

			for (u32 i = 0; i < size; ++i)
			{
				BeginObject(i);
				{
					operator()("val", val[i]);
					// Data() = Data()[""]; // Temporal patch to avoid named
					// properties
				}
				EndObject();
			}
		}
		else
		{
			u32 size = val.Size();
			SerializeArraySize(size);

			for (u32 i = 0; i < size; ++i)
			{
				BeginObject(i);
				{
					// Data()[""] = Data(); // Temporal patch to avoid named
					// properties
					operator()("val", val[i]);
				}
				EndObject();
			}
		}
		EndObject();
	}

	template <typename T>
	inline bool Archive::CustomSerializeOrNot(StringView name, T& val)
	{
		if constexpr (ClassTraits<T>::HasCustomSerialize)
		{
			return val.Serialize(*this, name);
		}
		else if constexpr (ClassTraits<T>::HasGlobalSerialize)
		{
			return ::Serialize(*this, name, val);
		}
		else if constexpr (IsStruct<T>())
		{
			SerializeStruct(name, val);
			return true;
		}
		else
		{
			Serialize(name, val);
			return true;
		}
	}
}    // namespace Rift
