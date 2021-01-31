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
void Serialize(Rift::Archive&, const char* name, T)
{}

namespace Rift
{
	class Archive
	{
		const bool bLoads = false;

	public:
		class Context* context;

		Archive() : bLoads(false), context{nullptr} {}
		Archive(bool bLoads) : bLoads(bLoads), context{nullptr} {}
		virtual ~Archive() = default;

		template <typename T>
		Archive& operator()(const char* name, T& val)
		{
			CustomSerializeOrNot(name, val);
			return *this;
		}

		virtual void Serialize(const char* name, bool& val) = 0;

		virtual void Serialize(const char* name, u8& val) = 0;

		virtual void Serialize(const char* name, i32& val) = 0;

		virtual void Serialize(const char* name, u32& val) = 0;

		virtual void Serialize(const char* name, float& val) = 0;

		virtual void Serialize(const char* name, String& val) = 0;

		virtual void Serialize(const char* name, Json& val) = 0;

		void Serialize(const char* name, v2& val);
		void Serialize(const char* name, v2_u32& val);
		void Serialize(const char* name, v3& val);
		void Serialize(const char* name, Quat& val);

		template <typename T>
		void Serialize(const char* name, OwnPtr<T>& val)
		{
			BeginObject(name);
			// Not yet supported. Hard and soft references need to take care of each
			// other while serializing
			/*if (IsLoading())
			{
				val->Serialize(*this);
			}

			if (val)
			{
				val->Serialize(*this);
			}*/
			EndObject();
		}

		template <typename T>
		void Serialize(const char* name, Ptr<T>& val)
		{
			Name ptrName;
			if (IsSaving())
				ptrName = val ? val->GetName() : TX("None");

			// CustomSerializeOrNot(name, ptrName);

			/*if(IsLoading())
				Find and Assign object */
		}

		template <typename T>
		void Serialize(const char* name, TArray<T>& val);

		template <typename T>
		void SerializeStruct(const char* name, T& val)
		{
			BeginObject(name);
			val.SerializeReflection(*this);
			EndObject();
		}

		// Starts an object by name
		virtual void BeginObject(const String& name)
		{
			BeginObject(name.c_str());
		};
		virtual void BeginObject(const char* name) = 0;
		virtual bool HasObject(const char* name) = 0;
		virtual bool IsObjectValid() = 0;

		// Starts an object by index (Array)
		virtual void BeginObject(u32 index) = 0;
		virtual void EndObject() = 0;

		bool IsLoading()
		{
			return bLoads;
		}
		bool IsSaving()
		{
			return !bLoads;
		}

		virtual void SerializeArraySize(u32& Size) = 0;

	private:
		// Selection of 'Serialize' call
		template <typename T>
		bool CustomSerializeOrNot(const char* name, T& val);
	};


	class JsonArchive : public Archive
	{
		Json baseData;
		TArray<Json*> depthData;

		const bool bBeautify;

	public:
		// Save Constructor
		JsonArchive(const bool bBeautify = true)
			: Archive(false)
			, baseData()
			, depthData{}
			, bBeautify{bBeautify}
		{}

		// Load constructor
		JsonArchive(const Json& data) : Archive(true), baseData(data), depthData{}, bBeautify{false}
		{}

		virtual ~JsonArchive() = default;

		String GetDataString() const
		{
			return baseData.dump(GetIndent());
		}
		const Json& GetData() const
		{
			return baseData;
		}

		i32 GetIndent() const
		{
			return bBeautify ? 2 : -1;
		}

	private:
		virtual void Serialize(const char* name, bool& val) override;

		virtual void Serialize(const char* name, u8& val) override;

		virtual void Serialize(const char* name, i32& val) override;

		virtual void Serialize(const char* name, u32& val) override;

		virtual void Serialize(const char* name, float& val) override;

		virtual void Serialize(const char* name, String& val) override;

		virtual void Serialize(const char* name, Json& val) override;

		Json& Data()
		{
			return !depthData.IsEmpty() ? *depthData.Last() : baseData;
		}

		virtual void BeginObject(const char* name) override
		{
			depthData.Add(&Data()[name]);
		}

		virtual bool HasObject(const char* name) override
		{
			return Data().find(name) != Data().end();
		}

		virtual void BeginObject(u32 index) override
		{
			depthData.Add(&Data()[index]);
		}

		virtual bool IsObjectValid() override
		{
			return !Data().is_null();
		}

		virtual void EndObject() override
		{
			depthData.RemoveAt(depthData.Size() - 1, false);
		}

		virtual void SerializeArraySize(u32& size) override
		{
			Json& data = Data();
			if (IsLoading())
			{
				size = data.is_null() ? 0 : (u32) data.size();
			}
			else
			{
				if (!data.is_array())
					data = Json::array();
				Json::array_t* ptr = data.get_ptr<Json::array_t*>();
				ptr->reserve((size_t) size);
			}
		};
	};


	template <typename T>
	inline void Archive::Serialize(const char* name, TArray<T>& val)
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
	inline bool Archive::CustomSerializeOrNot(const char* name, T& val)
	{
		if constexpr (ClassTraits<T>::HasCustomSerialize)
		{
			return val.Serialize(*this, name);
		}
		else if constexpr (ClassTraits<T>::HasGlobalSerialize)
		{
			return ::Serialize(*this, name, val);
		}
		else if constexpr (IsStructType<T>())
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
}	 // namespace Rift
